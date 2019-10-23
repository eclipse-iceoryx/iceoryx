// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "iceoryx_utils/internal/cxx/set.hpp"
#include "iceoryx_utils/internal/graphs/directed_graph.hpp"

#include <cstdint>

///@todo: refine and move to utils
/// same functionality as its parent class DirectedGraph, except that adding edges that close a cycle
/// is impossible now (this incurs a checking overhead of course)
template <typename VertexType, int32_t VERTEX_LIMIT, int32_t DEGREE_LIMIT>
class DirectedAcyclicGraph : public DirectedGraph<VertexType, VERTEX_LIMIT, DEGREE_LIMIT>
{
  private:
    using base_type = DirectedGraph<VertexType, VERTEX_LIMIT, DEGREE_LIMIT>;

  public:
    using Index_t = typename base_type::Index_t;

    DirectedAcyclicGraph()
    {
        init();
    }

    /// add an edge between fromVertex and toVertex to the graph
    /// @param [in] fromVertex
    /// @param [in] toVertex
    /// @return pointer true if the edge was added successfully, false otherwise
    /// (e.g. it would create a cycle)
    virtual bool addEdge(VertexType* fromVertex, VertexType* toVertex)
    {
        auto from = this->findVertex(fromVertex);
        auto to = this->findVertex(toVertex);
        if (from < 0 || to < 0 || from == to)
        {
            return false; // need to add vertices first or there would be a self loop
        }

        if (createsCycle(from, to))
        {
            return false; // new edge would create a cycle, do not add it
        }

        if (base_type::addEdge(fromVertex, toVertex))
        {
            updateConnectivity(from, to);
            return true;
        }

        return false;
    }

  private:
    ///@todo: static cast should only be a temporary solution, currently VERTEX_LIMIT has to
    /// be unsigned for the tests and since it is a postive quantity this makes sense
    /// TFixedVector cannot deal with this properly since its capacity is a signed type (which
    /// is nonconforming with STL and makes no sense since a capacity is always nonnegative)
    /// casting is no problem for the ranges we use, but for very large values (larger than
    // max of int32) the cast will change the value
    using IndexSet = iox::cxx::vector<Index_t, VERTEX_LIMIT>;

    // contains for each vertex index the indices of the vertices it can be reached from
    iox::cxx::vector<IndexSet, VERTEX_LIMIT> m_reachableFrom;

    // contains for each vertex index the indices of vertices that can be reached from it
    iox::cxx::vector<IndexSet, VERTEX_LIMIT> m_leadsTo;

    // check whether the graph where we add an edge between from and to would be cyclic
    bool createsCycle(const Index_t from, const Index_t to)
    {
        // if there is already is a path from the to-vertex to the from-vertex
        // adding an edge from the from-vertex to the to-vertex would create a cycle
        if (iox::cxx::set::hasElement(m_leadsTo[to], from))
        {
            return true;
        }
        return false;
    }

    // update the connectivity data (m_reachableFrom and m_leadsTo) when we add an edge between from and to
    // note that this is quite expensive for large graphs, but we do not expect large graphs
    // also note that this cannot easily be avoided in general, e.g. depth first search would be
    // even more costly
    void updateConnectivity(const Index_t from, const Index_t to)
    {
        // update predecessors of to-vertex
        // (due to the new edge it can now be reached by every vertex with a path to the from-vertex)
        auto& inTo = m_reachableFrom[to];
        iox::cxx::set::add(inTo, from);
        iox::cxx::set::unify(inTo, m_reachableFrom[from]);

        // update successors of from-vertex
        // (due to the new edge we can now reach every vertex that can be reached by the from-vertex)
        auto& outFrom = m_leadsTo[from];
        iox::cxx::set::add(outFrom, to);
        iox::cxx::set::unify(outFrom, m_leadsTo[to]);

        // update every predecessor of from-vertex
        //(from them we can reach every vertex that can be reached from the to-vertex )
        for (auto pred : m_reachableFrom[from])
        {
            iox::cxx::set::unify(m_leadsTo[pred], outFrom);
        }

        // update every successor of to-vertex
        //(they can now be reached from every predecessor of from-vertex )
        auto& inFrom = m_reachableFrom[from];
        for (auto succ : m_leadsTo[to])
        {
            iox::cxx::set::unify(m_reachableFrom[succ], inFrom);
        }
    }

    void init()
    {
        // as a small optimisation we could defer the
        // initialization to the time the vertex is actually added
        for (size_t i = 0; i < VERTEX_LIMIT; ++i)
        {
            m_reachableFrom.emplace_back(IndexSet());
            m_leadsTo.emplace_back(IndexSet());
        }
    }
};
