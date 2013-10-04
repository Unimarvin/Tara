/*****************************************************************************\
 BSD -- generating BSD automata

 Copyright (c) 2013 Simon Heiden

 BSD is free software: you can redistribute it and/or modify it under the
 terms of the GNU Affero General Public License as published by the Free
 Software Foundation, either version 3 of the License, or (at your option)
 any later version.

 BSD is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
 more details.

 You should have received a copy of the GNU Affero General Public License
 along with BSD.  If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/


#include <config.h>
#include <string>
#include <sstream>
#include "BSD.h"
#include "InnerMarking.h"
#include "Label.h"

#include "verbose.h"

using std::set;
using std::vector;


/******************
 * STATIC MEMBERS *
 ******************/


std::list<BSDNode *>* BSD::graph = NULL;
BSDNode* BSD::U = NULL;
BSDNode* BSD::emptyset = NULL;

MarkingList* BSD::templist = NULL;

std::map<InnerMarking_ID, int> * BSD::dfs = NULL;
std::map<InnerMarking_ID, int> * BSD::lowlink = NULL;
int BSD::maxdfs = 0;
std::stack<InnerMarking_ID> * BSD::S = NULL;
std::map<InnerMarking_ID, bool>* BSD::inStack = NULL;

std::list<unsigned int>* BSD::bisimtemp = NULL;


/******************
 * STATIC METHODS *
 ******************/

/*!
 This function sets up the BSD automaton graph structure and the U node and the empty node.
 */
void BSD::initialize() {
	// the BSD automaton
	graph = new std::list<BSDNode *>;

	// a temporary list to save already visited markings
	templist = new MarkingList;

	dfs = new std::map<InnerMarking_ID, int>;
	lowlink = new std::map<InnerMarking_ID, int>;
	S = new std::stack<InnerMarking_ID>;
	inStack = new std::map<InnerMarking_ID, bool>;
}

void BSD::finalize() {
	delete templist;
	delete dfs;
	delete lowlink;
	delete S;
	delete inStack;
}


/*========================================================
 *-------------------- BSD computation -------------------
 *========================================================*/

/*!
 \brief Creates the BSD automaton based on the given reachability graph.

 */
void BSD::computeBSD() {
	// clear existing graph
	graph->clear();

	std::list<MarkingList> tempSCC;

	// the U node
	U = new BSDNode;
	U->pointer = new BSDNode*[Label::events+1];
	U->isU = true;
	assignLambda(U, tempSCC);

	// the empty node
	emptyset = new BSDNode;
	emptyset->pointer = new BSDNode*[Label::events+1];
	emptyset->isU = false;
	assignLambda(emptyset, tempSCC);

	// set up the pointers ( which are all loops to the same nodes )
	for (Label_ID id = 2; id <= Label::events; ++id) {
		U->pointer[id] = U;
		emptyset->pointer[id] = emptyset;
	}

	// start with the initial marking
	std::list<MarkingList>* SCCs = computeClosureTarjan(0);

	// if the bound was broken in the initial node then the BSD automaton consists of only
	// the U node
	if (SCCs == NULL) {
		status("bound broken in initial node...");
		graph->push_back(U);
		return;
	}

	setup(*SCCs);
	delete SCCs;

	// iterate through the graph (start with the initial)
	// new nodes will be inserted on the fly at the back of the list
	std::list<BSDNode *>::iterator it = graph->begin();
	while (it != graph->end()) {
		// iterate through all the labels (except of \tau=0 and bound_broken=1)
		for (Label_ID id = 2/*sic!*/; id <= Label::events; ++id) {
			// compute the successor node after taking a step with current label
			computeSuccessor(**it, id);
		}
		++it;
	}

	// add the U node and the empty node at the back of the node list
	graph->push_back(U);
	graph->push_back(emptyset);
}


/*!
 \brief Compute the successor node of the given BSD node after performing a step with given label (if possible)

 \param[in]	node	the current node of the BSD automaton
 \param[in]	label	the label id

 \return pointer to computed BSD node or NULL if the bound was broken or if no step was possible
 */
