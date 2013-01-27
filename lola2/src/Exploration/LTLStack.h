#pragma once

#include <config.h>

class AutomataTree;

/*!
 \brief one element on the stack for LTL checks
 */
class LTLStackEntry {
public:
	/// ordinary constructor for entry
	LTLStackEntry(index_t * f, index_t cf, index_t * _states, index_t cs, index_t _lowlink, AutomataTree* _dfs) {
		assert(f); //firelist, first processed in firelist
		fl = f;
		current_on_firelist = cf;
		states = _states;
		current_on_statelist = cs;
		dfs = _dfs;
		lowlink = _lowlink;
	}
	/// copy constructor used by the search stack
	LTLStackEntry(LTLStackEntry& src) {
		// copy the states
		current_on_statelist = src.current_on_statelist;
		states = new index_t[current_on_statelist + 1];
		assert(states);
		assert(src.states);
		memcpy(states, src.states,(current_on_statelist + 1) * SIZEOF_INDEX_T);
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
	// value of the dfs and lowlink numbers
	AutomataTree* dfs;
	index_t lowlink;

};


/*!
 \brief one element on the stack for LTL fairness checks
 */
class LTLFairnessStackEntry {
public:
	/// ordinary constructor for entry
	LTLFairnessStackEntry(index_t * f, index_t cf, index_t * _states, index_t cs) {
		assert(f); //firelist, first processed in firelist
		fl = f;
		current_on_firelist = cf;
		states = _states;
		current_on_statelist = cs;
	}
	/// copy constructor used by the search stack
	LTLFairnessStackEntry(LTLFairnessStackEntry& src) {
		// copy the states
		current_on_statelist = src.current_on_statelist;
		states = new index_t[current_on_statelist + 1];
		assert(states);
		assert(src.states);
			memcpy(states, src.states,(current_on_statelist + 1) * SIZEOF_INDEX_T);

		// copy the firelist
		current_on_firelist = src.current_on_firelist;
		fl = new index_t[current_on_firelist + 1];
		assert(fl);
		assert(src.fl);
		memcpy(fl, src.fl, (current_on_firelist + 1) * SIZEOF_INDEX_T);
	}
	~LTLFairnessStackEntry() {
		if (fl)
			delete[] fl;
		if (states)
			delete[] states;
		fl = NULL;
		states = NULL;
	}
	index_t * fl; // array to take a firelist
	index_t current_on_firelist; // index of first processed element of fl
	index_t * states; // array to take a statelist
	index_t current_on_statelist; // index of first processed element of states
	// value of the dfs and lowlink numbers
};
