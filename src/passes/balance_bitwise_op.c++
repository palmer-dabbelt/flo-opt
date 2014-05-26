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
             * filter above are ones that match the opcode
             * provided. */

            /* FIXME: Do something with large connected components. */
            for (const auto& op: comp)
                to_fill->add_op(op);
        }
};

void init(void)
{
    std::vector<libflo::opcode> opcodes = {libflo::opcode::AND,
                                           libflo::opcode::OR,
                                           libflo::opcode::XOR};

    for (const auto& opcode: opcodes) {
        auto pass = std::make_shared<balance_bitwise_op>(opcode);
        pass_list_add(pass, pass_number::REBALANCE);
    }
}
