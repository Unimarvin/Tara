/*****************************************************************************\
 * Copyright 2006 Niels Lohmann                                              *
 *                                                                           *
 * This file is part of GNU BPEL2oWFN.                                       *
 *                                                                           *
 * GNU BPEL2oWFN is free software; you can redistribute it and/or modify it  *
 * under the terms of the GNU General Public License as published by the     *
 * Free Software Foundation; either version 2 of the License, or (at your    *
 * option) any later version.                                                *
 *                                                                           *
 * GNU BPEL2oWFN is distributed in the hope that it will be useful, but      *
 * WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General  *
 * Public License for more details.                                          *
 *                                                                           *
 * You should have received a copy of the GNU General Public License along   *
 * with GNU BPEL2oWFN; see file COPYING. if not, write to the Free Software  *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA. *
\*****************************************************************************/

/*!
 * \file    petrinet-reduction.cc
 *
 * \brief   Petri Net API: structural reduction
 *
 * \author  responsible: Niels Lohmann <nlohmann@informatik.hu-berlin.de>,
 *          last changes of: \$Author: nlohmann $
 *
 * \since   2006-03-16
 *
 * \date    \$Date: 2006/12/07 14:25:52 $
 *
 * \note    This file is part of the tool GNU BPEL2oWFN and was created during
 *          the project Tools4BPEL at the Humboldt-Universitšt zu Berlin. See
 *          http://www.informatik.hu-berlin.de/top/tools4bpel for details.
 *
 * \version \$Revision: 1.47 $
 *
 * \ingroup petrinet
 */





/******************************************************************************
 * Headers
 *****************************************************************************/

#include <list>
#include <cassert>
#include <set>

#include "petrinet.h"
#include "debug.h"		// debugging help
#include "helpers.h"
#include "options.h"





/******************************************************************************
 * Global variables
 *****************************************************************************/

set<unsigned int> visited;    // used for transitive reduction
set<unsigned int> visited2;   // used for transitive reduction





/******************************************************************************
 * Functions to structurally simplify the Petri net model
 *****************************************************************************/

/*!
 * Remove status places that are not read by any transition.
 */
void PetriNet::reduce_unused_status_places()
{
  list<Place*> unusedPlaces;

  for (set<Place*>::iterator p = P.begin(); p != P.end(); p++)
  {
    if ( postset(*p).empty() && !( (*p)->isFinal ) )
    {
      unusedPlaces.push_back(*p);
    }
  }

  // remove unused places
  for (list<Place*>::iterator p = unusedPlaces.begin(); p != unusedPlaces.end(); p++)
    if (P.find(*p) != P.end())
      removePlace(*p);
}





/*!
 * Remove transitions with empty pre or postset.
 */
void PetriNet::removeSuspiciousTransitions()
{
  list<Transition*> suspiciousTransitions;

  for (set<Transition*>::iterator t = T.begin(); t != T.end(); t++)
    if (postset(*t).empty() || preset(*t).empty())
      suspiciousTransitions.push_back(*t);

  // remove suspicious transitions
  for (list<Transition*>::iterator t = suspiciousTransitions.begin(); t != suspiciousTransitions.end(); t++)
    if (T.find(*t) != T.end())
      removeTransition(*t);
}





/*!
 * Remove structural dead nodes.
 */
