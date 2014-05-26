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

#ifndef LIBPASS__CONNECTED_COMPONENTS_HXX
#define LIBPASS__CONNECTED_COMPONENTS_HXX

#include <flo.h++>
#include <functional>
#include <unordered_map>

namespace libpass {
    /* Finds a set of connected components inside a Flo file. */
    class connected_components {
        typedef std::shared_ptr<node> node_ptr;
        typedef std::shared_ptr<operation> operation_ptr;
        typedef std::shared_ptr<flo> flo_ptr;

    private:
        std::unordered_map<size_t, std::vector<node_ptr>> _comp2node;
        std::unordered_map<size_t, std::vector<operation_ptr>> _comp2op;
        std::vector<size_t> _comps;

    public:
        /* This is a test function, which returns TRUE if the
         * operation "link" provides a connected edge from the vertex
         * "from" to the vertex "to". */
        using test_func_t = std::function<bool(const operation_ptr& link,
                                               const node_ptr& from,
                                               const node_ptr& to)>;

    public:
        /* Finds all the connected components of the given graph,
         * where connectivity is defined by the given function as
         * described above. */
        connected_components(const flo_ptr& graph, test_func_t func);

    public:
        /* Lists the component IDs that were inferred. */
        const std::vector<size_t>& component_ids(void) const
            { return _comps; }

        /* Lists the components that match a particular ID, either by
         * node name or by operation name. */
        const std::vector<node_ptr>& nodes(size_t id)
            { return _comp2node[id]; }
        const std::vector<operation_ptr>& ops(size_t id)
            { return _comp2op[id]; }
    };
}

#endif