void BSD::computeSuccessor(BSDNode &node, Label_ID label) {
	status("computing closure of BSD node %x after step with label %s", &node, Label::id2name[label].c_str());
	std::list<MarkingList> resultlist;
	// iterate through all marking ids in the BSD node
	for (MarkingList::const_iterator it = node.list.begin(); it != node.list.end(); ++it) {
		status("node %x, visiting marking id: %u", &node, *it);
		// iterate through all successors of the marking
		for (uint8_t i = 0; i < InnerMarking::inner_markings[*it]->out_degree; ++i) {
			// if there is a successor with the given label id then compute the closure of the successor marking
			if (InnerMarking::inner_markings[*it]->labels[i] == label) {
				std::list<MarkingList>* SCCs = computeClosureTarjan(InnerMarking::inner_markings[*it]->successors[i]);

				// if the bound was broken by taking a step with the current label then add a pointer from this node
				// to the U node with the current label and abort the computation
				if (SCCs == NULL) {
					node.pointer[label] = U;
					return;
				}

				// merge the closure with the already computed closures
				mergeSCCsWithoutDuplicates(resultlist, *SCCs);
				delete SCCs;

				// if label was found and the step was performed then break the for-loop
				break;
			}
		}
	}

	// if no step was possible then add a pointer from this node to the empty node with the current label
	if (resultlist.empty()) {
		node.pointer[label] = emptyset;
		return;
	}

	// else test if the node already exists and add a pointer from this node to the inserted (or existing) node
	node.pointer[label] = setup(resultlist);
}


/*!
 \brief Given a list of SCCs as input, this function checks if the node is already in the graph and if not
 	 	 it creates a new node and assigns a lambda value to it. It also sets up the other needed structures.

 \param[in]	SCCs	list of marking lists (SCCs)

 \return boolean value showing if the bound was broken
 */
BSDNode* BSD::setup(std::list<MarkingList> &SCCs) {
	MarkingList list;
	for (std::list<MarkingList>::const_iterator it = SCCs.begin(); it != SCCs.end(); ++it) {
		list.insert(list.end(), it->begin(), it->end());
	}
	list.sort();

	// check for the node in the graph and if not present insert it
	BSDNode* p = NULL;

	// the size of the list of markings (closure) in the given node
	unsigned int nodesize = list.size();

	// iterate through the graph
	for (std::list<BSDNode *>::iterator it = graph->begin(); it != graph->end(); ++it) {
		// we only have to test nodes with same sized marking lists
		if (nodesize == (**it).list.size()) {
			// check for equality of the nodes. If so then delete the given node and
			// return a pointer to the found node in the graph
			if (checkEquality(list, (**it).list)) {
				p = *it;
			}
		}
	}

	// if the node wasn't found in the graph then insert it at the back of the list
	// and set up the pointers (structure)
	// and we may as well compute the lambda values here
	if (p == NULL) {
		// set up the result node and return it
		graph->push_back(new BSDNode);
		p = graph->back();
		p->list = list;
		p->isU = false;
		p->pointer = new BSDNode*[Label::events+1];

		assignLambda(p, SCCs);
	}

	return p;
}


/*!
 \brief Compute tau-closure of a given reachable marking with Tarjan's algorithm

 \param[in]	id	the id of a marking

 \return list of SCCs (sorted) in the closure or NULL if bound was broken
 */
std::list<MarkingList>* BSD::computeClosureTarjan(InnerMarking_ID id) {
	// clear all helper structures
	dfs->clear();
	lowlink->clear();
	inStack->clear();

	templist->clear();

	// input: graph G = (node->list (V), E)

	maxdfs = 0;						// counter for dfs

	// make sure the stack is empty
	while (!S->empty())
		S->pop();

	return tarjanClosure(id);		// the call to tarjan visits all markings reachable from v
}


/*!
 \brief recursive and for BSD computation adjusted tarjan algorithm

 \param[in]	markingID	the id of a marking

 \return list of SCCs (sorted) in the closure or NULL if bound was broken
 */
