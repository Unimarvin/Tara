#ifndef NDEBUG
#include <iostream>
#include <fstream>
using std::cout;
using std::cerr;
using std::endl;
#endif

#include <cassert>
#include <sstream>

#include "automaton.h"
#include "petrinet.h"
#include "parser.h"
#include "formula.h"
#include "io.h"

using std::pair;
using std::string;
using std::ios_base;
using std::ostream;
using std::set;
using std::map;
using std::stringstream;

using pnapi::formula::Formula;

namespace pnapi
{

  namespace io
  {

    std::ios_base & owfn(std::ios_base & ios)
    {
      util::FormatData::data(ios) = util::OWFN;
      return ios;
    }

    std::istream & onwd(std::istream & ios)
    {
      util::FormatData::data(ios) = util::ONWD;
      return ios;
    }

    std::ostream & stat(std::ostream & ios)
    {
      util::FormatData::data(ios) = util::STAT;
      return ios;
    }

    std::ostream & dot(std::ostream & ios)
    {
      util::FormatData::data(ios) = util::DOT;
      return ios;
    }

    std::ostream & sa(std::ostream &os)
    {
      util::FormatData::data(os) = util::SA;
      return os;
    }

    util::Manipulator<std::pair<MetaInformation, std::string> >
    meta(MetaInformation i, const std::string & s)
    {
      return util::MetaManipulator(pair<MetaInformation, string>(i, s));
    }

    util::Manipulator<std::map<std::string, PetriNet *> >
    nets(std::map<std::string, PetriNet *> & nets)
    {
      return util::PetriNetManipulator(nets);
    }


    /*!
     */
    InputError::InputError(Type type, const string & filename, int line,
			   const string & token, const string & msg) :
      type(type), message(msg), token(token), line(line), filename(filename)
    {
    }


    /*!
     */
    std::ostream & operator<<(std::ostream & os, const io::InputError & e)
    {
      os << e.filename << ":" << e.line << ": error";
      if (!e.token.empty())
	switch (e.type)
	  {
	  case io::InputError::SYNTAX_ERROR:
	    os << " near '" << e.token << "'";
	    break;
	  case io::InputError::SEMANTIC_ERROR:
	    os << ": '" << e.token << "'";
	    break;
	  }
      return os << ": " << e.message;
    }


    /*!
     * Stream the PetriNet object to a given output stream, using the
     * stream format (see pnapi::io).
     */
    std::ostream & operator<<(std::ostream & os, const PetriNet & net)
    {
        switch (util::FormatData::data(os))
        {
            case util::OWFN:   net.output_owfn  (os); break;
            case util::DOT:    net.output_dot   (os); break;
            case util::GASTEX: net.output_gastex(os); break;

            case util::STAT:
                os << "|P|= "     << net.internalPlaces_.size()+net.inputPlaces_.size()+net.outputPlaces_.size() << "  ";
                os << "|P_in|= "  << net.inputPlaces_.size()    << "  ";
                os << "|P_out|= " << net.outputPlaces_.size()   << "  ";
                os << "|T|= "     << net.transitions_.size()    << "  ";
                os << "|F|= "     << net.arcs_.size();
            break;

            default: assert(false);
        }

        return os;
    }


    /*!
     * Stream the Automaton object to a given output stream using the stream
     * format (see pnapi::io).
     */
    std::ostream & operator<<(std::ostream &os, const Automaton &sa)
    {
      switch (util::FormatData::data(os))
      {
      case      util::SA:  sa.output_sa(os);   break;
      //case      util::STG: sa.output_stg(os);  break;
      default:  assert(false);
      }

      return os;
    }


    /*!
     * Second Automaton output has to be here because the ServiceAutomaton
     * class can convert Petri nets into a service automaton, and the Automaton
     * class can read automata from file (in .sa/.ig format).
     */
    std::ostream & operator<<(std::ostream &os, const ServiceAutomaton &sa)
    {
      switch (util::FormatData::data(os))
      {
      case      util::SA:   sa.output_sa(os);   break;
      //case      util::STG:  sa.output_stg(os);  break;
      default:  assert(false);
      }

      return os;
    }


