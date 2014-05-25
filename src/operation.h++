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

#ifndef OPERATION_HXX
#define OPERATION_HXX

#include <libflo/operation.h++>
#include <libflo/node.h++>
#include "node.h++"

class operation: public libflo::operation<node> {
public:
    operation(std::shared_ptr<node>& dest,
              const libflo::unknown<size_t>& width,
              const libflo::opcode& op,
              const std::vector<std::shared_ptr<node>>& s);
};

#endif