void PetriNet::reduce_dead_nodes()
{
  trace(TRACE_DEBUG, "[PN]\tRemoving structurally dead nodes...\n");
  
  bool done = false;

  while (!done)
  {
    done = true;
  
    list<Place*> deadPlaces;
    list<Transition*> deadTransitions;
    list<Place*> tempPlaces;

    // find unmarked places with empty preset
    for (set<Place*>::iterator p = P.begin(); p != P.end(); p++)
    {
      if (preset(*p).empty() && (*p)->tokens == 0)
      {
	deadPlaces.push_back(*p);
	tempPlaces.push_back(*p);
	trace(TRACE_VERY_DEBUG, "[PN]\tPlace p" + toString((*p)->id) + " is structurally dead.\n");
	done = false;
      }
    }

    while (!tempPlaces.empty())
    {
      // p is a dead place
      Place* p = tempPlaces.back();
      assert(p != NULL);

      tempPlaces.pop_back();
      set<Node*> ps = postset(p);

      // transitions in the postset of a dead place are dead
      for (set<Node*>::iterator t = ps.begin(); t != ps.end(); t++)
      {
      	deadTransitions.push_back( static_cast<Transition*>(*t) );
	trace(TRACE_VERY_DEBUG, "[PN]\tTransition t" + toString((*t)->id) + " is structurally dead\n");
	done = false;
      }
    }


    // remove dead places and transitions
    for (list<Place*>::iterator p = deadPlaces.begin(); p != deadPlaces.end(); p++)
      if (P.find(*p) != P.end())
	removePlace(*p);

    for (list<Transition*>::iterator t = deadTransitions.begin(); t != deadTransitions.end(); t++)
      if (T. find(*t) != T.end())
	removeTransition(*t);

    

    // remove isolated communication places
    list<Place*> uselessInputPlaces;

    for (set<Place*>::iterator p = P_in.begin(); p != P_in.end(); p++)
      if (postset(*p).empty())
	uselessInputPlaces.push_back(*p);

    for (list<Place*>::iterator p = uselessInputPlaces.begin(); p != uselessInputPlaces.end(); p++)
      if (P_in.find(*p) != P_in.end())
	P_in.erase(*p);

    list<Place*> uselessOutputPlaces;

    for (set<Place*>::iterator p = P_out.begin(); p != P_out.end(); p++)
      if (preset(*p).empty())
	uselessOutputPlaces.push_back(*p);

    for (list<Place*>::iterator p = uselessOutputPlaces.begin(); p != uselessOutputPlaces.end(); p++)
      if (P_out.find(*p) != P_out.end())
	P_out.erase(*p);
  }
}





/*!
 * \brief Elimination of identical places (RB1):
 *
 * If there exist two distinct (precondition 1) places with identical preset
 * (precondition 2) and postset (precondition 3), then they can be merged.
 */
void PetriNet::reduce_identical_places()
{
  set<pair<string, string> > placePairs;

  trace(TRACE_DEBUG, "[PN]\tApplying rule RB1 (elimination of identical places)...\n");

  // iterate the places
  for (set<Place*>::iterator p1 = P.begin(); p1 != P.end(); p1++)
  {
    set<Node*> preSet  = preset(*p1);
    set<Node*> postSet = postset(*p1);

    if ((preSet.empty()) || (postSet.empty()))
      continue;

    for (set<Node*>:: iterator preTransition = preSet.begin(); preTransition != preSet.end(); preTransition++)
    {
      set<Node*> pPostSet = postset(*preTransition);
      for (set<Node*>::iterator p2 = pPostSet.begin(); p2 != pPostSet.end(); p2++)
	if ((*p1 != *p2) &&		// precondition 1
	    (preSet == preset(*p2)) &&	// precondition 2
	    (postSet == postset(*p2)))	// precondition 3
	{
	  string id1 = *((*p1)->history.begin());
	  string id2 = *((*p2)->history.begin());
	  placePairs.insert(pair<string, string>(id1, id2));
	}
    }
  }
  
  trace(TRACE_VERY_DEBUG, "[PN]\tFound " + toString(placePairs.size()) + " places with same preset and postset...\n");

  // merge the found transitions
  for (set<pair<string, string> >::iterator labels = placePairs.begin();
      labels != placePairs.end(); labels++)
  {
    Place* p1 = findPlace(labels->first);
    Place* p2 = findPlace(labels->second);
    mergePlaces(p1, p2);
  }
}





/*!
 * \brief Elimination of identical transitions (RB2):
 *
 * If there exist two distinct (precondition 1) transitions with identical
 * preset (precondition 2) and postset (precondition 3), then they can be merged.
 */
