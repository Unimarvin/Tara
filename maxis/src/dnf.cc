/*****************************************************************************\
 Maxis -- Construct maximal services

 Copyright (C) 2010  Jarungjit Parnjai <parnjai@informatik.hu-berlin.de>

 Maxis is free software: you can redistribute it and/or modify it under the
 terms of the GNU Affero General Public License as published by the Free
 Software Foundation, either version 3 of the License, or (at your option)
 any later version.

 Maxis is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
 more details.

 You should have received a copy of the GNU Affero General Public License
 along with Maxis.  If not, see <http://www.gnu.org/licenses/>.
\*****************************************************************************/

#include <string>
#include <set>
#include <vector>
#include <stack>
#include <queue>
#include <iostream>
#include <sstream>
#include <cassert>

#include "dnf.h"
#include "util.h"

using std::string;
using std::set;
using std::stack;
using std::queue;
using std::vector;
using std::cerr;
using std::endl;
using std::pair;

/*!
 * \brief constructor
 *
 * \note  if left or right child is also a conjunction,
 *        it will be merged with this node.
 */
PartialOrder::PartialOrder(Formula * f, set<string> & cl) :
		aFormula(f->clone()), clause(cl) {
}
/*!
 * \brief destructor
 *
 * \note  children will also be deleted.
 */
PartialOrder::~PartialOrder() {
	delete(aFormula);
}

set<string> PartialOrder::get_conjunctive_string() const { return clause; }

set<PartialOrder *> PartialOrder::get_poset() const { return poset; }

bool PartialOrder::has_child() { return (!poset.empty()); }

void PartialOrder::insert_poset(PartialOrder *p) { poset.insert(p); }

void PartialOrder::remove_poset(PartialOrder *p)
{
	set<PartialOrder *>::iterator findit = poset.find(p);
	if (findit!=poset.end())
		poset.erase(p);
}

Formula * PartialOrder::get_aFormula() { return aFormula->clone(); }

std::ostream & PartialOrder::out(std::ostream & os) {
  os << "(";
  string delim = "";
  for(set<string>::iterator it=clause.begin(); it!=clause.end(); ++it) {
	  os << delim << it->data();
	  delim = ", ";
  }
  os << ") ";
  return os;
}


/* looks up search_string in the po structure
 * if the po structure already contains exact search string, then
 *    return the po pointers for both found_element and parent_element
 * otherwise,
 *    return found_element and parent element pointers for placing the search element
 */