std::list<MarkingList>* BSD::tarjanClosure(InnerMarking_ID markingID) {
	status("visiting marking %u", markingID);
	// add the current marking to the closure
	templist->push_back(markingID);

	(*dfs)[markingID] = maxdfs;			// set dfs index of current marking v
	(*lowlink)[markingID] = maxdfs;		// v.lowlink <= v.dfs
	maxdfs++;							// increment counter
	S->push(markingID);					// push v on top of stack
	(*inStack)[markingID] = true;		// set to true (v is in stack)

	std::list<MarkingList>* result = new std::list<MarkingList>;

	// iterate through neighbour markings v' of v
	status("\titerating through successors of marking %u:", markingID);
	for (uint8_t i = 0; i < InnerMarking::inner_markings[markingID]->out_degree; ++i) {
		// if the given bound is broken return true
		if (InnerMarking::inner_markings[markingID]->labels[i] == BOUND) {
			status("\tbound broken from marking %u", markingID);
			delete result;
			return NULL;
		}

		InnerMarking_ID idNeighbour = InnerMarking::inner_markings[markingID]->successors[i];
		// only consider \tau-steps (other steps don't matter for the closures)
		if (InnerMarking::inner_markings[markingID]->labels[i] == TAU) {
			status("\ttau step possible from marking %u to marking %u", markingID, InnerMarking::inner_markings[markingID]->successors[i]);
			bool visited = false; // v' visited?

			// test if the marking was already visited
			for (MarkingList::const_iterator it = templist->begin(); it != templist->end(); ++it) {
				if (*it == idNeighbour) {
					status("marking already visited");
					visited = true;
					break;
				}
			}

			if (!visited) {
				std::list<MarkingList>* temp = tarjanClosure(idNeighbour);	// recursive call

				if (temp == NULL) {
					status("\tbound broken (recursive abort, marking %u)", markingID);
					delete result;
					return NULL;
				}

				result->insert(result->end(), temp->begin(), temp->end());		// insert into the result list
				delete temp;

				(*lowlink)[markingID] = std::min((*lowlink)[markingID], (*lowlink)[idNeighbour]); // v.lowlink := min(v.lowlink, v'.lowlink);
			} else { // v' is visited
				// if v' is in the stack S
				if ((*inStack)[idNeighbour]) {
					(*lowlink)[markingID] = std::min((*lowlink)[markingID], (*dfs)[idNeighbour]); // v.lowlink := min(v.lowlink, v'.dfs);
				}
			}
		}
	}

	// if v.lowlink == v.dfs
	if ((*lowlink)[markingID] == (*dfs)[markingID]) { // root of a SCC
		MarkingList SCC;
		// compute the SCC
		InnerMarking_ID id = 0;
		do {
			id = S->top();			// take top of stack v*
			S->pop();				// remove top of stack
			(*inStack)[id] = false;	// set to false (v* isn't in stack any more)
			SCC.push_back(id);		// add v* to the SCC
		} while (id != markingID);

		SCC.sort();
		result->push_back(SCC);
	}

	return result;
}


/*!
 \brief merge two lists of (sorted) marking lists (SCCs) and skip duplicates

 \param[in]	result	the first list (which will also be used as the resulting list after merging)
 \param[in]	temp	the second list
 */
void BSD::mergeSCCsWithoutDuplicates(std::list<MarkingList> &result, std::list<MarkingList> &temp) {
	std::list<MarkingList> temporary;

	// iterate through the second list whose elements shall be inserted into the first list
	for (std::list<MarkingList>::iterator ittemp = temp.begin(); ittemp != temp.end(); ++ittemp) {
		// found an equal list?
		bool found_equal = false;
		unsigned int temp_size = ittemp->size();
		// iterate through the second list and look for the element of the first list
		for (std::list<MarkingList>::iterator itresult = result.begin(); itresult != result.end(); ++itresult) {
			// only check lists of equal size
			if (temp_size == itresult->size()) {
				found_equal = checkEquality(*ittemp, *itresult);
				// if the two lists are equal then break the inner for-loop
				if (found_equal)
					break;
			}
		}

		// if the element is not in the result list save it temporarily for later insertion
		if (!found_equal) {
			temporary.push_back(*ittemp);
		}
	}

	// insert all elements from the second list which weren't found in the first list
	result.insert(result.end(), temporary.begin(), temporary.end());
}


/*!
 \brief Check if the two given (sorted AND same size!!!) marking lists are equal

 \param[in]	list1	marking list 1
 \param[in]	list2	marking list 2

 \return boolean value showing if lists are equal or not
 */
bool BSD::checkEquality(MarkingList &list1, MarkingList &list2) {
	MarkingList::iterator it1 = list1.begin();
	MarkingList::iterator it2 = list2.begin();

	// iterate through the lists
	while (it1 != list1.end() /*&& it2 != list2.end()*/) { //shortened because we know that both lists are of the same size
		// check for equality of elements
		if (*it1 != *it2) {
			return false;
		}
		++it1;
		++it2;
	}

	// all elements are equal so return true
	return true;
}


/*!
 \brief Assign the lambda value to a node of the graph

 \param[in]	node	a node of the BSD automaton
 \param[in]	SCCs	a list of marking lists (SCCs of the node)
 */