void PetriNet::reduce_identical_transitions()
{
  set<pair<string, string> > transitionPairs;

  trace(TRACE_DEBUG, "[PN]\tApplying rule RB2 (elimination of identical transitions)...\n");

  // iterate the transitions
  for (set<Transition*>::iterator t1 = T.begin(); t1 != T.end(); t1++)
  {
    set<Node*> preSet  = preset(*t1);
    set<Node*> postSet = postset(*t1);

    for (set<Node*>:: iterator prePlace = preSet.begin(); prePlace != preSet.end(); prePlace++)
    {
      set<Node*> pPostSet = postset(*prePlace);
      for (set<Node*>::iterator t2 = pPostSet.begin(); t2 != pPostSet.end(); t2++)
	if ((*t1 != *t2) &&		// precondition 1
	    (preSet == preset(*t2)) &&	// precondition 2
	    (postSet == postset(*t2)))	// precondition 3
	{
	  string id1 = *((*t1)->history.begin());
	  string id2 = *((*t2)->history.begin());
	  transitionPairs.insert(pair<string, string>(id1, id2));
	}
    }
  }
  
  trace(TRACE_VERY_DEBUG, "[PN]\tFound " + toString(transitionPairs.size()) + " transitions with same preset and postset...\n");

  // merge the found transitions
  for (set<pair<string, string> >::iterator labels = transitionPairs.begin();
      labels != transitionPairs.end(); labels++)
  {
    Transition* t1 = findTransition(labels->first);
    Transition* t2 = findTransition(labels->second);

    mergeTransitions(t1, t2);
  }
}





/*!
 * \brief Fusion of series places (RA1):
 *
 * If there exists a transition with singleton preset and postset
 * (precondition 1) that are distinct (precondition 2) and where the place in
 * the preset has no other outgoing arcs (precondition 3), then the places
 * can be merged and the transition can be removed. Furthermore, none of the
 * places may be communicating (precondition 4).
 */
void PetriNet::reduce_series_places()
{
  trace(TRACE_DEBUG, "[PN]\tApplying rule RA1 (fusion of series places)...\n");

  set<string> uselessTransitions;
  set<pair<string, string> > placePairs;


  // iterate the transtions
  for (set<Transition*>::iterator t = T.begin(); t != T.end(); t++)
  {
    set<Node*> postSet = postset(*t);
    set<Node*> preSet  = preset (*t);
    Place* prePlace = static_cast<Place*>(*(preSet.begin()));
    Place* postPlace = static_cast<Place*>(*(postSet.begin()));

    if ((preSet.size() == 1) &&	(postSet.size() == 1) && // precondition 1
	(prePlace != postPlace) &&			 // precondition 2
	(postset(prePlace).size() == 1) &&		 // precondition 3
	(prePlace->type == INTERNAL) &&			 // precondition 4
	(postPlace->type == INTERNAL))
   {
      string id1 = *((*(preSet.begin()))->history.begin());
      string id2 = *((*(postSet.begin()))->history.begin());
      placePairs.insert(pair<string, string>(id1, id2));
      uselessTransitions.insert(*((*t)->history.begin()));
    }
  }

  // remove useless transtions
  for (set<string>::iterator label = uselessTransitions.begin();
      label != uselessTransitions.end(); label++)
  {
    Transition* uselessTransition = findTransition(*label);
    if (uselessTransition != NULL)
      removeTransition(uselessTransition);
  }  

  // merge place pairs
  for (set<pair<string, string> >::iterator placePair = placePairs.begin();
      placePair != placePairs.end(); placePair++)
  {
    mergePlaces(placePair->first, placePair->second);
  }
}





/*!
 * \brief Fusion of series transition (RA2):
 *
 * If there exists a place with singleton preset and postset (precondition 1)
 * and if the transition in its postset has no other incoming arcs
 * (precondition 2), then the preset and the postset can be merged and the
 * place can be removed.
 */
