#include "mynew.h"
#include "OG.h"
#include "state.h"
#include "options.h"
#include "debug.h"
#include "successorNodeList.h"
#include "BddRepresentation.h" 
#include "CNF.h"
#include "owfn.h"
#include <vector>

//! \fn operatingGuidelines::operatingGuidelines(oWFN * _PN) 
//! \param _PN
//! \brief constructor
operatingGuidelines::operatingGuidelines(oWFN * _PN) : communicationGraph(_PN) {
	
	 if (options[O_BDD] == true) {
		unsigned int nbrLabels = PN->placeInputCnt + PN->placeOutputCnt;
		bdd = new BddRepresentation(nbrLabels, (Cudd_ReorderingType)bdd_reordermethod);
	 }
}

//! \fn operatingGuidelines::~operatingGuidelines() 
//! \brief destructor !to be implemented!
operatingGuidelines::~operatingGuidelines() {

	  if (options[O_BDD] == true) {
		delete bdd;
	  }
	  
}

//! \fn void operatingGuidelines::buildGraph(vertex * currentNode)
//! \param currentNode current node of the graph
//! \brief builds up the graph recursively
void operatingGuidelines::buildGraph(vertex * currentNode) {

	string color = "";
	
	actualDepth++;

	trace(TRACE_1, "\n=================================================================\n");
	trace(TRACE_1, "\t current node: ");
	trace(TRACE_1, intToString(currentNode->getNumber()) + ", \t current depth: " + intToString(actualDepth) + "\n");
	if (debug_level >= TRACE_2) {
		cout << "\t (" << currentNode << ")" << endl;
	}

	trace(TRACE_2, "\t number of states in node: ");
	trace(TRACE_2, intToString(currentNode->reachGraphStateSet.size()) + "\n");

	if (currentNode->getColor() == RED) {
		// this may happen due to a message bound violation
		trace(TRACE_3, "\t\t\t node " + intToString(currentNode->getNumber()) + " has color RED\n");
		trace(TRACE_1, "=================================================================\n");
		return;
	}

	// get the annotation of the node (CNF)
	computeCNF(currentNode);					// calculate CNF of this node

	if (actualDepth > PN->commDepth) {
		// we have reached a leaf node
		analyseNode(currentNode, true);
		assert(currentNode->getColor() != BLACK);
		return;
	}


//	if (terminateBuildingGraph(currentNode)) {
//		if (currentNode->getColor() == RED) {
//			color = "RED";
//		} else if (currentNode->getColor() == BLUE) {
//			color = "BLUE";
//		} else {
//			color = "BLACK";
//		}
//		trace(TRACE_3, "\t\t\t node " + intToString(currentNode->getNumber()) + " has color " + color + " (leaf)\n");		
//
//		trace(TRACE_1, "=================================================================\n");		
//		return;
//	}
	
	trace(TRACE_1, "=================================================================\n");

	int i = 0;
		
	trace(TRACE_5, "iterating over inputSet\n");
	// iterate over all elements of inputSet
	while (i < PN->placeInputCnt) { 

		trace(TRACE_2, "\t\t\t\t    sending event: !");
		trace(TRACE_2, string(PN->inputPlacesArray[i]->name) + "\n");
		
		if (currentNode->eventsUsed[i] < PN->inputPlacesArray[i]->max_occurence){
			
			vertex * v = new vertex(PN->placeInputCnt + PN->placeOutputCnt);	// create new vertex of the graph
			currentVertex = currentNode;
			
			trace(TRACE_5, "calculating successor states\n");

			bool messageboundviolation =
			calculateSuccStatesInput(PN->inputPlacesArray[i]->index, currentNode, v);

			if (messageboundviolation) {
				trace(TRACE_2, "\t\t\t\t    sending event: !");
				trace(TRACE_2, PN->inputPlacesArray[i]->name);
				trace(TRACE_2, " at node " + intToString(currentNode->getNumber()) + " suppressed\n");
				
				delete v;
			} else {

				if (AddVertex (v, i, sending)) {
					
					buildGraph(v);				// going down with sending event...
	
					trace(TRACE_1, "\t\t backtracking to node " + intToString(currentNode->getNumber()) + "\n");
	
//					analyseNode(currentNode, false);
					trace(TRACE_5, "node analysed\n");
	
					actualDepth--;
				}
			}
		} else {
			trace(TRACE_2, "\t\t\t\t    sending event: !");
			trace(TRACE_2, string(PN->inputPlacesArray[i]->name));
			trace(TRACE_2, " suppressed (max_occurence reached)\n");
		}
		i++;
	}

	i = 0;
		
	trace(TRACE_5, "iterating over outputSet\n");
	// iterate over all elements of outputSet
	while (i < PN->placeOutputCnt) { 

		trace(TRACE_2, "\t\t\t\t  receiving event: ?");
		trace(TRACE_2, string(PN->outputPlacesArray[i]->name) + "\n");
	    
		if (currentNode->eventsUsed[i + PN->placeInputCnt] < PN->outputPlacesArray[i]->max_occurence) {
				
			vertex * v = new vertex(PN->placeInputCnt + PN->placeOutputCnt);	// create new vertex of the graph
			currentVertex = currentNode;
			
			calculateSuccStatesOutput(PN->outputPlacesArray[i]->index, currentNode, v);
			
			if (AddVertex (v, i, receiving)) {

				buildGraph(v);				// going down with receiving event...

				trace(TRACE_1, "\t\t backtracking to node " + intToString(currentNode->getNumber()) + "\n");
				analyseNode(currentNode, false);
				actualDepth--;
			}
		} 
		i++;	
	}

	trace(TRACE_2, "\t\t\t\t no events left...\n");
	analyseNode(currentNode, true);

	if (currentNode->getColor() == RED) {
		color = "RED";
	} else if (currentNode->getColor() == BLUE) {
		color = "BLUE";
		
	} else {
		color = "BLACK";
	}

	trace(TRACE_3, "\t\t\t node " + intToString(currentNode->getNumber()) + " has color " + color + "\n");

/*
	if (options[O_BDD] == true){
		bdd->addOrDeleteLeavingEdges(currentNode);
		//	currentNode->resetIteratingSuccNodes();
		if (currentNode->reachGraphStateSet.size() != 0){	 
			graphEdge* element;
			//cout << "currentNode: " << currentNode->getNumber() << "\tdelete node: ";
			while((element = currentNode->getNextEdge()) != NULL){
				vertex* vNext = element->getNode();
				//cout << vNext->getNumber()<< "  ";
				setOfVertices.erase(vNext); 
				delete vNext; 
			}
			//cout << endl;
		}
	}
*/
}

