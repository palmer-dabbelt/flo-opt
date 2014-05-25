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

#ifndef FLO_HXX
#define FLO_HXX

#include <libflo/flo.h++>
#include "operation.h++"
#include "node.h++"

class flo: public libflo::flo<node, operation> {
    typedef std::shared_ptr<operation> operation_ptr;
    typedef std::shared_ptr<node> node_ptr;

public:
    /* This constructor is used by libflo. */
    flo(std::map<std::string, node_ptr>& nodes,
        std::vector<operation_ptr>& ops);

public:
    /* This is pretty funky.  Essentially it's just a way to work
     * around the fact that just calling "flo::parse()" from some code
     * will actually end up giving you the version that returns a
     * libflo::flo<...> which can't be casted to this sort of flo. */
    static const std::shared_ptr<flo> parse(const std::string& filename)
        {
            return libflo::flo<node, operation>::parse_help<flo>(
                filename,
                libflo::flo<node, operation>::create_node
                );
        }
};

#endif