void BSD::assignLambda(BSDNode *node, std::list<MarkingList> &SCCs) {
	if (node == U) {
		node->lambda = 0;
		return;
	}

	if (node == emptyset) {
		node->lambda = 4;
		return;
	}

	// assume that there doesn't exist a marking m that is a stop except for inputs
	node->lambda = 3;

	// iterate through all SCCs
	for (std::list<MarkingList>::const_iterator itSCC = SCCs.begin(); itSCC != SCCs.end(); ++itSCC) {
		// is there a transition that leads out of the SCC (\tau or sending label)
		bool found_outlabel = false;
		// iterate through the markings of the SCC
		for (MarkingList::const_iterator itlist = itSCC->begin(); itlist != itSCC->end(); ++itlist) {
			// iterate through all successor labels
			for (uint8_t i = 0; i < InnerMarking::inner_markings[*itlist]->out_degree; ++i) {
				// test if the label is receiving (for the environment)
				if (RECEIVING(InnerMarking::inner_markings[*itlist]->labels[i])) {
					found_outlabel = true;
					break;
				} else if (InnerMarking::inner_markings[*itlist]->labels[i] == TAU) {
					// test if the \tau-successor is in the same SCC
					bool found_succ_in_SCC = false;
					InnerMarking_ID succ = InnerMarking::inner_markings[*itlist]->successors[i];
					// iterate through the markings of the SCC
					for (MarkingList::const_iterator it = itSCC->begin(); it != itSCC->end(); ++it) {
						if (*it == succ) {
							found_succ_in_SCC = true;
							break;
						}
					}
					// if the \tau-successor is not in the same SCC we found a transition out of the SCC
					if (!found_succ_in_SCC) {
						found_outlabel = true;
						break;
					}
				}
			}
		}

		if (!found_outlabel) {
			node->lambda = 2; // found a stop except for inputs but no dead except for inputs (yet)

			bool found_final = false;
			for (MarkingList::const_iterator it = itSCC->begin(); it != itSCC->end(); ++it) {
				if (InnerMarking::inner_markings[*it]->is_final) {
					found_final = true;
					break;
				}
			}
			if (!found_final) {
				node->lambda = 1;   // found a dead except for inputs
				return;             // abort the computation
			}
		}
	}
}


/*========================================================
 *------------------- b-partner check --------------------
 *========================================================*/

/*!
 \brief Checks for two BSD automata BSD_b(N1) and BSD_b(N2) if the two nets N1 and N2 are b-partners.

 The underlying open nets have to be composable and the composed net has to be closed.

 \param[in]	graph1	the first BSD automaton
 \param[in]	graph2	the second BSD automaton

 \return boolean value showing if the underlying nets are b-partners of one another
 */
bool BSD::check_b_partner(parsedGraph & graph1, parsedGraph & graph2) {
	// check if the interface sizes differ
	if (graph1.events != graph2.events) {
		message("Size of interface differs! No bisimulation possible.");
		return false;
	}

	// check if the bounds differ
	if (graph1.bound != graph2.bound) {
		message("bound differs! (net 1 - bound: %i, net 2 - bound: %i)", graph1.bound, graph2.bound);
		return false;
	}

	status("computing mapping...");
	// compute a mapping between label ids of net 1 and net 2 if possible
	std::map<Label_ID, Label_ID>* mapping = computeMappingBSD(graph1, graph2);

	// if no mapping could be found return false
	if (mapping == NULL) {
		message("Interfaces do not match. Nets not composable");
		return false;
	}

	// create a list to store nodes of the bisimulation to be computed
	bisimtemp = new std::list<unsigned int>[graph1.nodes];

	status("computing bisimulation...");
	// compute the bisimulation
	bool result = computeBiSimBSD(graph1, graph2, 0, 0, mapping);

	printBiSim(graph1, graph2, bisimtemp);

	// delete the list and mapping
	delete[] bisimtemp;
	delete mapping;

	// return the result
	return result;
}


/*!
 \brief Computes the bisimulation relation recursively

 \param[in]	graph1	the first BSD automaton
 \param[in]	graph2	the second BSD automaton
 \param[in]	node_g1		a node of the first graph
 \param[in]	node_g2		a node of the second graph
 \param[in]	mapping		the mapping between labels of graph 1 and 2

 \return boolean value showing if the underlying nets can be b-partners of one another
 */
bool BSD::computeBiSimBSD(parsedGraph & graph1, parsedGraph & graph2, unsigned int node_g1, unsigned int node_g2, std::map<Label_ID, Label_ID> * mapping) {
	// check if the node pair was already visited
	for (std::list<unsigned int>::const_iterator it = bisimtemp[node_g1].begin(); it != bisimtemp[node_g1].end(); ++it) {
		if (*it == node_g2)
			return true;
	}

	// for all nodes (node1,node2) in the bisimulation the following has to hold:
	// lambda(node1) + lambda(node2) > 3
	if (graph1.lambdas[node_g1] + graph2.lambdas[node_g2] <= 3) {
		status("%s and %s don't meet the requirement! aborting computation...", graph1.names[node_g1].c_str(), graph2.names[node_g2].c_str());
		// return false recursively (abort)
		return false;
	} else {
		// add the node to the list of visited nodes
		bisimtemp[node_g1].push_back(node_g2);
	}

	// check successors
	for (unsigned int id = 0; id < graph1.events; ++id) {
		if (!computeBiSimBSD(graph1, graph2, graph1.pointer[node_g1][id], graph2.pointer[node_g2][(*mapping)[id]], mapping)) {
			// abort recursively
			return false;
		}
	}

	// if all went well return true
	return true;
}