void PetriNet::reduce_series_transitions()
{
  trace(TRACE_DEBUG, "[PN]\tApplying rule RA2 (fusion of series transitions)...\n");

  set<string> uselessPlaces;
  set<pair<string, string> > transitionPairs;


  // iterate the places
  for (set<Place*>::iterator p = P.begin(); p != P.end(); p++)
  {
    if ((postset(*p).size() == 1) && (preset(*p).size() == 1)) // precondition 1
    {
      Transition* t1 = static_cast<Transition*>(*(preset(*p).begin()));
      Transition* t2 = static_cast<Transition*>(*(postset(*p).begin()));

      if (preset(t2).size() == 1) // precondition 2
      {
	string id1 = *(t1->history.begin());
	string id2 = *(t2->history.begin());
	transitionPairs.insert(pair<string, string>(id1, id2));
	uselessPlaces.insert(*((*p)->history.begin()));
      }
    }
  }


  // remove useless places
  for (set<string>::iterator label = uselessPlaces.begin();
      label != uselessPlaces.end(); label++)
  {
    Place *uselessPlace = findPlace(*label);
    removePlace(uselessPlace);
  }


  // merge transition pairs
  for (set<pair<string, string> >::iterator transitionPair = transitionPairs.begin();
      transitionPair != transitionPairs.end(); transitionPair++)
  {
    Transition* t1 = findTransition(transitionPair->first);
    Transition* t2 = findTransition(transitionPair->second);
    mergeTransitions(t1, t2);
  }
}





/*!
 * \brief Elimination of self-loop places (RC1)
 *
 * \return number of removed places
 *
 * \pre \f$ p \f$ is a place of the net: \f$ p \in P \f$
 * \pre \f$ p \f$ is initiall marked: \f$ m_0(p) > 0 \f$
 * \pre \f$ p \f$ has one transition in its preset and one transition in its postset: \f$ |{}^\bullet p| = 1 \f$, \f$ |p^\bullet| = 1 \f$
 * \pre the transition in \f$ p \f$'s preset in the same as the transition in \f$ p \f$'s postset: \f$ p^\bullet \cap {}^\bullet p \neq \emptyset \f$
 *
 * \post \f$ p \f$ is removed: \f$ P' = P \; \backslash \; \{p\} \f$
 * \post \f$ p \f$'s ingoing and outgoing arcs are removed: \f$ F' = F \; \backslash \; \left( (\{p\}\times T) \cup (T \times \{p\}) \right) \f$
 */
unsigned int PetriNet::reduce_self_loop_places()
{
  set<string> self_loop_places;
  unsigned int result = 0;

  // find places fulfilling the preconditions
  for (set<Place *>::iterator p = P.begin(); p != P.end(); p++)
    if ((*p)->tokens > 0)
      if (postset(*p).size() == 1 && preset(*p).size() == 1)
	if (preset(*p) == postset(*p))
	  self_loop_places.insert((*p)->history[0]);

  // remove useless places
  for (set<string>::iterator label = self_loop_places.begin();
      label != self_loop_places.end(); label++)
  {
    Place *self_loop_place = findPlace(*label);
    assert(self_loop_place != NULL);
    removePlace(self_loop_place);
    result++;
  }

  return result;
}




/*!
 * Calls some simple structural reduction rules for Petri nets:
 *
 *  - Structural dead nodes are removed.
 *
 *  - Elimination of identical places (RB1)
 *  - Elimination of identical transitions (RB2)
 *  - Fusion of series places (RA1)
 *  - Fusion of series transitions (RA2)
 *  - Elimination of self-loop places (RC1)
 * 
 * \todo
 *  - (nlohmann) improve performance
 *  - (nlohmann) implement more reduction rules
 *
 */
void PetriNet::reduce()
{
  trace(TRACE_DEBUG, "[PN]\tPetri net size before simplification: " + information() + "\n");
  trace(TRACE_INFORMATION, "Simplifying Petri net...\n");

  string old = information();
  bool done = false;
  int passes = 1;
  while (!done)
  {
    reduce_dead_nodes();
    reduce_unused_status_places();
    removeSuspiciousTransitions();

    reduce_identical_places();		// RB1
    reduce_identical_transitions();	// RB2
    reduce_series_places();		// RA1
    reduce_series_transitions();	// RA2
    reduce_self_loop_places();		// RC1

//    if (parameters[P_TRED])
//      transitiveReduction();
    
    trace(TRACE_DEBUG, "[PN]\tPetri net size after simplification pass " + toString(passes++) + ": " + information() + "\n");

    done = (old == information());
    old = information();

  }

  trace(TRACE_INFORMATION, "Simplifying complete.\n");
  trace(TRACE_DEBUG, "[PN]\tPetri net size after simplification: " + information() + "\n");
}





/******************************************************************************
 * TRANSITIVE REDUCTION (alpha state)
 *****************************************************************************/

