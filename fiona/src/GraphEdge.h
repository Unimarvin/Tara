/*****************************************************************************
 * Copyright 2005, 2006 Peter Massuthe, Daniela Weinberg                     *
 *                                                                           *
 * This file is part of Fiona.                                               *
 *                                                                           *
 * Fiona is free software; you can redistribute it and/or modify it          *
 * under the terms of the GNU General Public License as published by the     *
 * Free Software Foundation; either version 2 of the License, or (at your    *
 * option) any later version.                                                *
 *                                                                           *
 * Fiona is distributed in the hope that it will be useful, but WITHOUT      *
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or     *
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for  *
 * more details.                                                             *
 *                                                                           *
 * You should have received a copy of the GNU General Public License along   *
 * with Fiona; if not, write to the Free Software Foundation, Inc., 51       *
 * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.                      *
 *****************************************************************************/

/*!
 * \file    GraphEdge.h
 *
 * \brief   functions for traversing IG / OG
 *
 * \author  responsible: Daniela Weinberg <weinberg@informatik.hu-berlin.de>
 *
 * \note    This file is part of the tool Fiona and was created during the
 *          project "Tools4BPEL" at the Humboldt-Universitšt zu Berlin. See
 *          http://www.informatik.hu-berlin.de/top/tools4bpel for details.
 *
 */

#ifndef GRAPHEDGE_H
#define GRAPHEDGE_H

#include "mynew.h"
#include <string>

class GraphNode;

//! Possible types of a GraphEdge.
enum GraphEdgeType {SENDING, RECEIVING};

/**
 * @todo TODO: Turn this template class into a normal class with GraphNodeType =
 * GraphNode when GraphNode and OGFromFileNode are merged.
 */
template<typename GraphNodeType = GraphNode>
class GraphEdge {
private:
    /**
     * Points to the destination node of this edge.
     */
    GraphNodeType* dstNode;

    /** Label of the edge. */
    std::string label;

public:
    /**
     * Constructs a GraphEdge.
     * @param dstNodeP Points to the destination of this edge.
     * @param labelP label of this edge.
     */
    GraphEdge(GraphNodeType* dstNodeP, const std::string& labelP);

    /**
     * Returns the label of this edge.
     */
    std::string getLabel() const;

    /**
     * Returns the type if this edge. (RECEIVING or SENDING)
     */
    GraphEdgeType getType() const;

    /**
     * Returns the node this edge points to.
     */
    GraphNodeType* getDstNode() const;

#undef new
    /**
     * Provides user defined operator new. Needed to trace all new operations
     * on this class.
     */
    NEW_OPERATOR(GraphEdge)
#define new NEW_NEW
};


template<typename GraphNodeType>
GraphEdge<GraphNodeType>::GraphEdge(GraphNodeType* dstNodeP,
    const std::string& labelP) :
    dstNode(dstNodeP),
    label(labelP) {
}


template<typename GraphNodeType>
GraphNodeType* GraphEdge<GraphNodeType>::getDstNode() const {
    return dstNode;
}


template<typename GraphNodeType>
std::string GraphEdge<GraphNodeType>::getLabel() const {
    return label;
}


#endif //GRAPHEDGE_H