    /*!
     * Reads a Petri net from a stream (in most cases backed by a file). The
     * format
     * of the stream data is not determined automatically. You have to set it
     * explicitly using a stream manipulator from pnapi::io.
     */
    std::istream & operator>>(std::istream & is, PetriNet & net)
      throw (InputError)
    {
      switch (util::FormatData::data(is))
	{
	case util::OWFN:
	  {
	    parser::owfn::Parser parser;
	    parser::owfn::Visitor visitor;
	    parser.parse(is).visit(visitor);
	    net = visitor.getPetriNet();
	    net.meta_ = util::MetaData::data(is);
	    break;
	  }

	case util::ONWD:
	  {
	    map<string, PetriNet *> nets = util::PetriNetData::data(is);
	    parser::onwd::Parser parser;
	    parser::onwd::Visitor visitor(nets);
	    parser.parse(is).visit(visitor);
	    net.createFromWiring(visitor.instances(), visitor.wiring());
	    net.meta_ = util::MetaData::data(is);
	    break;
	  }

	default:
	  assert(false);  // unsupported input format
	}

      return is;
    }


    std::istream & operator>>(std::istream & is, const util::Manipulator<
			      std::pair<MetaInformation, std::string> > & m)
    {
      util::MetaData::data(is)[m.data.first] = m.data.second;
      return is;
    }


    std::ostream & operator<<(std::ostream & os, const util::Manipulator<
			      std::pair<MetaInformation, std::string> > & m)
    {
      util::MetaData::data(os)[m.data.first] = m.data.second;
      return os;
    }



    /*************************************************************************
     ***** INTERNAL UTILITIES
     *************************************************************************/

    namespace util
    {

      Manipulator<Mode> mode(Mode m)
      {
	return Manipulator<Mode>(m);
      }


      Manipulator<Delim> delim(const string & s)
      {
	Delim d; d.delim = s;
	return Manipulator<Delim>(d);
      }


      bool compareContainerElements(const Node * n1, const Node * n2)
      {
	return n1->getName() < n2->getName();
      }


      bool compareContainerElements(Place * p1, Place * p2)
      {
	return compareContainerElements((Node *) p1, (Node *) p2);
      }


      bool compareContainerElements(const Place * p1, const Place * p2)
      {
	return compareContainerElements((Node *) p1, (Node *) p2);
      }


      bool compareContainerElements(Transition * t1, Transition * t2)
      {
	return compareContainerElements((Node *) t1, (Node *) t2);
      }


      bool compareContainerElements(Arc * f1, Arc * f2)
      {
	return compareContainerElements(&f1->getPlace(), &f2->getPlace());
      }


      bool compareContainerElements(const formula::Formula *,
				    const formula::Formula *)
      {
	return false;
      }


      set<Place *> filterMarkedPlaces(const set<Place *> & places)
      {
	set<Place *> filtered;
	for (set<Place *>::iterator it = places.begin(); it != places.end();
	     ++it)
	  if ((*it)->getTokenCount() > 0)
	    filtered.insert(*it);
	return filtered;
      }


      std::multimap<unsigned int, Place *>
      groupPlacesByCapacity(const set<Place *> & places)
      {
	std::multimap<unsigned int, Place *> grouped;
	for (set<Place *>::iterator it = places.begin(); it != places.end();
	     ++it)
	  grouped.insert(pair<unsigned int, Place *>((*it)->getCapacity(),*it));
	return grouped;
      }


      void outputGroupPrefix(std::ostream & os, const std::string & s)
      {
	os << s << ": ";
      }


      void outputGroupPrefix(std::ostream & os, unsigned int capacity)
      {
	if (capacity > 0)
	  os << "SAFE " << capacity << ": ";
      }


      ostream & operator<<(ostream & os, const pnapi::Arc & arc)
      {
	bool interface = true;

	switch (FormatData::data(os))
	  {
	  case OWFN:    /* ARCS: OWFN    */
	    os << arc.getPlace().getName();
	    if (arc.getWeight() != 1)
	      os << ":" << arc.getWeight();
	    break;

	  case DOT:     /* ARCS: DOT     */
	    if (!interface && arc.getPlace().getType() != Place::INTERNAL)
	      break;
	    os << " " << arc.getSourceNode() << " -> " << arc.getTargetNode()
	       << "\t[";
	    if (arc.getWeight() != 1)
	      os << "label=\"" << arc.getWeight() << "\"";
	    if (arc.getPlace().getType() == Place::INTERNAL)
	      os << "weight=10000.0";
	    os << "]";
	    break;

	  default: assert(false);
	  }
	return os;
      }