/*!
 \brief compute mapping from labels of graph 1 to graph 2 (they may have different IDs for the corresponding labels)

 \param[in]	graph1	the first BSD automaton (list of BSD node pointers)
 \param[in]	graph2	the second BSD automaton (list of BSD node pointers)

 \return mapping
 */
std::map<Label_ID, Label_ID>* BSD::computeMappingBSD(parsedGraph & graph1, parsedGraph & graph2) {
	std::map<Label_ID, Label_ID>* result = new std::map<Label_ID, Label_ID>;

	// iterate through the label ids of graph 1
	for (std::map<Label_ID, std::string>::const_iterator it1 = graph1.id2name->begin(); it1 != graph1.id2name->end(); ++it1) {
//		// skip the \tau and bound_broken label
//		if (it1->first < 2)
//			continue;
		bool found = false;
		// iterate through the label ids of graph 2
		for (std::map<Label_ID, std::string>::const_iterator it2 = graph2.id2name->begin(); it2 != graph2.id2name->end(); ++it2) {
//			// skip the \tau and bound_broken label
//			if (it2->first < 2)
//				continue;
			// check for equality of the label names
			if (it1->second == it2->second) {
				found = true;
				// the label has to be sending on the one side and receiving on the other side or vice versa
				if ((*graph1.is_sending_label)[it1->first] != (*graph2.is_sending_label)[it2->first]) {
					// add the mapping from id 1 to id 2
					(*result)[it1->first] = it2->first;
					break;
				} else {
					// abort
					status("label %s of net 1 and net 2 are of equal type (sending/receiving)", it1->second.c_str(), it2->second.c_str());
					delete result;
					return NULL;
				}
			}
		}
		// if no corresponding label was found then abort
		if (!found) {
			status("label %s of net 1 doesn't match any label of net 2", it1->second.c_str());
			delete result;
			return NULL;
		}
	}

	// return the mapping
	return result;
}


/*========================================================
 *-------------------- CSD computation -------------------
 *========================================================*/

/*!
 \brief Creates the CSD automaton based on the BSD automaton.

 */
void BSD::computeCSD(BSDgraph & graph) {
	bool graphChanged = true;

	// repeat while graph changes
	while (graphChanged) {
		// assume the graph doesn't change
		graphChanged = false;

		// iterate through all nodes
		for (std::list<BSDNode *>::const_iterator it = graph.graph->begin(); it != graph.graph->end(); ++it) {
			// ignore the current node if the node is the U node
			if ((*it)->isU || *it == graph.emptyset)
				continue;

			if ((*it)->lambda == 1) {
				// iterate through the receiving labels (sending for the environment)
				bool allSuccAreU = true;
				for (unsigned int id = graph.first_send; id <= graph.last_send; ++id) {
					// if a successor is not the U node then stop the iteration, nothing to do here...
					if (!(*it)->pointer[id]->isU) {
						allSuccAreU = false;
						break;
					}
				}

				// if all sending labels successors are the U node then change the current node to the U node
				// and prepare for another round of computation (graph has changed)
				if (allSuccAreU) {
					(*it)->isU = true;
					graphChanged = true;
				}
			}

			// iterate through the sending labels (receiving for the environment)
			for (unsigned int id = graph.first_receive; id <= graph.last_receive; ++id) {
				// if a successor is the U node then stop the iteration...
				if ((*it)->pointer[id]->isU) {
					(*it)->isU = true;
					graphChanged = true;
					break;
				}
			}

		}

	}
}


/*========================================================
 *------------------- b-conformance check ----------------
 *========================================================*/

/*!
 \brief Checks for two CSD automata CSD_b(N1) and CSD_b(N2) if N1 b-conforms N2.

 The underlying open nets have to be interface-equivalent.

 \param[in]	graph1	the first CSD automaton
 \param[in]	graph2	the second CSD automaton

 \return boolean value showing if the underlying nets are b-partners of one another
 */
