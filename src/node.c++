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

#include "node.h++"

node::node(const std::string name,
           const libflo::unknown<size_t>& width,
           const libflo::unknown<size_t>& depth,
           bool is_mem,
           bool is_const,
           libflo::unknown<size_t> cycle,
           const libflo::unknown<std::string>& posn)
    : libflo::node(name, width, depth, is_mem, is_const, cycle, posn)
{
}

std::shared_ptr<node> node::make_temp(const std::shared_ptr<node>& t)
{
    static size_t index = 0;
    char name[1024];
    snprintf(name, 1024, "OPT" SIZET_FORMAT, index++);

    return std::make_shared<node>(name,
                                  t->width_u(),
                                  libflo::unknown<size_t>(),
                                  false,
                                  false,
                                  libflo::unknown<size_t>(),
                                  libflo::unknown<std::string>()
        );
}
