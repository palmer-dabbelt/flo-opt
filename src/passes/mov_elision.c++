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

typedef std::shared_ptr<node> node_ptr;
typedef std::shared_ptr<operation> operation_ptr;
typedef std::shared_ptr<flo> flo_ptr;

static void init(void) __attribute__((constructor));

class mov_elision: public pass {
private:
    const std::string _name;

public:
    mov_elision(void)
        : _name("mov_elision")
        {
        }

    const std::string& name(void) const
        {
            return _name;
        }

    const flo_ptr operate(const flo_ptr& input) const
        {
            /* Build a map that contains the new node that replaces
             * each input node. */
            std::unordered_map<node_ptr, node_ptr> target;
            for (const auto& op: input->operations()) {
                if (op->op() != libflo::opcode::MOV)
                    continue;

                target[op->d()] = op->s();
            }

            /* Rewrite every operation to eliminate the output of MOV
             * operations. */
            auto out = flo::empty();
            for (const auto& op: input->operations()) {
                if (op->op() == libflo::opcode::MOV)
                    continue;

                std::vector<node_ptr> sv;
                for (const auto& source: op->sources()) {
                    auto l = target.find(source);
                    if (l == target.end())
                        sv.push_back(source);
                    else
                        sv.push_back(l->second);
                }

                auto nop = std::make_shared<operation>(
                    op->d(),
                    op->d()->width(),
                    op->op(),
                    sv
                    );
                out->add_op(nop);
            }

            return out;
        }
};

void init(void)
{
    std::vector<pass_number> passes = {pass_number::LATE_DCE};

    for (const auto &pass_number: passes) {
        auto pass = std::make_shared<mov_elision>();
        pass_list_add(pass, pass_number);
    }
}
