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

#include "pass_number.h++"

/* A global list of all the pass numbers that are in use by the
 * system, which is initailized as a library constructor. */
static std::vector<pass_number> pass_numbers;
static void init_pass_numbers(void) __attribute__((constructor));

const std::vector<pass_number>& all_pass_numbers(void)
{
    return pass_numbers;
}

void init_pass_numbers(void)
{
    for (size_t i = 0; i <= max_pass_number; ++i)
        pass_numbers.push_back((pass_number)i);
}