//! \fn void operatingGuidelines::computeCNF(vertex * node)
//! \param node the node for which the annotation is calculated
//! \brief calculates the annotation (CNF) for the node
void operatingGuidelines::computeCNF(vertex * node) {
	
	trace(TRACE_5, "operatingGuidelines::computeCNF(vertex * node): start\n");
	StateSet::iterator iter;			// iterator over the states of the node
	
	if (!options[O_CALC_ALL_STATES]) { // in case of the state reduced graph
		
		// iterate over all states of the node
		for (iter = PN->setOfStatesTemp.begin();
			 iter != PN->setOfStatesTemp.end(); iter++) {
			if ((*iter)->type == DEADLOCK || (*iter)->type == FINALSTATE)  {
				// we just consider the maximal states only
				
				clause * cl = new clause();
				
				// get the marking of this state
				(*iter)->decodeShowOnly(PN);
							
				// get the activated output events
				for (int i = 0; i < PN->placeOutputCnt; i++) {
					if (PN->CurrentMarking[PN->outputPlacesArray[i]->index] > 0) {
						cl->addLiteral(PN->outputPlacesArray[i]->name);	
					}
				}
				
				// get all the input events
				for (int i = 0; i < PN->placeInputCnt; i++) {
					cl->addLiteral(PN->inputPlacesArray[i]->name);
				}
				
				node->addClause(cl, (*iter)->type == FINALSTATE);
			}
		}
	} else {	// no state reduction

		// iterate over all states of the node
		for (iter = node->reachGraphStateSet.begin();
			 iter != node->reachGraphStateSet.end(); iter++) {
			if ((*iter)->type == DEADLOCK || (*iter)->type == FINALSTATE)  {
				// we just consider the maximal states only
				
				clause * cl = new clause();
				
				// get the marking of this state
				(*iter)->decodeShowOnly(PN);
							
				// get the activated output events
				for (int i = 0; i < PN->placeOutputCnt; i++) {
					if (PN->CurrentMarking[PN->outputPlacesArray[i]->index] > 0) {
						cl->addLiteral(PN->outputPlacesArray[i]->name);	
					}
				}
				
				// get all the input events
				for (int i = 0; i < PN->placeInputCnt; i++) {
					cl->addLiteral(PN->inputPlacesArray[i]->name);
				}
				
				node->addClause(cl, (*iter)->type == FINALSTATE);
			}
		}
	}
		
	PN->setOfStatesTemp.clear();
	
	trace(TRACE_5, "operatingGuidelines::computeCNF(vertex * node): end\n");
}