bool BSD::check_b_conformance(parsedGraph & graph1, parsedGraph & graph2) {
	// check if the interface sizes differ
	if (graph1.events != graph2.events) {
		message("Size of interface differs! No bisimulation possible.");
		return false;
	}

	// check if the bounds differ
	if (graph1.bound != graph2.bound) {
		message("bound differs! (net 1 - bound: %i, net 2 - bound: %i)", graph1.bound, graph2.bound);
		return false;
	}

	status("computing mapping...");
	// compute a mapping between label ids of net 1 and net 2 if possible
	std::map<Label_ID, Label_ID>* mapping = computeMappingCSD(graph1, graph2);

	// if no mapping could be found return false
	if (mapping == NULL) {
		message("Nets are not interface-equivalent.");
		return false;
	}

	// create a list to store nodes of the bisimulation to be computed
	bisimtemp = new std::list<unsigned int>[graph1.nodes];

	status("computing bisimulation...");
	// compute the bisimulation
	bool result = computeBiSimCSD(graph1, graph2, 0, 0, mapping);

	printBiSim(graph1, graph2, bisimtemp);

	// delete the list and mapping
	delete[] bisimtemp;
	delete mapping;

	// return the result
	return result;
}

/*!
 \brief Computes the bisimulation relation recursively

 \param[in]	graph1	the first CSD automaton
 \param[in]	graph2	the second CSD automaton
 \param[in]	node_g1		a node of the first graph
 \param[in]	node_g2		a node of the second graph
 \param[in]	mapping		the mapping between labels of graph 1 and 2

 \return boolean value showing if the underlying nets can be b-partners of one another
 */
bool BSD::computeBiSimCSD(parsedGraph & graph1, parsedGraph & graph2, unsigned int node_g1, unsigned int node_g2, std::map<Label_ID, Label_ID> * mapping) {
	// check if the node pair was already visited
	for (std::list<unsigned int>::const_iterator it = bisimtemp[node_g1].begin(); it != bisimtemp[node_g1].end(); ++it) {
		if (*it == node_g2)
			return true;
	}

	// for all nodes (node1,node2) in the bisimulation the following has to hold:
	// lambda(node1) >= lambda(node2)
	if (graph1.lambdas[node_g1] < graph2.lambdas[node_g2]) {
		status("%s and %s don't meet the requirement! aborting computation...", graph1.names[node_g1].c_str(), graph2.names[node_g2].c_str());
		// return false recursively (abort)
		return false;
	} else {
		// add the node to the list of visited nodes
		bisimtemp[node_g1].push_back(node_g2);
	}

	// check successors
	for (unsigned int id = 0; id < graph1.events; ++id) {
		if (!computeBiSimCSD(graph1, graph2, graph1.pointer[node_g1][id], graph2.pointer[node_g2][(*mapping)[id]], mapping)) {
			// abort recursively
			return false;
		}
	}

	// if all went well return true
	return true;
}

/*!
 \brief compute mapping from labels of graph 1 to graph 2 (they may have different IDs for the corresponding labels)

 \param[in]	graph1	the first CSD automaton (list of BSD node pointers)
 \param[in]	graph2	the second CSD automaton (list of BSD node pointers)

 \return mapping
 */
std::map<Label_ID, Label_ID>* BSD::computeMappingCSD(parsedGraph & graph1, parsedGraph & graph2) {
	std::map<Label_ID, Label_ID>* result = new std::map<Label_ID, Label_ID>;

	// iterate through the label ids of graph 1
	for (std::map<Label_ID, std::string>::const_iterator it1 = graph1.id2name->begin(); it1 != graph1.id2name->end(); ++it1) {
		bool found = false;
		// iterate through the label ids of graph 2
		for (std::map<Label_ID, std::string>::const_iterator it2 = graph2.id2name->begin(); it2 != graph2.id2name->end(); ++it2) {
			// check for equality of the label names
			if (it1->second == it2->second) {
				found = true;
				// the label has to be sending or receiving on both nets
				if ((*graph1.is_sending_label)[it1->first] == (*graph2.is_sending_label)[it2->first]) {
					// add the mapping from id 1 to id 2
					status("label %u of net 1 is mapped to label %u of net 2.", it1->first, it2->first);
					(*result)[it1->first] = it2->first;
					break;
				} else {
					// abort
					status("label %s of net 1 and net 2 are of different type (one sending/one receiving)", it1->second.c_str(), it2->second.c_str());
					delete result;
					return NULL;
				}
			}
		}
		// if no corresponding label was found then abort
		if (!found) {
			status("label %s of net 1 doesn't match any label of net 2", it1->second.c_str());
			delete result;
			return NULL;
		}
	}

	// return the mapping
	return result;
}


/*========================================================
 *--------------------- DOT to BSD parser ----------------
 *========================================================*/

/*!
 \brief Parses a DOT file which represents a BSD or CSD automaton.

 There have to be parameters in the comments specifying the bound, the node count and the event count:
 	 bound:             x
 	 nodes:             y
 	 labels (total):    z

 The initial node of the graph is assumed to be the first node parsed. There is a test on correct label
 and node count. However, there is no test on correct graph input, meaning that the parser assumes that
 for each node there is given all z labels. If some labels are missing for some nodes then they are
 obviously also missing in the parsed graph. This may lead to wrong results or segmentation faults.

 The format of the edges should be: "(node).lambda" -> "(node).lambda" [...,label="...",...];

 \param[in]	is		the input stream (the input file)

 \return the parsed graph
 */
