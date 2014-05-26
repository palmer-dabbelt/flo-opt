/*
 * Copyright (C) 2014 Palmer Dabbelt
 *   <palmer.dabbelt@eecs.berkeley.edu>
 *
 * This file is part of flo-opt.
 *
 * flo-opt is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * flo-opt is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with flo-opt.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <pass.h++>
#include <pass_list.h++>
#include <libflo/opcode.h++>
#include <libpass/connected_components.h++>
#include <queue>

typedef std::shared_ptr<node> node_ptr;
typedef std::shared_ptr<operation> operation_ptr;
typedef std::shared_ptr<flo> flo_ptr;

static void init(void) __attribute__((constructor));

class balance_bitwise_op: public pass {
private:
    const libflo::opcode _opcode;
    const std::string _name;

public:
    balance_bitwise_op(const libflo::opcode& opcode)
        : _opcode(opcode),
          _name("balance_bitwise_op: " + libflo::opcode_to_string(opcode))
        {
        }

    const std::string& name(void) const
        {
            return _name;
        }

    const flo_ptr operate(const flo_ptr& input) const
        {
            auto output = flo::empty();

            /* Builds a list of the connected components, where a edge
             * is defined as an operation that matches the type this
             * pass was created with.  Essentially what's going to
             * happen is that we'll end up with a bunch of small
             * connected components: all non-OP components will have a
             * single node in their connected component (as they're
             * connected to nothing) and the rest of the ops will
             * have */
            auto test_func = [&](const operation_ptr& op,
                                 const node_ptr& from __attribute__((unused)),
                                 const node_ptr& to __attribute__((unused)))
                -> bool
                {
                    return op->op() == _opcode;
                };
            libpass::connected_components comp(input, test_func);

            for (const auto& id: comp.component_ids())
                fill_cc(output, comp.ops(id));

            return output;
        }

private:
    void fill_cc(flo_ptr& to_fill,
                 const std::vector<operation_ptr>& comp) const
        {
            /* Small connected components can't have anything done to
             * them so there's no point in bothering with any
             * optimization. */
            if (comp.size() < 3) {
                for (const auto& op: comp)
                    to_fill->add_op(op);
                return;
            }
            /* The only connected components that made it through that
             * filter above are ones that match the opcode provided --
             * that's a side effect of the operation being so
             * small. */

            /* Map a node to the operation that produces it. */
            std::unordered_map<node_ptr, operation_ptr> node2binop;
            std::unordered_map<node_ptr, operation_ptr> node2op;
            for (const auto& op: comp) {
                if (op->op() == _opcode)
                    node2binop[op->d()] = op;
                else
                    node2op[op->d()] = op;
            }

            /* Walk through the list of operations trying to figure
             * out which are outputs.  This is doen in two passes,
             * which probably isn't the post efficient: essentially
             * what happens is that we add every node to a map and
             * then remove every node that is read by any operation in
             * the tree.  The remaining nodes must be outputs. */
            std::unordered_map<node_ptr, bool> is_output;
            for (const auto& op: comp)
                is_output[op->d()] = true;
            for (const auto& op: comp)
                for (const auto& source: op->sources())
                    is_output[source] = false;

            /* Now that we know which nodes are outputs of this
             * connected component we need to go ahead and reconstruct
             * the output nodes in a more efficient manner. */
            for (const auto& op: comp) {
                /* Operations that aren't an outputs still need to be
                 * emitted because they could be used by some other
                 * part of the circuit.  It's expected that a dead
                 * code elimination phase runs AFTER this, so that's
                 * considered OK for now. */
                /* FIXME: Actually do DCE. */
                if (is_output[op->d()] == false) {
                    to_fill->add_op(op);
                    continue;
                }

                /* Determine the set of nodes that are inputs to this
                 * output.  These are just any nodes that aren't
                 * constructed by anything. */
                std::unordered_map<node_ptr, bool> in_input_set;
                std::queue<node_ptr> input_set;
                std::queue<operation_ptr> queue({op});
                while (queue.size() > 0) {
                    auto cur = queue.front();
                    queue.pop();

                    for (const auto source: cur->sources()) {
                        auto l = node2binop.find(source);
                        if (l != node2binop.end()) {
                            queue.push(l->second);
                        } else if (in_input_set[source] != true) {
                            input_set.push(source);
                            to_fill->add_op(node2op[source]);
                            in_input_set[source] = true;
                        }
                    }
                }

                /* Now that we've got the input set go ahead and
                 * construct a balanced tree that computes that input
                 * exactly. */
                while (input_set.size() > 1) {
                    auto a = input_set.front(); input_set.pop();
                    auto b = input_set.front(); input_set.pop();
                    auto n = node::make_temp(a);

                    auto op = std::make_shared<operation>(
                        n,
                        n->width_u(),
                        _opcode,
                        std::vector<node_ptr>({a, b})
                        );

                    input_set.push(n);
                    to_fill->add_op(op);
                }

                /* There's exactly one node remaining in the input
                 * set, which is actually the node that should be
                 * output.  Note that it's fine to emit this MOV here
                 * as we'll just be getting rid of it later in a
                 * special pass. */
                /* FIXME: Actually implement MOV elision. */
                auto mov_op = std::make_shared<operation>(
                    op->d(),
                    op->d()->width(),
                    libflo::opcode::MOV,
                    std::vector<node_ptr>({input_set.front()})
                    );
                to_fill->add_op(mov_op);
            }
        }
};

void init(void)
{
    std::vector<libflo::opcode> opcodes = {libflo::opcode::AND,
                                           libflo::opcode::OR};

    for (const auto& opcode: opcodes) {
        auto pass = std::make_shared<balance_bitwise_op>(opcode);
        pass_list_add(pass, pass_number::REBALANCE);
    }
}
