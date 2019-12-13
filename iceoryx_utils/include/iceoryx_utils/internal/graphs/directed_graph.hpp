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

#include "iceoryx_utils/cxx/vector.hpp"

#include <cstdint>
#include <limits>

///@todo: refine and move to utils
// remark: only supports adding but no removal of vertices or edges
// and this graph is not necessarily acyclic
template <typename VertexType, int32_t VERTEX_LIMIT, int32_t DEGREE_LIMIT>
class DirectedGraph
{
  public:
    using Index_t = int32_t;
    using AdjacencyList = iox::cxx::vector<VertexType*, DEGREE_LIMIT>;

    static constexpr Index_t INVALID_INDEX = -1;

    /// add a vertex to the graph
    /// @param [in] vertex vertex to be added
    /// @return pointer true if the vertex was added successfully, false otherwise
    /// (e.g. capacity reached or vertex already exists)
    bool addVertex(VertexType* vertex)
    {
        if (numberOfVertices() >= VERTEX_LIMIT)
        {
            return false;
        }

        if (findVertex(vertex) >= 0)
        {
            return false; // already exists
        }

        VertexData data(vertex);
        m_vertices.emplace_back(std::move(data));
        return true;
    }

    /// add an edge between fromVertex and toVertex to the graph
    /// @param [in] fromVertex
    /// @param [in] toVertex
    /// @return pointer true if the edge was added successfully, false otherwise
    /// (e.g. vertices do not exist)
    virtual bool addEdge(VertexType* fromVertex, VertexType* toVertex)
    {
        auto from = findVertex(fromVertex);
        auto to = findVertex(toVertex);
        if (from < 0 || to < 0 || from == to)
        {
            return false; // need to add vertices first (could do this here as well)
        }
        VertexData& fromData = m_vertices[from];
        VertexData& toData = m_vertices[to];

        if (static_cast<size_t>(fromData.successorIndices.size()) >= DEGREE_LIMIT
            || static_cast<size_t>(toData.predecessorIndices.size()) >= DEGREE_LIMIT)
        {
            return false; // degree limit of at least one vertex would be exceeded
        }

        // only store indices, value can be found at this index
        fromData.successorIndices.emplace_back(to);
        toData.predecessorIndices.emplace_back(from);

        // add to successor/predecessor lists to return on demand
        fromData.successors.emplace_back(toVertex);
        toData.predecessors.emplace_back(fromVertex);

        ++m_numEdges;

        return true;
    }

    /// get the internal index of a given vertex
    /// @param [in] vertex vertex of which the index is requested
    /// @return internal index of vertex, -1 if vertex does not exist
    Index_t getIndex(VertexType const* vertex)
    {
        return findVertex(vertex);
    }

    /// get the direct successors of a given vertex in the graph
    /// @param [in] vertex vertex of which the successors are requested
    /// @return pointer to the list of successors, nullptr if vertex does not exist
    const AdjacencyList* getSuccessors(VertexType const* vertex)
    {
        return getSuccessors(findVertex(vertex));
    }

    /// get the direct predecessors of a given vertex in the graph
    /// @param [in] vertex vertex of which the predecessors are requested
    /// @return pointer to the list of predecessors, nullptr if vertex does not exist
    const AdjacencyList* getPredecessors(VertexType const* vertex)
    {
        return getPredecessors(findVertex(vertex));
    }

    /// get the direct successors of a given vertex index in the graph
    /// @param [in] index index of vertex of which the successors are requested
    /// @return pointer to the list of successors, nullptr if index does not exist in the graph
    const AdjacencyList* getSuccessors(Index_t index)
    {
        if (index >= 0 && index < static_cast<Index_t>(numberOfVertices()))
        {
            return &m_vertices[index].successors;
        }
        return nullptr;
    }

    /// get the direct predecessors of a given vertex index in the graph
    /// @param [in] index index of vertex of which the predecessors are requested
    /// @return pointer to the list of predecessors, nullptr if index does not exist in the graph
    const AdjacencyList* getPredecessors(Index_t index)
    {
        if (index >= 0 && index < static_cast<Index_t>(numberOfVertices()))
        {
            return &m_vertices[index].predecessors;
        }
        return nullptr;
    }


    /// get the source vertices of the graph, i.e. vertices without incoming edges
    /// @return vector filled with source vertices (might be empty if e.g. the graph is a cycle)
    iox::cxx::vector<VertexType*, VERTEX_LIMIT> getSources()
    {
        iox::cxx::vector<VertexType*, VERTEX_LIMIT> result;
        for (auto& vertexData : m_vertices)
        {
            if (vertexData.predecessors.size() == 0)
            {
                result.emplace_back(vertexData.vertex);
            }
        }
        return result;
    }

    /// get the sink vertices of the graph, i.e. vertices without outgoing edges
    /// @return vector filled with sink vertices (might be empty if e.g. the graph is a cycle)
    iox::cxx::vector<VertexType*, VERTEX_LIMIT> getSinks()
    {
        iox::cxx::vector<VertexType*, VERTEX_LIMIT> result;
        for (auto& vertexData : m_vertices)
        {
            if (vertexData.successors.size() == 0)
            {
                result.emplace_back(vertexData.vertex);
            }
        }
        return result;
    }

    /// check whether the given vertex is a source
    /// @param [in] vertex to be checked
    /// @return true iff the vertex is a source
    bool isSource(VertexType const* vertex)
    {
        auto index = findVertex(vertex);
        if (isValid(index))
        {
            if (m_vertices[index].predecessors.size() == 0)
            {
                return true;
            }
        }
        return false;
    }

    /// check whether the given vertex is a sink
    /// @param [in] vertex to be checked
    /// @return true iff the vertex is a sink
    bool isSink(VertexType const* vertex)
    {
        auto index = findVertex(vertex);
        if (isValid(index))
        {
            if (m_vertices[index].successors.size() == 0)
            {
                return true;
            }
        }
        return false;
    }


    /// get the number of vertices
    /// @return number of vertices
    size_t numberOfVertices()
    {
        return m_vertices.size();
    }

    /// get the number of edges
    /// @return number of edges
    size_t numberOfEdges()
    {
        return m_numEdges;
    }

  protected:
    using AdjacencyIndexList = iox::cxx::vector<Index_t, DEGREE_LIMIT>;

    struct VertexData
    {
        explicit VertexData(VertexType* vertex)
            : vertex(vertex)
        {
        }

        VertexType* vertex;

        AdjacencyIndexList predecessorIndices; // indices to navigate the graph
        AdjacencyIndexList successorIndices;

        AdjacencyList predecessors; // values to provide references externally
        AdjacencyList successors;
    };

    iox::cxx::vector<VertexData, VERTEX_LIMIT> m_vertices;
    size_t m_numEdges{0};

    // static assert to check comparison operator?
    // requires ==operator
    Index_t findVertex(VertexType const* vertex) const
    {
        // linear search for now, could be improved using binary search if we store the vertices sorted
        // (but would require insertion in the middle)
        Index_t n = static_cast<Index_t>(m_vertices.size());
        for (Index_t i = 0; i < n; ++i)
        {
            const auto& compareVertex = m_vertices[i].vertex;
            if (vertex == compareVertex)
            {
                return i;
            }
        }
        return INVALID_INDEX; // not found
    }

    bool isValid(Index_t index)
    {
        return index >= 0 && index < static_cast<Index_t>(m_vertices.size());
    }
};
