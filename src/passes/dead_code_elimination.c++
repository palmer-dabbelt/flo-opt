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
#include <unordered_map>
#include <queue>

typedef std::shared_ptr<node> node_ptr;
typedef std::shared_ptr<operation> operation_ptr;
typedef std::shared_ptr<flo> flo_ptr;

static void init(void) __attribute__((constructor));

class dead_code_elimination: public pass {
private:
    const std::string _name;

public:
    dead_code_elimination(void)
        : _name("dead_code_elimination")
        {
        }

    const std::string& name(void) const
        {
            return _name;
        }

    const flo_ptr operate(const flo_ptr& input) const
        {
            std::unordered_map<node_ptr, bool> emitted;
            auto out = flo::empty();

            /* Build a map from nodes to the operation that emits
             * them. */
            std::unordered_map<node_ptr, operation_ptr> node2op;
            for (const auto& op: input->operations())
                node2op[op->d()] = op;

            /* Find every node in the system  */
            for (const auto& check_op: input->operations()) {
                if (check_op->op() != libflo::opcode::OUT)
                    continue;

                std::queue<operation_ptr> queue({check_op});
                while (queue.size() > 0) {
                    auto op = queue.front(); queue.pop();

                    auto emit = [&](const node_ptr& to_emit) -> void
                        {
                            if (to_emit->is_const())
                                return;

                            auto l = emitted.find(to_emit);
                            if (l != emitted.end())
                                return;

                            emitted[to_emit] = true;

                            auto nl = node2op.find(to_emit);
                            if (nl == node2op.end()) {
                                fprintf(stderr, "Unable to lookup node '%s'\n",
                                        to_emit->name().c_str()
                                    );
                                abort();
                            }

                            out->add_op(nl->second);
                            queue.push(nl->second);
                        };

                    emit(op->d());
                    for (const auto& source: op->sources())
                        emit(source);
                }
            }

            return out;
        }
};

void init(void)
{
    std::vector<pass_number> passes = {pass_number::LATE_DCE};

    for (const auto &pass_number: passes) {
        auto pass = std::make_shared<dead_code_elimination>();
        pass_list_add(pass, pass_number);
    }
}
