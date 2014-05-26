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

#include "pass_list.h++"
#include <map>

static std::multimap<pass_number, std::shared_ptr<pass>> pass_map;

void pass_list_add(const std::shared_ptr<pass>& pass, const pass_number& num)
{
    pass_map.insert(std::make_pair(num, pass));
}

const std::vector<std::shared_ptr<pass>> pass_list_lookup(const pass_number& n)
{
    auto v = std::vector<std::shared_ptr<pass>>();

    for (auto it = pass_map.find(n); it != pass_map.upper_bound(n) && it != pass_map.end(); ++it)
        v.push_back(it->second);

    return v;
}