parsedGraph * BSD::dot2graph_parse(std::istream & is) {
	std::string line;
	Label_ID idcounter = 0;
	int state = 0;
	unsigned int currentnode = 0;

	parsedGraph * graph = new parsedGraph;

	graph->id2name = new std::map<Label_ID, std::string>;
	graph->name2id = new std::map<std::string, Label_ID>;
	graph->is_sending_label = new std::map<Label_ID, bool>;

	status("parsing...");
	while (std::getline(is, line)) {
		if (state < 4) {
			// search for the bound
			if (line.find("bound:") != std::string::npos) {
				line = line.substr(line.find("bound:") + 6, line.length());
				// Skip delimiters at beginning.
				std::string::size_type start = line.find_first_not_of(" \t", 0);
				// Find first "non-delimiter".
				std::string::size_type end = line.find_first_of(" \t", start);
				graph->bound = atoi(line.substr(start, end - start).c_str());
				++state;
			}

			// search for the node count
			if (line.find("nodes:") != std::string::npos) {
				line = line.substr(line.find("nodes:") + 6, line.length());
				// Skip delimiters at beginning.
				std::string::size_type start = line.find_first_not_of(" \t", 0);
				// Find first "non-delimiter".
				std::string::size_type end = line.find_first_of(" \t", start);
				graph->nodes = atoi(line.substr(start, end - start).c_str());
				++state;
			}

			// search for the label count
			if (line.find("labels (total):") != std::string::npos) {
				line = line.substr(line.find("labels (total):") + 15, line.length());
				// Skip delimiters at beginning.
				std::string::size_type start = line.find_first_not_of(" \t", 0);
				// Find first "non-delimiter".
				std::string::size_type end = line.find_first_of(" \t", start);
				graph->events = atoi(line.substr(start, end - start).c_str());
				++state;
			}

			if (state == 3) {
				graph->names = new std::string[graph->nodes];
				graph->lambdas = new int[graph->nodes];

				graph->pointer = new unsigned int*[graph->nodes];
				for (unsigned int i = 0; i < graph->nodes; ++i)
					graph->pointer[i] = new unsigned int[graph->events];
				++state;
			}

		} else {

			// we are only interested in parsing the edges (which also have labels assigned)
			if (line.find("->") != std::string::npos && line.find("label") != std::string::npos) {
				// (should be) form of the lines: "(node).lambda" -> "(node).lambda" [...,label="...",...];
				std::list<std::string> tokens;
				// tokenize the current line
				Tokenize(line, tokens, "(). \"->\t");

				// iterate through the tokens and set node names and lambda values accordingly
				std::list<std::string>::const_iterator it = tokens.begin();
				std::string node1 = "";
				if (it != tokens.end()) {
					node1 = *it;
					// status("node 1: %s", node1.c_str());
					++it;
				} else
					continue;
				std::string lambda1 = "";
				if (it != tokens.end()) {
					lambda1 = *it;
					// status("lambda 1: %s", lambda1.c_str());
					++it;
				} else
					continue;
				std::string node2 = "";
				if (it != tokens.end()) {
					node2 = *it;
					// status("node 2: %s", node2.c_str());
					++it;
				} else
					continue;
				std::string lambda2 = "";
				if (it != tokens.end()) {
					lambda2 = *it;
					// status("lambda 2: %s", lambda2.c_str());
					++it;
				} else
					continue;

				// collect the labels and also all options
				std::string labels = "";
				std::string options = "";
				bool nextAreLabels = false;
				while (it != tokens.end()) {
					options += *it;
					if (nextAreLabels)
						labels = *it;
					if (it->find("label") != std::string::npos)
						nextAreLabels = true;
					else
						nextAreLabels = false;
					++it;
				}
				// status("labels: %s", labels.c_str());

				unsigned int p_node1 = -1;
				unsigned int p_node2 = -1;
				// iterate through the already parsed nodes and search for the two current parsed nodes
				for (unsigned int i = 0; i < currentnode; ++i) {
					// status("searching...");
					if (p_node1 == -1 && graph->names[i] == node1) {
						p_node1 = i;
					}
					if (p_node2 == -1 && graph->names[i] == node2) {
						p_node2 = i;
					}
					// stop searching if both nodes were found
					if (p_node1 != -1 && p_node2 != -1) {
						break;
					}
				}


				// if node 1 was not found then insert it into the graph
				if (p_node1 == -1) {
					if (currentnode >= graph->nodes)
						abort(11, "Parsed parameter doesn't match parsed nodes' count.");
					graph->names[currentnode] = node1;
					graph->lambdas[currentnode] = atoi(lambda1.c_str());

					// if the node is the U or empty node then set the pointers accordingly
					if (node1 == "U") {
						graph->U = currentnode;
					} else if (node1 == "empty") {
						graph->emptyset = currentnode;
					}

					p_node1 = currentnode;
					++currentnode;
				}

				// if node 2 was not found then insert it into the graph
				if (p_node2 == -1) {
					if (currentnode >= graph->nodes)
						abort(11, "Parsed parameter doesn't match parsed nodes' count.");
					graph->names[currentnode] = node2;
					graph->lambdas[currentnode] = atoi(lambda2.c_str());

					p_node2 = currentnode;
					++currentnode;
				}


				// tokenize the label string (might be several labels on one edge)
				tokens.clear();
				Tokenize(labels, tokens, ", ");
				it = tokens.begin();
				while (it != tokens.end()) {
					// status("found label: %s", it->c_str());
					// save the parsed label and set a new id if it has not yet been processed
					if (graph->name2id->find(*it) == graph->name2id->end()) {
						if (idcounter >= graph->events)
							abort(12, "Parsed parameter doesn't match parsed labels' count.");
						(*graph->name2id)[*it] = idcounter;
						(*graph->id2name)[idcounter] = *it;

						if (options.find("color=red") != std::string::npos) {
							(*graph->is_sending_label)[idcounter] = true;
						} else {
							(*graph->is_sending_label)[idcounter] = false;
						}

						// increase the label id counter
						++idcounter;
					}
					// set the pointer accordingly
					// status("set the pointer from node %u with label %u to node %u", p_node1, ((*graph->name2id)[*it]), p_node2);
					graph->pointer[p_node1][((*graph->name2id)[*it])] = p_node2;
					++it;
				}
			}
		}
	}

	if (state < 4) {
		abort(10, "Input file misses bound, nodes and/or events attributes.");
	}

	// the cases when there are less parsed nodes or labels than given in the attributes
	if (currentnode < graph->nodes)
		abort(11, "Parsed parameter doesn't match parsed nodes' count.");
	if (idcounter < graph->events)
		abort(12, "Parsed parameter doesn't match parsed labels' count.");

	return graph;
}