      ostream & operator<<(ostream & os, const pnapi::Node & node)
      {
	const Place * p = dynamic_cast<const Place *>(&node);
	if (p != NULL)
	  return os << *p;
	else
	  return os << *dynamic_cast<const Transition *>(&node);
      }


      // FIXME: remove dot prefix (when in dot namespace)
      string dot_getNodeName(const Node & n, bool withSuffix = false)
      {
	static PetriNet * net = NULL;
	static map<string, string> names;

	if (net != &n.getPetriNet())
	  {
	    net = &n.getPetriNet();
	    names.clear();

	    int i = 0;
	    set<Place *> places = net->getPlaces();
	    for (set<Place *>::iterator it = places.begin(); 
		 it != places.end(); ++it)
	      {
		stringstream ss; ss << "p" << ++i;
		names[(*it)->getName()] = ss.str();
	      }

	    i = 0;
	    set<Transition *> transitions = net->getTransitions();
	    for (set<Transition *>::iterator it = transitions.begin(); 
		 it != transitions.end(); ++it)
	      {
		stringstream ss; ss << "t" << ++i;
		names[(*it)->getName()] = ss.str();
	      }
	  }

	return names[n.getName()] + string(withSuffix ? "_l" : "");
      }


      // FIXME: remove dot prefix (when in dot namespace)
      void dot_outputNode(ostream & os, const Node & n, const string & attr)
      {
	Mode mode = ModeData::data(os);
	string dotName   = dot_getNodeName(n);
	string dotName_l = dot_getNodeName(n, true);

	os << dotName;

	if (mode == NORMAL)
	  os << "\t[" << attr << "]" << endl
	     << " " << dotName_l << "\t[style=invis]" << endl
	     << " " << dotName_l << " -> " << dotName
	     << " [headlabel=\"" << n.getName() << "\"]";

	if (mode == INNER)
	  os << " " << dotName_l;
      }


      ostream & operator<<(ostream & os, const pnapi::Place & p)
      {
	switch (FormatData::data(os))
	  {
	  case OWFN:    /* PLACES: OWFN    */
	    os << p.getName();
	    if (ModeData::data(os) == PLACE_TOKEN &&
		p.getTokenCount() != 1)
	      os << ": " << p.getTokenCount();
	    break;


	  case DOT:     /* PLACES: DOT     */
	    {
	      // place attributes (used in NORMAL mode)
	      stringstream attributes;
	      if (p.getTokenCount() == 1)
		attributes << "fillcolor=black peripheries=2 height=\".2\" "
			   << "width=\".2\" ";
	      else if (p.getTokenCount() > 1)
		attributes << "label=\"" << p.getTokenCount() << "\" "
			   << "fontcolor=black fontname=\"Helvetica\" ";

	      switch (p.getType())
		{
		case Node::INPUT:  
		  attributes << "fillcolor=orange"; 
		  break;
		case Node::OUTPUT: 
		  attributes << "fillcolor=yellow"; 
		  break;
		default: 
		  if (p.wasInterface())
		    attributes << "fillcolor=lightgoldenrod1";
		  break;
		}

	      // output the place as a node
	      dot_outputNode(os, p, attributes.str());
	      break;
	    }


	  default: assert(false);   /* PLACES: <UNKNOWN> */
	  }
	return os;
      }


