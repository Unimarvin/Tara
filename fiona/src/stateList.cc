#include "stateList.h"
#include "graph.h"
#include "graphEdge.h"
#include "vertex.h"

#include <iostream>
#include <stddef.h>
#include <string>
#include "main.h"
#include "enums.h"
#include "debug.h"

#ifdef LOG_NEW
#include "mynew.h"
#endif

using namespace std;


clause::clause(graphEdge * _edge) : edge(_edge), nextElement(NULL)  {
	
}

clause::~clause() {
	if (edge->getNode() == NULL) {
		delete edge;	
	}
}

void clause::setEdge(graphEdge * _edge) {
	if (edge != NULL) {
		if (edge->getNode() == NULL) {  // in case we have stored a "fake" edge, we delete that one
			delete edge;	
		} 
		edge = _edge;					// set the edge stored to the one given
	}	
}

//! \fn reachGraphState::reachGraphState(char * _label, stateType _type, bool isMinimal)
//! \param _label the label/ name of the state
//! \param _type type of the state (transient, deadlock)
//! \param isMinimal the state is a minimal state or not
//! \brief constructor
reachGraphState::reachGraphState(bool isMinimal) :
                state(NULL), isMin(isMinimal), firstElement(NULL) {
     	
}

//! \fn reachGraphState::reachGraphState(char * _label, stateType _type, bool isMinimal, State * s)
//! \param _label the label/ name of the state
//! \param _type type of the state (transient, deadlock)
//! \param isMinimal the state is a minimal state or not
//! \param s pointer to Karsten's state
//! \brief constructor
reachGraphState::reachGraphState(bool isMinimal, State * s) :
        state(s), isMin(isMinimal), firstElement(NULL) {
}

//! \fn reachGraphState::~reachGraphState()
//! \brief destructor
reachGraphState::~reachGraphState() {

}

//! \fn bool reachGraphState::isMinimal()
//! \return true, if the state is a minimal state, false otherwise
//! \brief returns true, if the state is a minimal state, false otherwise
bool reachGraphState::isMinimal() {
    return isMin;
}

//! \fn vertexColor reachGraphState::calcColor()
//! \return true, if the state is a bad state (it is red ;-)), false otherwise
//! \brief returns true, if the state is a bad state (it is red ;-)), false otherwise
//! a state is red, if all events it activates lead to bad nodes
vertexColor reachGraphState::calcColor() {

	trace(TRACE_5, "reachGraphState::calcColor(): start\n");

	if (firstElement == NULL) {		// since theres is no clause we can't conclude anything
		return BLACK;	
	}

	clause * cl = firstElement;
	clause * clPrev = NULL;
	bool indefinite = false;
	
	while (cl) {					// check the clause stored
		if (cl->edge->getNode() != NULL && cl->edge->getNode()->getColor() == BLUE) {
			trace(TRACE_5, "reachGraphState::calcColor(): end\n");
            return BLUE;
        } 
        if (cl->edge->getNode() == NULL || cl->edge->getNode()->getColor() == BLACK) {
            indefinite = true;
        } else {
	        if (cl->edge->getNode() != NULL && cl->edge->getNode()->getColor() == RED) {
	            // delete that literal in the clause since it points to a red node
	            if (clPrev != NULL) {
	            	clPrev->nextElement = cl->nextElement;
	            } else if (firstElement == cl) {
	            	firstElement = cl->nextElement;
	            }
	            
	            clause * clTemp = cl->nextElement;	// remember the next literal in list
          
	            delete cl;							// delete clause
	            cl = clTemp;						// get the remembered literal
	            continue ;
	        } 
        }
        clPrev = cl;			// remember this literal
		cl = cl->nextElement;	
	}
	
	trace(TRACE_5, "reachGraphState::calcColor(): end\n");
	
	if (indefinite) {
		return BLACK;
	} else {
	    return RED;
	}
}

//! \fn void reachGraphState::addClauseElement(char * clauseLabel)
//! \param clauseLabel the label to be added to the clause list of this state
//! \brief adds the given label to the clause list of this state
void reachGraphState::addClauseElement(char * label) {
  //  clauseLabel.insert(label);
	clause * cl = firstElement;
	
	graphEdge * newEdge = new graphEdge(NULL, label, sending);
	
	if (firstElement == NULL) {
		firstElement = new clause(newEdge);	// create a new clause literal	
	} else {
		while (cl->nextElement) {		// get the last literal of the clause
			cl = cl->nextElement;	
		}	  
		cl->nextElement = new clause(newEdge);	// create a new clause literal	
	} 
}

//! \fn string reachGraphState::getClause()
//! \return the clause as a string
//! \brief returns the clause as a string
string reachGraphState::getClause() {
    string clauseString = "";
    bool comma = false;

	if (firstElement == NULL) {		// since theres is no clause we can't conclude anything
		return "NULL";	
	}

	clause * cl = firstElement;

    while (cl) {
        if (cl->edge->getNode() != NULL && cl->edge->getNode()->getColor() != RED && 
        			cl->edge->getNode()->getStateList()->setOfReachGraphStates.size() > 0) {
            if (comma) {
                clauseString += "+";
            }
            if (cl->edge->getType() == sending) {
                clauseString += "!";
            } else {
                clauseString += "?";
            }
            clauseString += cl->edge->getLabel();
            comma = true;
        }    	
    	cl = cl->nextElement;	
    }
    
    return clauseString;
}

