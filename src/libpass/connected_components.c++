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

#include "connected_components.h++"
#include <unordered_map>
#include <queue>
using namespace libpass;

namespace std {
    template <class item_t>
    std::queue<item_t> make_queue(const item_t& item)
    { return std::queue<item_t>({item}); }

    template <class item_t>
    std::vector<item_t> make_vector(const item_t& item)
    { return std::vector<item_t>({item}); }
}

connected_components::connected_components(const flo_ptr& graph,
                                           test_func_t func)
    : _comp2node(),
      _comp2op(),
      _comps()
{
    /* The first thing we need to do here is beild the incoming and
     * outgoing maps.  These allow me to quickly build the connected
     * component of a graph without the need to call the node
     * interrogation function a large number of times. */
    std::multimap<node_ptr, node_ptr> incoming, outgoing;
    for (const auto& edge: graph->operations()) {
        auto to = edge->d();
        for (const auto& from: edge->sources()) {
            if (func(edge, from, to) == true) {
                incoming.insert(std::make_pair(to, from));
                outgoing.insert(std::make_pair(from, to));
            }
        }
    }

    /* Create a map from nodes to operations -- this way we only need
     * to track the nodes of a connected component (which makes the
     * whole thing look more like a regular graph algorithm) but we
     * can still return the operation list. */
    std::unordered_map<node_ptr, operation_ptr> node2op;
    for (const auto& op: graph->operations())
        node2op[op->d()] = op;

    /* Begin building the connected component map by simply picking
     * some node that's yet to be assigned to a component, assigning
     * it to a component, and then proceeding to walk every node
     * that's connected to that node in order to produce the connected
     * component. */
    std::unordered_map<node_ptr, size_t> node2comp;
    std::multimap<size_t, node_ptr> comp2node;
    size_t current_component = 0;
    for (const auto& node: graph->nodes()) {
        bool inc_component = false;

        /* If this node was previously found then don't worry about it
         * at all! */
        if (node2comp.find(node) != node2comp.end())
            continue;

        /* There's just a queue here which ensures that every node is
         * eventually found.  The invariant is that any node that's
         * been inserted into "node2comp" has been inserted into this
         * queue exactly once (though it may have been on another run
         * through the top-level loop).  The one exception is that
         * first node that was added, which isn't added anywhere. */
        auto to_process = std::make_queue(node);
        while (to_process.size() > 0) {
            auto cur = to_process.front();
            to_process.pop();

            /* Check to see if this node has already been placed into
             * a component.  If so then we don't need to enqueue it
             * again, as it's already been processed. */
            auto l = node2comp.find(cur);

            if (l != node2comp.end()) {
                /* Here's a consistancy check: if we can reach a node
                 * in a different connected component from this
                 * connected component then that's a problem. */
                if (l->second != current_component) {
                    fprintf(stderr, "ERROR: node '%s' connects to '%s'\n",
                            node->name().c_str(),
                            cur->name().c_str()
                        );
                    fprintf(stderr, "  '%s' in " SIZET_FORMAT "\n",
                            cur->name().c_str(),
                            l->second
                        );
                    fprintf(stderr, "  '%s' in " SIZET_FORMAT "\n",
                            node->name().c_str(),
                            current_component
                        );
                    abort();
                }

                /* Move on to another node, this one has already been
                 * placed. */
                continue;
            }

            /* At this point we know that this node has yet to
             * actually be placed in any component.  That means we
             * want to insert this node and all the nodes it can see
             * into this node's connected component.  As there's a
             * whole bunch of stuff that needs to do exactly the same
             * operation, this function quickly allows us to do
             * that. */
            auto insert_node = [&](const node_ptr& node) -> void
                {
                    if (node2comp.find(node) != node2comp.end())
                        return;

                    node2comp[node] = current_component;
                    comp2node.insert(std::make_pair(current_component, node));
                    to_process.push(node);
                    inc_component = true;
                };

            /* Now we simply insert every node this one can see. */
            insert_node(cur);
            for (auto it = incoming.find(cur); it != incoming.end(); ++it)
                insert_node(it->second);
            for (auto it = outgoing.find(cur); it != outgoing.end(); ++it)
                insert_node(it->second);
        }

        if (inc_component == true)
            current_component++;
    }

    /* At this point we're done, which means it's time to sanity check
     * the list of components that were discovered while adding them
     * all to the big map. */
    for (size_t cmp = 0; cmp < current_component; ++cmp) {
        std::vector<node_ptr> nodes;
        std::vector<operation_ptr> ops;

        for (auto it = comp2node.find(cmp); it != comp2node.upper_bound(cmp) && it != comp2node.end(); ++it) {
            auto node = it->second;

            auto l = node2comp.find(node);
            if (l == node2comp.end()) {
                fprintf(stderr, "Found one-directional component\n");
                abort();
            }

            if (l->second != cmp) {
                fprintf(stderr, "Found mis-matched component\n");
                abort();
            }

            nodes.push_back(node);

            auto ol = node2op.find(node);
            if (ol != node2op.end())
                ops.push_back(ol->second);
        }

        /* And here's really the crux of the whole thing: we're just
         * building up the internal data structures that represent all
         * the connected components of a graph.  Unfortunately C++
         * doesn't allow us to mark these as const... :(. */
        _comp2node[cmp] = nodes;
        _comp2op[cmp] = ops;
        _comps.push_back(cmp);
    }
}
