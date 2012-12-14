/*!
 \file LTLExploration.h
 \author Gregor
 \status new

 \brief Evaluates an LTL Property
 */

#pragma once

#include <config.h>
#include <Exploration/SearchStack.h>
#include <Exploration/ChooseTransition.h>
#include <Exploration/SimpleProperty.h>
#include <Net/NetState.h>
#include <Exploration/Firelist.h>
#include <Stores/Store.h>
#include <Formula/BuechiAutomata.h>

/// a simpe struct for a tree used as payload of the store
class AutomataTree {
public:
	uint32_t state;
	// lowlink can be maintained locally
//	uint64_t lowlink;
	uint64_t dfs;
	AutomataTree* smaller;
	AutomataTree* bigger;
};

/*!
 \brief one element on the stack for simple properties

 A simple stack entry contains two elements
 - a firelist (as an array of numbers)
 - the current index on the firelist
 */
class LTLStackEntry {
public:
	/// ordinary constructor for entry
	LTLStackEntry(index_t * f, index_t cf, index_t * _states, index_t cs,
			index_t ls, index_t _lowlink, AutomataTree* _dfs) {
		assert(f); //firelist, first processed in firelist
		fl = f;
		current_on_firelist = cf;
		states = _states;
		current_on_statelist = cs;
		length_of_statelists = ls;
		dfs = _dfs;
		lowlink = _lowlink;
	}
	/// copy constructor used by the search stack
	LTLStackEntry(LTLStackEntry& src) {
		// copy the states
		current_on_statelist = src.current_on_statelist;
		length_of_statelists = src.length_of_statelists;
		states = new index_t[current_on_statelist + 1];
		assert(states);
		assert(src.states);
		if (current_on_firelist == 0)
			memcpy(states, src.states,
					(current_on_statelist + 1) * SIZEOF_INDEX_T);
		else
			memcpy(states, src.states, (length_of_statelists) * SIZEOF_INDEX_T);

		// copy the firelist
		current_on_firelist = src.current_on_firelist;
		fl = new index_t[current_on_firelist + 1];
		assert(fl);
		assert(src.fl);
		memcpy(fl, src.fl, (current_on_firelist + 1) * SIZEOF_INDEX_T);

		dfs = src.dfs;
		lowlink = src.lowlink;

	}
	~LTLStackEntry() {
		if (fl)
			delete[] fl;
		if (states)
			delete[] states;
		fl = NULL;
		dfs = 0;
		states = NULL;
	}
	index_t * fl; // array to take a firelist
	index_t current_on_firelist; // index of first processed element of fl
	index_t * states; // array to take a statelist
	index_t current_on_statelist; // index of first processed element of states
	index_t length_of_statelists; // number on statelist, needed to reset after one iteration of
	// value of the dfs and lowlink numbers
	AutomataTree* dfs;
	index_t lowlink;

};

class LTLExploration {
public:
	LTLExploration();

	~LTLExploration();

	bool checkProperty(BuechiAutomata &automata, Store<AutomataTree> &store,
			Firelist &firelist, NetState &ns);
};