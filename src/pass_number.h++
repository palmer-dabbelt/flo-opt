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

#ifndef PASS_NUMBER_HXX
#define PASS_NUMBER_HXX

#include <vector>
#include <unistd.h>

/* This contains the list of pass numbers that the system knows about.
 * Be sure to update "max_pass_number" if this changes! */
enum class pass_number {
    INIT,
    FINAL
};

/* The largest pass number in the system, useful for iterating through
 * passes.  This must be kept in sync manually! */
static const size_t max_pass_number = (size_t)(pass_number::FINAL);

/* Returns a list of all the pass numbers that are known to the system
 * during this execution. */
const std::vector<pass_number> all_pass_numbers(void);

#endif