/*!
 \brief Tokenizes the given string into the given list of strings devided by the chars in the delimiters.

 \param[in]		str			the string to be tokenized
 \param[out]	tokens		list of tokens	(list of strings)
 \param[in]		delimiters	the delimiters	(string)
 */
void BSD::Tokenize(const std::string& str, std::list<std::string>& tokens, const std::string& delimiters) {
    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos = str.find_first_of(delimiters, lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}


/*========================================================
 *---------------------- TEST OUTPUT ---------------------
 *========================================================*/

/*!
 \brief print the BSD automaton in the shell (if verbose is switched on)

 \param[in]	graph	the BSD automaton
 */
void BSD::printParsedGraph(parsedGraph & graph) {
	std::stringstream temp (std::stringstream::in | std::stringstream::out);
	temp << std::endl;
//	temp << "U: " << graph.U << ", empty: " << graph.emptyset << std::endl;
	for (int i = 0; i < graph.nodes; ++i) {
		temp << graph.names[i] << ":  lambda: " << graph.lambdas[i] << std::endl;
		for (unsigned int id = 0; id < graph.events; ++id) {
			temp  << "    (" << (*graph.id2name)[id] << " -> " << graph.names[graph.pointer[i][id]] << "), ";
			if ((*graph.is_sending_label)[id])
				temp << "sending";
			else
				temp << "receiving";
			temp << std::endl;
		}
		temp << std::endl;
	}

	status("%s", temp.str().c_str());
}

/*!
 \brief print the bisimulation relation

 \param[in]	graph1	the first parsed automaton
 \param[in]	graph2	the second parsed automaton
 \param[in]	bisim	the bisimulation relation
 */
void BSD::printBiSim(parsedGraph & graph1, parsedGraph & graph2, std::list<unsigned int>* bisim) {
	std::stringstream temp (std::stringstream::in | std::stringstream::out);
	temp << std::endl;
//	temp << "U: " << graph.U << ", empty: " << graph.emptyset << std::endl;
	for (int i = 0; i < graph1.nodes; ++i) {
		temp << graph1.names[i] << " <-> ";
		for (std::list<unsigned int>::const_iterator it = bisim[i].begin(); it != bisim[i].end(); ++it) {
			temp << graph2.names[*it] << "; ";
		}
		temp << std::endl;
	}

	status("%s", temp.str().c_str());
}