pair<PartialOrder *, PartialOrder *> DNF::lookup(const PartialOrder * search_element, std::ostream  & os ) const {

	typedef pair<set<PartialOrder *>, set<PartialOrder *>::iterator > TraversePair;

	stack< PartialOrder *> parentStack;
	stack< TraversePair > lookupStack;

	parentStack.push(po);

	set<PartialOrder *> neighbors = po->get_poset();
	set<PartialOrder *>::iterator neighbors_iterator = neighbors.begin();
	lookupStack.push(TraversePair (neighbors, neighbors_iterator));

	set<string> temp_string = po->get_conjunctive_string();
	set<string> search_string = search_element->get_conjunctive_string();

	os << "======================" << endl;
	os << "search_string = (";
	for (set<string>::iterator sit = search_string.begin(); sit != search_string.end(); ++sit)
		os << *sit << ", ";
	os << ")" << endl;

	bool duplicated = false;
	bool found = false;
	PartialOrder * temp_element = NULL;
	while (!lookupStack.empty() && !duplicated && !found)
	{
		TraversePair tpair = lookupStack.top();
		lookupStack.pop();
		set<PartialOrder *> temp_set = tpair.first;
		set<PartialOrder *>::iterator temp_iterator = tpair.second;

/*		os << "temp_set = ";

		os << "(";
		for (set<PartialOrder *>::iterator tsit = temp_set.begin(); tsit != temp_set.end(); ++tsit)
			(*tsit)->out(os) << ", ";
		os << ")" << endl; // << "temp_iterator = ";
*/
		while (*temp_iterator != *temp_set.end() )
		{
/*			os << "temp_iterator = " << *(temp_iterator) << endl;
			os << "temp_set.end() = " << *(temp_set.end()) << endl;
*/
			temp_element = * temp_iterator;
			temp_string = temp_element->get_conjunctive_string();

			os << "temp_string = (";
			for (set<string>::iterator it = temp_string.begin(); it != temp_string.end(); ++it)
				os << *it << ", ";
			os << ")" << endl;

			pair<bool, bool> result = util::setRelation(temp_string, search_string);
			bool superset = result.first;
			bool subset = result.second;
			if (superset && subset) {
				// found a duplicate conjunction of literals
//				os << "duplicated!" <<endl;
				duplicated = true;
				break;
			}
			if (!superset && subset) {
				// temp_string is a subset of search_string,
//				os << "temp_string is a subset of search_string" << endl;

				if (search_string.size()-temp_string.size()==1) {
					found = true;
					parentStack.push(temp_element);
					temp_element = NULL;
					break;
				}

				// otherwise, go deeper
				parentStack.push( *temp_iterator );
				// push current lookup entry back into lookupStack
				lookupStack.push( TraversePair (temp_set, temp_iterator) );

				// push a children into lookupStack
				set<PartialOrder *> children = temp_element->get_poset();
				if (!children.empty()) {
					set<PartialOrder *>::iterator children_iterator = children.begin();
					lookupStack.push(TraversePair (children, children_iterator));
				}
				break;
			}
			if (superset && !subset) {
//				os << "temp_string is a superset of search_string" << endl;

				if (search_string.size()-temp_string.size()==1) {
					found = true;
					parentStack.push(temp_element);
					break;
				}

				// temp_string is a superset of search_string, traverse deeper
				parentStack.push( *temp_iterator );
				// push current lookup entry back into lookupStack
				lookupStack.push( TraversePair (temp_set, temp_iterator) );

				// push a current lookup entry into lookupStack
				set<PartialOrder *> children = temp_element->get_poset();
				if (!children.empty()) {
					set<PartialOrder *>::iterator children_iterator = children.begin();
					lookupStack.push(TraversePair (children, children_iterator));
//				os << "pushing non-empty children of : ";
//					temp_element->out(os) <<  ", it : ";
//					(*children_iterator)->out(os) << endl;
				}
				break;
			}
			if (!superset && !subset) {
				// wrong path, search next neighbors
//				os << "search next neighbor" << endl;
				temp_iterator++;
				continue;
			}
		}

		// back track
		if (!parentStack.empty() && !found) {
//			os << "pop : ";
//			(parentStack.top())->out(os) << endl;
			parentStack.pop();
		}
	}
/*
    os << "end loop!" << endl;
	os << "======================" << endl;
*/
	PartialOrder * parent_element = NULL;
	PartialOrder * found_element = NULL;

	if (!duplicated) {
		if (parentStack.empty()) {
//			os << "parent stack empty" << endl;
			parent_element = po;
			found_element = NULL;
		} else {
			if (found) {
//				os << "element found" << endl;
				parent_element = parentStack.top();
				found_element = temp_element;
			} else {
//				os << "element not found" << endl;
				parent_element = po;
				found_element = NULL;
			}
		}
	}
/*
	else {
		os << "duplicated!" << endl;
	}

	os << endl;
*/
	return  pair<PartialOrder *, PartialOrder *> (found_element, parent_element);
}