//! \fn void reachGraphState::setEdge(graphEdge * edge)
//! \param edge
//! \brief sets the the clause's edge to the given edge
void reachGraphState::setEdge(graphEdge * edge) {
  //  clauseSetLabel::iterator iter = clauseLabel.find(edge->getLabel());

 	clause * cl = firstElement;
 	
 	while (cl) {
		if  (strcmp(cl->edge->getLabel(), edge->getLabel()) == 0) {
			// we have found a pseudo edge with that label, so store the correct edge right here
			cl->setEdge(edge);				
			return;
		}
 		cl = cl->nextElement;	
 	}
}


//! \fn inline bool operator == (const reachGraphState& s1, const reachGraphState& s2)
//! \param s1 left hand state
//! \param s2 right hand state
//! \brief implements the operator == by comparing the labels/ names of the two states
inline bool operator == (const reachGraphState& s1, const reachGraphState& s2) {
    return (s1.state == s2.state);
}

//! \fn ostream& operator << (ostream& os, const reachGraphState& s)
//! \param os output stream
//! \param s state
//! \brief implements the operator << by printing the label/ name of the state to the output stream
ostream& operator << (ostream& os, const reachGraphState& s) {

    //os << "[" << s.label << "]" ;         !!! TODO!!!!!!!!!! myMarking auswerten

    return os;
}


//! \fn stateList::stateList()
//! \brief constructor
stateList::stateList() {
}

//! \fn stateList::~stateList()
//! \brief destructor
stateList::~stateList() {
	reachGraphStateSet::iterator iter;
	
	for (iter = setOfReachGraphStates.begin(); iter != setOfReachGraphStates.end(); iter++) {
		delete *iter;	
	}	
}

//! \fn bool stateList::addElement(State * s, stateType _type, bool isMinimal, char * label)
//! \param s pointer to the state (Karsten) to be added to the list
//! \param _type
//! \param isMinimal
//! \param label
//! \brief adds a state (Karsten) to the set of states of a node
bool stateList::addElement(State * s, bool isMinimal) {

    trace(TRACE_5, "stateList::addElement(State * s, bool isMinimal): start\n");

    reachGraphState *state = new reachGraphState(isMinimal, s);
    pair<reachGraphStateSet::iterator, bool> result = setOfReachGraphStates.insert(state);

    return result.second;       // returns whether the element got inserted (yes) or not (no)
}

bool stateList::findElement(State * s, bool isMinimal) {
    reachGraphState *state = new reachGraphState(isMinimal, s);

    reachGraphStateSet::iterator iter = setOfReachGraphStates.find(state);

    delete state;

    return (iter != setOfReachGraphStates.end());
}

//! \fn int stateList::elementCount()
//! \return number of elements of the list
//! \brief returns the number of elements of the list
int stateList::elementCount() {
    return setOfReachGraphStates.size();
}


//! \fn bool operator == (const stateList& s1, const stateList& s2)
//! \param s1 left hand state
//! \param s2 right hand state
//! \brief implements the operator ==
bool operator == (const stateList& s1, const stateList& s2) {
    if (s1.setOfReachGraphStates.size() == s2.setOfReachGraphStates.size()) {
        if (s1.setOfReachGraphStates.size() == 0) {
            return true;
        }
        int found = 0;
        int foundTest = 0;

        reachGraphStateSet::iterator iter1, iter2;

        for (iter1 = s1.setOfReachGraphStates.begin(); iter1 != s1.setOfReachGraphStates.end(); iter1++) {
            foundTest = found;
            for (iter2 = s2.setOfReachGraphStates.begin(); iter2 != s2.setOfReachGraphStates.end(); iter2++) {
                if ((*iter2)->state == (*iter1)->state) {
                    found++;
                }
            }
            if (foundTest == found) {   // check, if the state (*iter1) was found in the second list at all
                return false;           // no, then return false
            }
        }
        return (found == s1.setOfReachGraphStates.size());
    }

    // number of elements in both sets are not the same
    return false;
}

//! \fn bool operator < (const stateList& s1, const stateList& s2)
//! \param s1 left hand state
//! \param s2 right hand state
//! \brief implements the operator <
bool operator < (const stateList& s1, const stateList& s2) {

    if (s1.setOfReachGraphStates.size() < s2.setOfReachGraphStates.size()) {
        return true;
    } else if (s1.setOfReachGraphStates.size() > s2.setOfReachGraphStates.size()) {
        return false;
    }

    reachGraphStateSet::iterator iter1, iter2;

    iter1 = s1.setOfReachGraphStates.begin();
    iter2 = s2.setOfReachGraphStates.begin();

    while ((iter1 != s1.setOfReachGraphStates.end()) && (iter2 != s2.setOfReachGraphStates.end())) {
            if ((*iter1)->state < (*iter2)->state) {
                return true;
            }
            if ((*iter1)->state > (*iter2)->state) {
                return false;
            }
        iter1++;
        iter2++;
    }

    return false;
}

//! \fn ostream& operator << (ostream& os, const stateList& s)
//! \param os output stream
//! \param s statelist
//! \brief implements the operator << by printing the label/ name of all the states
//! of the list to the output stream
ostream& operator << (ostream& os, const stateList& s) {
    reachGraphStateSet::iterator iter;

    for (iter = s.setOfReachGraphStates.begin(); iter != s.setOfReachGraphStates.end(); iter++) {
        os << *(*iter) << "\\n";
    }
    return os;
}