      ostream & operator<<(ostream & os, const pnapi::Transition & t)
      {
	switch (FormatData::data(os))
	  {
	  case OWFN:    /* TRANSITIONS: OWFN    */
	    os << "TRANSITION " << t.getName() << endl;
	    /*
	    switch (t.getType())
	      {
	      case Node::INTERNAL: os                        << endl; break;
	      case Node::INPUT:    os << " { input }"        << endl; break;
	      case Node::OUTPUT:   os << " { output }"       << endl; break;
	      case Node::INOUT:    os << " { input/output }" << endl; break;
	      }
	    */
	    os << delim(", ")
	       << "  CONSUME " << t.getPresetArcs()  << ";" << endl
	       << "  PRODUCE " << t.getPostsetArcs() << ";" << endl;
	    break;


	  case DOT:     /* TRANSITIONS: DOT     */
	    {
	      // transition attributes (used in NORMAL mode)
	      stringstream attributes;
	      switch(t.getType())
		{
		case(Node::INPUT):  attributes << "fillcolor=orange"; break;
		case(Node::OUTPUT): attributes << "fillcolor=yellow"; break;
		case(Node::INOUT):  attributes
		    << "fillcolor=gold label=< <TABLE BORDER=\"1\""
		    << " CELLBORDER=\"0\" CELLSPACING=\"0\" CELLPADDING=\"0\""
		    << " HEIGHT=\"21\" WIDTH=\"21\" FIXEDSIZE=\"TRUE\"><TR>"
		    << "<TD HEIGHT=\"11\" WIDTH=\"21\" FIXEDSIZE=\"TRUE\""
		    << " BGCOLOR=\"ORANGE\">"
		    << "</TD></TR><TR>"
		    << "<TD HEIGHT=\"10\" WIDTH=\"21\" FIXEDSIZE=\"TRUE\""
		    << " BGCOLOR=\"YELLOW\">"
		    << "</TD></TR></TABLE> >";
		  break;
		default: break;
		}
	      
	      // output the transition as a node
	      dot_outputNode(os, t, attributes.str());
	      break;
	    }


	  default: assert(false);   /* TRANSITIONS: <UNKNOWN> */
	  }
	return os;
      }


      ostream & operator<<(ostream & os, const pnapi::Condition & c)
      {
	return os << c.formula();
      }


      ostream & operator<<(ostream & os, const formula::Formula & f)
      {
	return f.output(os);
      }


      ostream & operator<<(ostream & os, const formula::Negation & f)
      {
	return os << "NOT (" << **f.children().begin() << ")";
      }


      ostream & operator<<(ostream & os, const formula::Conjunction & f)
      {
	string wildcard;
	set<const Formula *> children = f.children();

	/* WILDCARD COLLAPSING */
	set<const Place *> formulaPlaces = f.places();
	if (!formulaPlaces.empty())
	  {
	    set<Place *> netPlaces =
	      (*formulaPlaces.begin())->getPetriNet().getPlaces();
	    if (formulaPlaces.size() == netPlaces.size())
	      {
		set<const Formula *> filteredChildren;
		for (set<const Formula *>::iterator it = children.begin();
		     it != children.end(); ++it)
		  {
		    const formula::FormulaEqual * f =
		      dynamic_cast<const formula::FormulaEqual *>(*it);
		    if (f == NULL || f->tokens() != 0)
		      filteredChildren.insert(*it);
		    else
		      wildcard = " AND ALL_OTHER_PLACES_EMPTY";
		  }
		if (filteredChildren.empty() && !wildcard.empty())
		  wildcard = "ALL_PLACES_EMPTY";
		children = filteredChildren;
	      }
	  }

	if (f.children().empty())
	  return os << formula::FormulaTrue();
	else
	  return os << "(" << delim(" AND ") << children << wildcard << ")";
      }


      ostream & operator<<(ostream & os, const formula::Disjunction & f)
      {
	if (f.children().empty())
	  return os << formula::FormulaFalse();
	else
	  return os << "(" << delim(" OR ") << f.children() << ")";
      }


      ostream & operator<<(ostream & os, const formula::FormulaTrue &)
      {
	return os << "TRUE";
      }


      ostream & operator<<(ostream & os, const formula::FormulaFalse &)
      {
	return os << "FALSE";
      }


      ostream & operator<<(ostream & os, const formula::FormulaEqual & f)
      {
	return os << f.place().getName() << " = " << f.tokens();
      }


      ostream & operator<<(ostream & os, const formula::FormulaNotEqual & f)
      {
	return os << f.place().getName() << " != " << f.tokens();
      }


      ostream & operator<<(ostream & os, const formula::FormulaGreater & f)
      {
	return os << f.place().getName() << " > " << f.tokens();
      }


      ostream & operator<<(ostream & os, const formula::FormulaGreaterEqual & f)
      {
	return os << f.place().getName() << " >= " << f.tokens();
      }


      ostream & operator<<(ostream & os, const formula::FormulaLess & f)
      {
	return os << f.place().getName() << " < " << f.tokens();
      }


      ostream & operator<<(ostream & os, const formula::FormulaLessEqual & f)
      {
	return os << f.place().getName() << " <= " << f.tokens();
      }


      ostream & operator<<(ostream &os, const State &s)
      {
        os << s.getName();

        return os;
      }

    }

  }

}