/*
// depth-first search returning the set of reachable nodes
set<unsigned int> dfs(unsigned int i, map<unsigned int, set<unsigned int> > &Adj)
{
  set<unsigned int> result;
  result.insert(i);
  visited2.insert(i);

  for (set<unsigned int>::iterator it = Adj[i].begin(); it != Adj[i].end(); it++)
  {
    if (visited2.find(*it) == visited2.end())
      result = setUnion(result, dfs(*it, Adj));
  }

  return result;
}





// creates accessibility list from adjacency list
map<unsigned int, set<unsigned int> > toAcc(map<unsigned int, set<unsigned int> > &Adj, set<unsigned int> &nodes)
{
  map<unsigned int, set<unsigned int> > result;

  for (set<unsigned int>::iterator it = nodes.begin(); it != nodes.end(); it++)
  {
    result[*it] = dfs(*it, Adj);
    result[*it].erase(*it);
    visited2.clear();
  }

  trace(TRACE_VERY_DEBUG, "[PN]\tDFS complete.\n");

  return result;
}





void prune_acc(unsigned int i, map<unsigned int, set<unsigned int> > &Acc, map<unsigned int, set<unsigned int> > &Adj)
{
  trace(TRACE_VERY_DEBUG, "[PN]\tCalling prune_acc for t" + toString(i) + "\n");

  for (set<unsigned int>::iterator it = Acc[i].begin(); it != Acc[i].end(); it++)
  {
    if (Acc[*it].empty())
      visited.insert(*it);
    else
      prune_acc(*it, Acc, Adj);
  }

  for (set<unsigned int>::iterator it = Acc[i].begin(); it != Acc[i].end(); it++)
    for (set<unsigned int>::iterator it2 = Adj[*it].begin(); it2 != Adj[*it].end(); it2++)
      if (Acc[i].find(*it2) != Acc[i].end())
	Adj[i].erase(*it2);

  visited.insert(i);
}





// wrapper functions for the transitive reduction
void PetriNet::transitiveReduction()
{
  trace(TRACE_DEBUG, "[PN]\tApplying transitive reduction...\n");
  
  set<unsigned int> nodes;
  map<unsigned int, set<unsigned int> > Adj;

  // generate the list of nodes
  for (set<Transition*>::iterator t = T.begin(); t != T.end(); t++)
    nodes.insert((*t)->id);

  // generate the adjacency lists
  for (set<Place*>::iterator p = P.begin(); p != P.end(); p++)
  {
    set<Node*> pre = preset(*p);
    set<Node*> post = postset(*p);

    if (pre.size() > 0 && post.size() > 0)
      for (set<Node*>::iterator t1 = pre.begin(); t1 != pre.end(); t1++)
	for (set<Node*>::iterator t2 = post.begin(); t2 != post.end(); t2++)
	  Adj[(*t1)->id].insert((*t2)->id);
  }

  // generate the accessibility list
  map<unsigned int, set<unsigned int> > Acc = toAcc(Adj, nodes);

  // the accessibility is base for the transitive reduction
  map<unsigned int, set<unsigned int> > Adj_reduced = Acc;

  // iterate the nodes and reduce
  for (set<unsigned int>::iterator it = nodes.begin(); it != nodes.end(); it++)
  {
    if (visited.find(*it) == visited.end())
      prune_acc(*it, Acc, Adj_reduced);
  }

  visited.clear();

  set<Place*> transitivePlaces;

  for (set<unsigned int>::iterator it = nodes.begin(); it != nodes.end(); it++)
  {
    set<unsigned int> difference = setDifference(Adj[*it], Adj_reduced[*it]);
    for (set<unsigned int>::iterator it2 = difference.begin(); it2 != difference.end(); it2++)
      transitivePlaces.insert(findPlace(*it, *it2));
  }

  int actualDeleted = 0;

  // remove transitive places
  for (set<Place*>::iterator p = transitivePlaces.begin(); p != transitivePlaces.end(); p++)
  {
    // only remove places that are not conflicting
    if ( postset(*p).size() == 1 )
    {
      actualDeleted++;
      removePlace(*p);
    }
  }

  trace(TRACE_DEBUG, "[PN]\tRemoved " + toString(actualDeleted) + " transitive places\n");
}
*/
