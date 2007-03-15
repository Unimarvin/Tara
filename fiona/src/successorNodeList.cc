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
 * \file    successorNodeList.cc
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
 
#include "mynew.h"
#include "successorNodeList.h"
#include "graphEdge.h"
#include "vertex.h"
#include "enums.h"
#include <string>
#include <iostream>

using namespace std;

//! \fn successorNodeList::successorNodeList()
//! \brief constructor 
successorNodeList::successorNodeList() : 
	firstElement(NULL),
	nextElement(NULL) {

	duringIteration = true;
}

//! \fn successorNodeList::~successorNodeList()
//! \brief destructor
successorNodeList::~successorNodeList() {
	deleteList(firstElement);
}

//! \fn void successorNodeList::deleteList(graphEdge * element)
//! \param element pointer to the next element
//! \brief deletes the whole list recursively
void successorNodeList::deleteList(graphEdge * element) {
	if (element == NULL) {
		return;
	} else {
		deleteList(element->getNextElement());
	}

	delete element;
}

//! \fn void successorNodeList::addNextNode(graphEdge * newEdge)
//! \param newEdge the edge which is to be added to the list
//! \brief adding an additional node to the end of the list
void successorNodeList::addNextNode(graphEdge * newEdge) {

	if (firstElement == NULL) {
		firstElement = newEdge;
		return;
	}

	graphEdge * tmpEdge = firstElement;

	while (tmpEdge) {
		if (tmpEdge->getNextElement() != NULL) {
			tmpEdge = tmpEdge->getNextElement();
		} else {
			tmpEdge->setNextElement(newEdge);
			return;
		}
	}
	return;
}


//! \fn void successorNodeList::resetIterating()
//! \brief resets the iteration through the list, so one can start a whole new iteration
void successorNodeList::resetIterating() {
	nextElement = NULL;
	duringIteration = true;
}


//! \fn graphEdge * successorNodeList::getNextElement()
//! \return a pointer to the next graphEdge in case of iterating through the list, NULL if the end of the list has been reached
//! \brief returns a pointer to the next graphEdge in case of iterating through the list, NULL if the end of the list has been reached
graphEdge * successorNodeList::getNextElement() {
	if (duringIteration) {
		// we have to return the next element
		if (nextElement == NULL) {
			// next element is the first one
			nextElement = firstElement;
			return nextElement;
		} else {
			// return successor if not the last node
			nextElement = nextElement->getNextElement();
			if (nextElement == NULL) {
				duringIteration = false;
			}
			return nextElement;
		}
	} else {
		// last time we gave back the last node
		nextElement = NULL;
		duringIteration = true;
		return NULL;
	}
}


//! \fn void successorNodeList::removeNodeFromList(vertex * node, bool iterating)
//! \param node a pointer to the node that shall be remove from the list
//! \param iterating flag to show whether we are iterating through the list right now
//! \brief remove the given node from the list
void successorNodeList::removeNodeFromList(vertex * node, bool iterating) {
	if (firstElement == NULL) {
		return;	
	}

	graphEdge * prevEdge = firstElement;
	graphEdge * tmpEdge = firstElement;
	
	while (tmpEdge) {
		// test if this edge is the one to be removed
		if (tmpEdge->getNode() == node) {
			if (tmpEdge == firstElement) {
				firstElement = tmpEdge->getNextElement();
				nextElement = NULL;
			} else {
				nextElement = prevEdge;
				prevEdge->setNextElement(tmpEdge->getNextElement());	
				if (iterating) {
					nextElement = prevEdge;	// in case we are iterating through the list, we have to
											// visit the previous element once again
				}				
			}
			delete tmpEdge;							// remove element from list, the node itself is not deleted
			return;
		}
		// not the one, so go on
		prevEdge = tmpEdge;
		tmpEdge = tmpEdge->getNextElement();	
	}		
}
