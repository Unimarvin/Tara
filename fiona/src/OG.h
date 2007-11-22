/*****************************************************************************
 * Copyright 2005, 2006, 2007 Peter Massuthe, Daniela Weinberg               *
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
 * \file    OG.h
 *
 * \brief   functions for Operating Guidelines (OG)
 *
 * \author  responsible: Peter Massuthe <massuthe@informatik.hu-berlin.de>
 *
 * \note    This file is part of the tool Fiona and was created during the
 *          project "Tools4BPEL" at the Humboldt-Universitšt zu Berlin. See
 *          http://www.informatik.hu-berlin.de/top/tools4bpel for details.
 *
 */

#ifndef OG_H_
#define OG_H_

#include "mynew.h"
#include "CommunicationGraph.h"
#include "BddRepresentation.h"
#include <map>

class oWFN;


class OG : public CommunicationGraph {
    private:
        /// calculates the annotation (CNF) for the node
        void computeCNF(GraphNode* node) const;

        /// Builds the OG of the associated PN recursively starting at
        /// currentNode.
        void buildGraph(GraphNode* currentNode, double progress_plus);

        /// calculates the set of successor states in case of an input message
        void calculateSuccStatesInput(unsigned int, GraphNode*, GraphNode*);
        
        /// calculates the set of successor states in case of an output message
        void calculateSuccStatesOutput(unsigned int, GraphNode*, GraphNode*);

        /// adds a node to the OG
        void addGraphNode(GraphNode*, GraphNode*); // for OG
        
        /// creates an edge in the OG
        void addGraphEdge(GraphNode*,
                          GraphNode*,
                          oWFN::Places_t::size_type,
                          GraphEdgeType); // for OG

    public:
        /// constructor
        OG(oWFN *);
        
        /// basic deconstructor
        ~OG();

        BddRepresentation * bdd;

        /// Turns all blue nodes that should be red into red ones and
        /// simplifies their annotations by removing unneeded literals.
        void correctNodeColorsAndShortenAnnotations();

        /// Builds the OG of the associated PN
        void buildGraph();

        /// converts an OG into its BDD representation
        void convertToBdd();

        /// converts an OG into its BDD representation including the red nodes and the markings of the nodes
        void convertToBddFull();


        // Provides user defined operator new. Needed to trace all new operations on this class.
#undef new
        NEW_OPERATOR(OG)
#define new NEW_NEW
};


#endif /*OG_H_*/