//! \fn bool operatingGuidelines::terminateBuildingGraph(vertex * node)
//! \param node the vertex to be checked
//! \brief figure out when to terminate the building of the graph
//! return true, if building up shall be terminated, false otherwise 
//bool operatingGuidelines::terminateBuildingGraph(vertex * node) {
//	trace(TRACE_5, "bool operatingGuidelines::terminateBuildingGraph(vertex * node): start\n");
//	
//    analyseNode(node, false);
//
////	if (analyseNode(node, false) == TERMINATE) {
////		trace(TRACE_5, "node analysed\n");
////		trace(TRACE_5, "bool operatingGuidelines::terminateBuildingGraph(vertex * node): end\n");
////		return true;
////	}
//	
////	trace(TRACE_5, "node analysed, color is ");
////	if (node->getColor() == BLUE) {
////		trace(TRACE_5, "blue\n");
////	} else if (node->getColor() == RED) {
////		trace(TRACE_5, "red\n");
////	} else {
////		trace(TRACE_5, "black\n");
////	}
//	
//	
//	// do not change termination | here, but set commDepth in main.cc
//	//                           v       to desired value
//	if (actualDepth > PN->getCommDepth()) {	// we have reached the maximal communication depth
//		
//		assert(node->getNumber() > PN->getCommDepth());
//		
////		if (node->getColor() != BLUE) {
//			node->setColor(RED);
////		}
//		trace(TRACE_3, "graph reached commDepth. " + intToString(actualDepth) +" terminating\n");
//		trace(TRACE_5, "bool operatingGuidelines::terminateBuildingGraph(vertex * node): end\n");
//		return true;
//	}
//	trace(TRACE_5, "bool operatingGuidelines::terminateBuildingGraph(vertex * node): end (returning false)\n");
//	return false;
//}

void operatingGuidelines::convertToBdd() {
	trace(TRACE_5, "operatingGuidelines::convertToBdd(): start\n");
	
	vertex * tmp = root;
    bool visitedNodes[numberOfVertices];

    for (int i = 0; i < numberOfVertices; i++) {
        visitedNodes[i] = 0;
    }
   
    //unsigned int nbrLabels = PN->placeInputCnt + PN->placeOutputCnt;
    this->bdd->convertRootNode(root);
    this->bdd->generateRepresentation(tmp, visitedNodes);
    this->bdd->reorder((Cudd_ReorderingType)bdd_reordermethod);
    trace(TRACE_5,"operatingGuidelines::convertToBdd()\n");
}
