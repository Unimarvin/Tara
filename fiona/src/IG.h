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
 * \file    IG.h
 *
 * \brief   functions for Interaction Graphs (IG)
 *
 * \author  responsible: Daniela Weinberg <weinberg@informatik.hu-berlin.de>
 *
 * \note    This file is part of the tool Fiona and was created during the
 *          project "Tools4BPEL" at the Humboldt-Universitšt zu Berlin. See
 *          http://www.informatik.hu-berlin.de/top/tools4bpel for details.
 *
 */

#ifndef IG_H_
#define IG_H_

#include "mynew.h"
#include "CommunicationGraph.h"

class oWFN;


class interactionGraph : public CommunicationGraph {

    private:
        bool addGraphNode(GraphNodeCommon<GraphNode>*,
                          GraphNodeCommon<GraphNode>*,
                          messageMultiSet,
                          GraphEdgeType); // for IG

    public:
        // Provides user defined operator new. Needed to trace all new
        // operations on this class.
#undef new
        NEW_OPERATOR(interactionGraph)
#define new NEW_NEW

        interactionGraph(oWFN *);
        ~interactionGraph();

        bool checkMaximalEvents(messageMultiSet, GraphNode *, GraphEdgeType);

        void buildGraph();
        void buildGraph(GraphNode * );
        void buildReducedGraph(GraphNode * );

        void getActivatedEventsComputeCNF(GraphNode *, setOfMessages &, setOfMessages &);

        void calculateSuccStatesInput(messageMultiSet, GraphNode *, GraphNode *);
        void calculateSuccStatesOutput(messageMultiSet, GraphNode *, GraphNode *);

        setOfMessages combineReceivingEvents(GraphNode *, setOfMessages &);
        setOfMessages receivingBeforeSending(GraphNode *);

        void calculateSuccStatesOutputSet(messageMultiSet, GraphNode *);
        void calculateSuccStatesInputReduced(messageMultiSet, GraphNode *);
};

#endif /*IG_H_*/