/// constructor
DNF::DNF(Formula * f, set<string> & literals, std::ostream & os) :
		minimal(false), maximal(false), originalFormula(f->clone()), all_literals(literals), po(NULL)  {

	PartialOrder * found;
	PartialOrder * parent;

	// root or partial order structure is always Literal "false"
	string str = "false";
	Literal * l = new Literal(& str);
	set<string> l_clause;

	// l_clause of root node is always an empty string
	l_clause.insert(str);
	po = new PartialOrder(static_cast<Formula *> (l), l_clause);

	os << "original formula = ";
	originalFormula->out(os) << endl;

	// get all children elements of an original Formula
	set<string> child_clause;
	Conjunction * c = dynamic_cast<Conjunction *> (originalFormula);
	if (c != NULL) {
		// assert : originalFormula contains only 1 clause of Conjunction
		child_clause = conjunction_string_literals(c);

		PartialOrder * child = new PartialOrder(f, child_clause);

		if (!po->has_child())  {
			po->insert_poset( child );
//			os << "first inserted under po! " << endl;
		} else {
			pair<PartialOrder *, PartialOrder *> results = lookup(child, os);
			found = results.first;
			parent = results.second;
			if (parent != NULL) {
				if ((parent == po) && (found == NULL)) {
					po->insert_poset( child );
//					os << "inserted under po! " << endl;
				} else {
					parent->insert_poset( child );
//					os << "inserted IN BETWEEN : ";
//					parent->out(os);
					if (found != NULL) {
						child->insert_poset( found );
						parent->remove_poset( found );
//						os << " and ";
//						found->out(os);
					}
//					os << endl;
				}
			} // else it is a duplicated literal, do nothing
//			else os << "do nothing!" << endl;
		}
		disjuncts.insert(originalFormula);
	} else {
		set<Formula *> children = originalFormula->getElements();
		for(set<Formula *>::iterator it=children.begin(); it!=children.end(); ++it) {

			// assert : each children is either Literal or Conjunction
			Formula * f = * it;
			Literal * l = dynamic_cast<Literal *> (f);
			if (l != NULL) {
				child_clause.insert( l->getString() );
			} else {
				c = dynamic_cast<Conjunction *> (f);
				if (c != NULL) {
					child_clause = conjunction_string_literals(c);
				}
			}

			if (!child_clause.empty()) {
				// alternatively, one can check if it is a multiple conjunction
				PartialOrder * child = new PartialOrder(f, child_clause);
				if (!po->has_child()) {
					po->insert_poset( child );
//					os << "first inserted under po! " << endl;
				} else {
					pair<PartialOrder *, PartialOrder *> results = lookup(child, os);
					found = results.first;
					parent = results.second;
					if (parent != NULL) {
						if ((parent == po) && (found == NULL)) {
							po->insert_poset( child );
//							os << "inserted under po! " << endl;
						} else {
							parent->insert_poset( child );
//							os << "inserted IN BETWEEN : ";
//							parent->out(os);
							if (found != NULL) {
								child->insert_poset( found );
								parent->remove_poset( found );
//								os << " and ";
//								found->out(os);
							}
//							os << endl;
						}
					} // else it is a duplicated conjunction, do nothing
//					else os << "do nothing!" << endl;
				}
			}
			child_clause.clear();
			disjuncts.insert(f);
		}  /* end iterator */
	}

}

// destructor
DNF::~DNF(){
	traverseDelete(po);
	disjuncts.clear();
	all_literals.clear();
}


void DNF::traverseDelete(PartialOrder * node)
{
	set<PartialOrder *> children = node->get_poset();
	for(set<PartialOrder *>::iterator it=children.begin(); it!=children.end(); ++it) {
		traverseDelete( *it );
	}
    delete(node);
}

set<string> DNF::conjunction_string_literals(const Conjunction * c) const {
	set<string> clause;

	set<Formula *> children = c->getElements();
	for(set<Formula *>::iterator it=children.begin(); it!=children.end(); ++it)  {
		Literal * l = dynamic_cast<Literal *> (* it);
		if (l != NULL)
			clause.insert(l->getString());
	}
	return clause;
}

/// construct a minimal DNF
void DNF::minimize(std::ostream & os) {
	/* maintain only the first level children
	 * #TODO : (a * true) = a
	 */

	set<PartialOrder *> children = po->get_poset();
//  os << "ps = ";
// 	po->out(os) << endl;
//	os << "children = (";

	for(set<PartialOrder *>::iterator it=children.begin(); it!=children.end(); ++it) {
//		(*it)->out(os) << ", ";

		set<PartialOrder *> grandChildren = (*it)->get_poset();
		for(set<PartialOrder *>::iterator git=grandChildren.begin(); git!=grandChildren.end(); ++git) {
//			os << "git = ";
//			(*git)->out(os) << endl;

			(*it)->remove_poset(*git);
			traverseDelete( *git );
		}
	}
//	os << ")" << endl;
}

/// construct a complete DNF
void DNF::maximize()
{
	/* #TODO */
}

/// does DNF cover a given conjunction
bool DNF::is_covered(const Conjunction &) const { return false; }

/// is the partial order set of DNF minimal
bool DNF::is_maximal() const {  return maximal; }

// is the partial order set of DNF maximal
bool DNF::is_minimal() const { return minimal;  }

/* write DNF output
 */
Disjunction * DNF::get_DNF()
{
 	Disjunction * aFormula = new Disjunction(disjuncts);
    return aFormula;
}

/* write to stream
 */
std::ostream & DNF::out(std::ostream & os)
{
	// po->out(os);
	std::ostringstream oss;
	os << "[" << traverseOutput(oss, po).str() << "]" << endl;
    return os;
}

/* preorder traversal
 */
std::ostringstream & DNF::traverseOutput(std::ostringstream & oss, PartialOrder * node)
{
	(node)->out(oss) << ", ";
    set<PartialOrder *> child = node->get_poset();
	for(set<PartialOrder *>::iterator cit=child.begin(); cit!=child.end(); ++cit) {
		traverseOutput(oss, *cit);
	}
	return oss;
}
