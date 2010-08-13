/*****************************************************************************\
 * Copyright (c) 2010. All rights reserved. Dirk Fahland. AGPL3.0
 * 
 * ServiceTechnology.org - Petri Net API/Java
 * 
 * This program and the accompanying materials are made available under
 * the terms of the GNU Affero General Public License Version 3 or later,
 * which accompanies this distribution, and is available at 
 * http://www.gnu.org/licenses/agpl.txt
 * 
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * 
\*****************************************************************************/

package hub.top.petrinet;

import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

/**
 * A Petri net. Base class of the Petri net API. Provides all basic functions for
 * manipulating the net (create/delete nodes and arcs, ...)
 * 
 * @author Dirk Fahland
 */
public class PetriNet {
  
  protected int nodeId;

  // all places of the net
  private HashSet<Place> places;
  // all transitions of the net
  private HashSet<Transition> transitions;
  
  // all roles of the net
  private HashSet<String> roles;
  
  /**
   * Create an empty Petri net
   */
  public PetriNet () {
    this.places = new HashSet<Place>();
    this.transitions = new HashSet<Transition>();
    this.roles = new HashSet<String>();
    
    nodeId = 0;
  }
  
  /**
   * Create a copy of the given net.
   * @param net
   */
  public PetriNet (PetriNet net) {
    
    this();   // initialize basic fields
    
    HashMap<Place, Place> createMap = new HashMap<Place, Place>();
    
    // and copy the existing Petri net
    for (Transition t : net.getTransitions()) {

      // consisting of all transitions
      Transition t_ = this.addTransition(t.getName());
      
      // together with their pre-sets
      for (Place p : t.getPreSet()) {
        // see if place already exists in the net
        Place p_ = createMap.get(p);
        if (p_ == null) {
          // if not: create it
          p_ = this.addPlace(p.getName());
          p_.setTokens(p.getTokens());
          createMap.put(p, p_);
        }
        this.addArc(p_, t_);
      }
    
      // and post-sets
      for (Place p : t.getPostSet()) {
        // see if place already exists in the net
        Place p_ = createMap.get(p);
        if (p_ == null) {
          // if not: create it
          p_ = this.addPlace(p.getName());
          p_.setTokens(p.getTokens());
          createMap.put(p, p_);
        }
        this.addArc(t_, p_);
      }
    }
  }
  
  /**
   * @return next unique id for a new node
   */
  protected int getNextNodeID () {
    return ++nodeId;
  }
  
  /**
   * Add place with the given name to the net. The net must not contain a place
   * of the same name.
   * 
   * @param name
   * @return the created place
   */
  public Place addPlace_unique(String name) {
    if (findPlace(name) != null) {
      assert false : "Tried to add existing place";
      return null;
    }
    return addPlace(name);
  }

  /**
   * Add place with the given name to the net.
   * 
   * @param name
   * @return the created place
   */
  public Place addPlace(String name) {
    Place p = new Place(this, name);
    places.add(p);
    return p;
  }

  /**
   * Add transition with the given name to the net. The net must not contain a transition
   * of the same name.
   * 
   * @param name
   * @return the created transition
   */
  public Transition addTransition_unique(String name) {
    if (findTransition(name) != null) {
      
      assert false : "Tried to add existing transition";
      return null;
    }
    return addTransition(name);
  }
  
  /**
   * Add transition with the given name to the net.
   * 
   * @param name
   * @return the created transition
   */
  public Transition addTransition(String name) {
    Transition t = new Transition(this, name);
    transitions.add(t);
    return t;
  }
  
  /**
   * Create an arc from source to target in the net, if the arc is not
   * present (otherwise return existing arc)
   * 
   * @param src
   * @param tgt
   * @return arc from src to tgt
   */
  private Arc _addArc(Node src, Node tgt) {
    for (Arc a : src.getOutgoing()) {
      if (a.getTarget() == tgt) return a;
    }
    
    Arc a = new Arc(src,tgt);
    src.attachArc(a);
    tgt.attachArc(a);
    return a;
  }
  
  /**
   * Create a new arc from {@link Transition} t to {@link Place} p. 
   * @param t
   * @param p
   * @return created arc
   */
  public Arc addArc(Transition t, Place p) {
    if (!transitions.contains(t) || !places.contains(p)) {
      assert false : "Tried to draw arc between non-existing nodes";
      return null;
    }
    return _addArc(t,p);
  }
  
  /**
   * Create a new arc from {@link Place} p to {@link Transition} t.
   * @param p
   * @param t
   * @return created arc
   */
  public Arc addArc(Place p, Transition t) {
    if (!places.contains(p) || !transitions.contains(t)) {
      assert false : "Tried to draw arc between non-existing nodes";
      return null;
    }
    return _addArc(p,t);
  }
  
  /**
   * Create arc from source node to target node. The net must contain nodes
   * with the given names.
   * @param srcName
   * @param tgtName
   * @return created arc
   */
  public Arc addArc(String srcName, String tgtName) {
    // try place -> transitions
    Node src = findPlace(srcName);
    Node tgt = findTransition(tgtName);
    if (src != null && tgt != null) return _addArc(src, tgt);
    
    // else: src == null || tgt == null
    // try transitions -> place
    src = findTransition(srcName);
    tgt = findPlace(tgtName);
    if (src != null && tgt != null) return _addArc(src, tgt);

    assert false : "Tried to draw arc between non-existing nodes";
    return null; 
  }
  
  /**
   * @param name
   * @return the place with the given name if it exists in the net; assumes
   * that that is at most one place with the given name in the net
   */
  public Place findPlace(String name) {
    for (Place p : places) {
      if (p.getName().equals(name))
        return p;
    }
    return null;
  }

  /**
   * @param name
   * @return the transition with the given name if it exists; assumes
   * that that is at most one transition with the given name in the net
   */
  public Transition findTransition(String name) {
    for (Transition t : transitions) {
      if (t.getName().equals(name))
        return t;
    }
    return null;
  }
  
  /**
   * replace all names of places and transitions with identifiers pNUM and tNUM, respectively
   */
  public void anonymizeNet() {
    int num = 0;
    for (Transition t : transitions) {
      this.setName(t, "t"+num);
      num++;
    }
    num = 0;
    for (Place p : places) {
      this.setName(p, "p"+num);
      num++;
    }

  }
  
  /**
   * @return arcs of the net, iterates over all nodes of the net
   * and returns the arcs
   */
  public HashSet<Arc> getArcs() {
    HashSet<Arc> arcs = new HashSet<Arc>();
    for (Transition t : transitions) {
      arcs.addAll(t.getIncoming());
      arcs.addAll(t.getOutgoing());
    }
    return arcs;
  }
  
  /**
   * @return transitions of the net
   */
  public HashSet<Transition> getTransitions() {
    return transitions;
  }
  
  /**
   * @return places of the net
   */
  public HashSet<Place> getPlaces() {
    return places;
  }
  
  /**
   * @return roles of the net
   */
  public HashSet<String> getRoles() {
    return roles;
  }
  
  /**
   * Mark the place with the given <code>placeName</code> with <code>num</code> tokens.
   * @param placeName
   * @param num
   */
  public void setTokens(String placeName, int num) {
    Place p = findPlace(placeName);
    if (p != null) p.setTokens(num);
  }
  
  /**
   * Mark the given place <code>p</code> with <code>num</code> tokens.
   * @param p
   * @param num
   */
  public void setTokens(Place p, int num) {
    p.setTokens(num);
  }
  
  /**
   * Change name of node n to newName. Checks if a node having newName already exists.
   * If yes, then an exception is thrown.
   * 
   * @param n
   * @param newName
   */
  public void setName(Node n, String newName) {
    if (n.getName().equals(newName)) return;
    
    if (findPlace(newName) != null) {
      assert false : "Place with name "+newName+" already exists.";
      return;
    }
    if (findTransition(newName) != null) {
      assert false : "Transition with name "+newName+" already exists.";
      return;
    }
    n.setName(newName);
  }

  /**
   * Add role to the roles of the net. 
   * @param role
   */
  public void addRole(String role) {
    this.roles.add(role);
  }
  
  /**
   * @param role
   * @return if this net knows the given role
   */
  public boolean hasRole(String role) {
    return this.roles.contains(role);
  }

  /**
   * Let each place of the net acquire the roles of its
   * pre- and post-transitions.
   */
  public void spreadRolesToPlaces_union() {
    
    for (Place p : places) {
      for (Transition t : p.getPreSet())
        for (String role : t.getRoles()) p.addRole(role);
      for (Transition t : p.getPostSet())
        for (String role : t.getRoles()) p.addRole(role);
    }
  }
  
  /**
   * Give each transition that has no role the explicit role "unassigned"
   */
  public void setRoles_unassigned() {
    for (Transition t : transitions) {
      if (t.getRoles().size() == 0)
        t.addRole("unassigned");
    }
  }
  
  /* -----------------------------------------------------------------------
   * 
   *   Net manipulations
   * 
   * ----------------------------------------------------------------------- */
  

  /**
   * Merge the transitions t1 and t2 in the net. Adds pre- and post-places of
   * t2 to the pre- or post-set of t1 and removes t2 from the net.
   * 
   * @param t1
   * @param t2
   */
  public void mergeTransitions(Transition t1, Transition t2) {
    for (Place p : t2.getPreSet()) {
      if (!t1.getPreSet().contains(p)) {
        addArc(p, t1); 
      }
    }
    for (Place p : t2.getPostSet()) {
      if (!t1.getPostSet().contains(p)) {
        addArc(t1,p);
      }
    }
    removeTransition(t2);
  }
  
  /**
   * Merge the places p1 and p2 in the net. Creates a new place pMerge; the
   * pre-and post-transitions of pMerge are the pre- and post-transitions of
   * p1 and p2, respectively.
   * 
   * The original places p1 and p2 are not removed from the net. Use
   * {@link #removePlace(PtNet, Place)} to remove either once this is appropriate.
   * 
   * @param p1
   * @param p2
   * @return the new place pMerge
   */
  public Place mergePlaces(Place p1, Place p2) {
    Place pMerged = addPlace(p1.getName()+"_"+p2.getName());
    setTokens(pMerged, p1.getTokens() + p2.getTokens());

    for (Transition t : p1.getPreSet()) {
      addArc(t, pMerged);
    }
    for (Transition t : p1.getPreSet()) {
      if (pMerged.getPreSet().contains(t)) continue;
      addArc(t, pMerged);
    }
    for (Transition t : p1.getPostSet()) {
      addArc(pMerged, t);
    }
    for (Transition t : p2.getPostSet()) {
      if (pMerged.getPostSet().contains(t)) continue;
      addArc(pMerged, t);
    }
    return pMerged;
  }
  
  /**
   * Remove transition t from the net. Remove also all adjacent arcs.
   * 
   * @param t
   */
  public void removeTransition(Transition t) {
    LinkedList<Arc> a_remove = new LinkedList<Arc>();
    a_remove.addAll(t.getIncoming());
    a_remove.addAll(t.getOutgoing());
    for (Arc a : a_remove) {
      removeArc(a);
    }
    transitions.remove(t);
  }
  
  /**
   * Remove place p from the net. Remove also all adjacent arcs and update
   * the markings where p is involved.
   * 
   * @param p
   */
  public void removePlace(Place p) {
    
    if (!places.contains(p)) {
      System.err.println("Error. Tried to remove place "+p.getName()+" from net, but this place is not contained in the net");
      return;
    }
    
    setTokens(p, 0);
    
    LinkedList<Arc> a_remove = new LinkedList<Arc>();
    a_remove.addAll(p.getIncoming());
    a_remove.addAll(p.getOutgoing());
    for (Arc a : a_remove) {
      removeArc(a);
    }
    places.remove(p);
  }
  
  /**
   * Remove arc a from the net. 
   * @param a
   */
  public void removeArc(Arc a) {
    Node src = a.getSource();
    Node tgt = a.getTarget();
    src.detachArc(a);
    tgt.detachArc(a);
  }

  /**
   * Check whether transitions t1 and t2 are parallel (except for the places in
   * the set ignore). Two transitions are parallel if their pre- and post-sets
   * are identical.
   * 
   * @param t1
   * @param t2
   * @param ignore
   * @return
   *    true iff transitions t1 and t2 are parallel except for the places in
   *    the set ignore
   */
  public boolean parallelTransitions(Transition t1, Transition t2, Collection<Place> ignore) {
    for (Place p : t1.getPreSet()) {
      if (ignore != null && ignore.contains(p)) continue;
      if (!t2.getPreSet().contains(p)) return false;
    }
    for (Place p : t1.getPostSet()) {
      if (ignore != null && ignore.contains(p)) continue;
      if (!t2.getPostSet().contains(p)) return false;
    }
    for (Place p : t2.getPreSet()) {
      if (ignore != null && ignore.contains(p)) continue;
      if (!t1.getPreSet().contains(p)) return false;
    }
    for (Place p : t2.getPostSet()) {
      if (ignore != null && ignore.contains(p)) continue;
      if (!t1.getPostSet().contains(p)) return false;
    }
    return true;
  }
  
  /**
   * @param t1
   * @param t2
   * @return
   *    true iff transitions t1 and t2 have the same label and the same
   *             pre-set but different post-sets.
   */
  public boolean nondeterministicTransitions(Transition t1, Transition t2) {
    if (t1 == t2) return false;
    if (!t1.getName().equals(t2.getName())) return false;
    if (t1.getPreSet().size() != t2.getPreSet().size()) return true;
    
    for (Place p : t1.getPreSet()) {
      if (!t2.getPreSet().contains(p)) return false;
    }
    // each pre-place of t1 is a pre-place of t2 (and vice versa because both have
    // pre-sets of equal size)

    if (t1.getPostSet().size() != t2.getPostSet().size()) return true;
    
    for (Place p : t1.getPostSet()) {
      if (!t2.getPostSet().contains(p)) return true;
    }
    // eace post-place of t1 is also a post-place of t2
    return false;
  }
  
  /**
   * Check whether transitions t1 and t2 are parallel. Two transitions are
   * parallel if their pre- and post-sets are identical.
   * 
   * @param t1
   * @param t2
   * @return
   *    true iff transitions t1 and t2 are parallel
   */
  public boolean parallelTransitions(Transition t1, Transition t2) {
    return parallelTransitions(t1, t2, null);
  }
  
  public void removeParallelTransitions() {
    Transition[] trans = getTransitions().toArray(new Transition[getTransitions().size()]);
    for (int i=0; i<trans.length-1; i++) {
      if (trans[i] == null) continue;
      
      for (int j=i+1; j<trans.length; j++) {
        if (trans[j] == null) continue;
        
        String prefix_i = trans[i].getName();
        String prefix_j = trans[j].getName();
        
        if (prefix_i.equals(prefix_j)) {
  
          if (parallelTransitions(trans[i], trans[j], null)) {
            if (trans[i].id < trans[j].id) {
              removeTransition(trans[j]);
              trans[j] = null;
            } else {
              removeTransition(trans[i]);
              trans[i] = null;
              break;
            }
          }
        }
      }
    }
  }


  /**
   * structurally reduce the Petri net to remove all tau transitions
   * 
   * TODO: re-implement all branching bisimulation preserving structural reduction rules for branching processes
   */
  public void removeTauTransitions() {
    LinkedList<Transition> taus = new LinkedList<Transition>();
    for (Transition t : transitions) {
      if (t.isTau()) {
        taus.add(t);
      }
    }
    
    boolean netChanged = true;
    while (netChanged) {
      netChanged = false;
      
      for (Transition t : taus) {
        if (t.getPreSet().size() == 1 && t.getPostSet().size() == 1) {
          Place pIn = t.getPreSet().get(0);
          Place pOut = t.getPostSet().get(0);
          mergePlaces(pIn, pOut);
          removeTransition(t);
          taus.remove(t);
          netChanged = true;
          break;
        }
      }
    }
  }

  /**
   * hide any place of the net between two tau transitions
   * (i.e. set it its visibility to tau) 
   */
  public void makePlacesInvisible() {
    for (Place p : places) {
      if (p.getPreSet().size() != 1 || p.getPostSet().size() != 1) continue;
      Transition t1 = p.getPreSet().get(0);
      Transition t2 = p.getPostSet().get(0);
      if (t1.isTau() && t2.isTau())
        p.setTau(true);
    }
  }

  /**
   * Remove all isolated nodes (empty pre-set and empty post-set) from the net.
   */
  public void removeIsolatedNodes() {
    LinkedList<Transition> t_remove = new LinkedList<Transition>();
    for (Transition t : transitions) {
      if (t.getPreSet().size() == 0 && t.getPostSet().size() == 0) {
        t_remove.add(t);
      }
    }
    LinkedList<Place> p_remove = new LinkedList<Place>();
    for (Place p : places) {
      if (p.getPreSet().size() == 0 && p.getPostSet().size() == 0
          && p.getTokens() == 0) {
        p_remove.add(p);
      }
    }
    for (Transition t : t_remove) removeTransition(t);
    for (Place p : p_remove) removePlace(p);
  }

  
  /* -----------------------------------------------------------------------
   * 
   *   Output
   * 
   * ----------------------------------------------------------------------- */
  
  /**
   * Write the Petri net in GraphViz dot format
   * 
   * @param net
   * @return
   */
  public String toDot() {
    StringBuilder b = new StringBuilder();
    b.append("digraph BP {\n");
    
    // standard style for nodes and edges
    b.append("graph [fontname=\"Helvetica\" nodesep=0.3 ranksep=\"0.2 equally\" fontsize=10];\n");
    b.append("node [fontname=\"Helvetica\" fontsize=8 fixedsize width=\".3\" height=\".3\" label=\"\" style=filled fillcolor=white];\n");
    b.append("edge [fontname=\"Helvetica\" fontsize=8 color=white arrowhead=none weight=\"20.0\"];\n");

    String tokenFillString = "fillcolor=black peripheries=2 height=\".2\" width=\".2\" ";
    
    HashMap<Node, Integer> nodeIDs = new HashMap<Node, Integer>();
    int nodeID = 0;
    
    // first print all places
    b.append("\n\n");
    b.append("node [shape=circle];\n");
    for (Place p : places) {
      nodeID++;
      nodeIDs.put(p, nodeID);
      
      if (p.getTokens() > 0)
        b.append("  p"+nodeID+" ["+tokenFillString+"]\n");
      else
        b.append("  p"+nodeID+" []\n");
      
      String auxLabel = ""; //"ROLES: "+toString(p.getRoles());
        
      b.append("  p"+nodeID+"_l [shape=none];\n");
      b.append("  p"+nodeID+"_l -> p"+nodeID+" [headlabel=\""+p.getName()+" "+auxLabel+"\"]\n");
    }

    // then print all events
    b.append("\n\n");
    b.append("node [shape=box];\n");
    for (Transition t : transitions) {
      nodeID++;
      nodeIDs.put(t, nodeID);

      b.append("  t"+nodeID+" []\n");
      
      String auxLabel  = "";  //"ROLES: "+toString(t.getRoles());
      
      b.append("  t"+nodeID+"_l [shape=none];\n");
      b.append("  t"+nodeID+"_l -> t"+nodeID+" [headlabel=\""+t.getName()+" "+auxLabel+"\"]\n");
    }
    
    /*
    b.append("\n\n");
    b.append(" subgraph cluster1\n");
    b.append(" {\n  ");
    for (CNode n : nodes) {
      if (n.isEvent) b.append("e"+n.localId+" e"+n.localId+"_l ");
      else b.append("c"+n.localId+" c"+n.localId+"_l ");
    }
    b.append("\n  label=\"\"\n");
    b.append(" }\n");
    */
    
    // finally, print all edges
    b.append("\n\n");
    b.append(" edge [fontname=\"Helvetica\" fontsize=8 arrowhead=normal color=black];\n");
    for (Arc arc : getArcs()) {
      if (arc.getSource() instanceof Transition)
        b.append("  t"+nodeIDs.get(arc.getSource())+" -> p"+nodeIDs.get(arc.getTarget())+" [weight=10000.0]\n");
      else
        b.append("  p"+nodeIDs.get(arc.getSource())+" -> t"+nodeIDs.get(arc.getTarget())+" [weight=10000.0]\n");
    }
    b.append("}");
    return b.toString();
  }
  
  /**
   * Write the Petri net in GraphViz dot format
   * 
   * @param net
   * @return
   */
  public String toDot_swimlanes() {
    StringBuilder b = new StringBuilder();
    b.append("digraph BP {\n");
    
    // standard style for nodes and edges
    b.append("graph [fontname=\"Helvetica\" nodesep=0.3 ranksep=\"0.2 equally\" fontsize=10];\n");
    b.append("node [fontname=\"Helvetica\" fontsize=8 fixedsize width=\".3\" height=\".3\" label=\"\" style=filled fillcolor=white];\n");
    b.append("edge [fontname=\"Helvetica\" fontsize=8 color=white arrowhead=none weight=\"20.0\"];\n");

    String tokenFillString = "fillcolor=black peripheries=2 height=\".2\" width=\".2\" ";
    String preconditionFillString = "fillcolor=gray66";
    
    HashMap<Node, Integer> nodeIDs = new HashMap<Node, Integer>();
    int nodeID = 0;
    
    HashSet< HashSet<String> > swimlanes = new HashSet< HashSet<String> >();
    for (Transition t : getTransitions()) {
      swimlanes.add(t.getRoles());
    }
    HashSet< Place > printedPlaces = new HashSet<Place>();
    
    int laneNum=0;
    
    // for each swimlane
    for (HashSet<String> lane : swimlanes) {
      laneNum++;
      
      b.append("subgraph cluster_"+laneNum+" {");

      // first print all places
      b.append("\n\n");
      b.append("node [shape=circle];\n");
      for (Place p : places) {
        
        // only places that are exactly in the given lane
        if (!lane.equals(p.getRoles())) continue;
        printedPlaces.add(p);
        
        nodeID++;
        nodeIDs.put(p, nodeID);
        
        if (p.getTokens() > 0)
          b.append("  p"+nodeID+" ["+tokenFillString+"]\n");
        else if (p.getIncoming().isEmpty())
          b.append("  p"+nodeID+" ["+preconditionFillString+"]\n");
        else
          b.append("  p"+nodeID+" []\n");
        
        String auxLabel = "";//"ROLES: "+toString(p.getRoles());
          
        b.append("  p"+nodeID+"_l [shape=none];\n");
        b.append("  p"+nodeID+"_l -> p"+nodeID+" [headlabel=\""+p.getName()+" "+auxLabel+"\"]\n");
      }
  
      // then print all events
      b.append("\n\n");
      b.append("node [shape=box];\n");
      for (Transition t : transitions) {
        
        if (!lane.equals(t.getRoles())) continue;
        
        nodeID++;
        nodeIDs.put(t, nodeID);
  
        b.append("  t"+nodeID+" []\n");
        
        String auxLabel  = "";//"ROLES: "+toString(t.getRoles());
        
        b.append("  t"+nodeID+"_l [shape=none];\n");
        b.append("  t"+nodeID+"_l -> t"+nodeID+" [headlabel=\""+t.getName()+" "+auxLabel+"\"]\n");
      }
      
      b.append("}\n\n");  //subgraph
    }
    
    // print all remaining places
    b.append("\n\n");
    b.append("node [shape=circle];\n");
    for (Place p : places) {
      
      // only places that have not been printed yet
      if (printedPlaces.contains(p)) continue;
      
      nodeID++;
      nodeIDs.put(p, nodeID);
      
      if (p.getTokens() > 0)
        b.append("  p"+nodeID+" ["+tokenFillString+"]\n");
      else if (p.getIncoming().isEmpty())
        b.append("  p"+nodeID+" ["+preconditionFillString+"]\n");
      else
        b.append("  p"+nodeID+" []\n");
      
      String auxLabel = "ROLES: "+toString(p.getRoles());
        
      b.append("  p"+nodeID+"_l [shape=none];\n");
      b.append("  p"+nodeID+"_l -> p"+nodeID+" [headlabel=\""+toLoLA_ident(p.getName())+" "+auxLabel+"\"]\n");
    }
    
    // finally, print all edges
    b.append("\n\n");
    b.append(" edge [fontname=\"Helvetica\" fontsize=8 arrowhead=normal color=black];\n");
    for (Arc arc : getArcs()) {
      if (arc.getSource() instanceof Transition)
        b.append("  t"+nodeIDs.get(arc.getSource())+" -> p"+nodeIDs.get(arc.getTarget())+" [weight=10000.0]\n");
      else
        b.append("  p"+nodeIDs.get(arc.getSource())+" -> t"+nodeIDs.get(arc.getTarget())+" [weight=10000.0]\n");
    }
    b.append("}");
      
    return b.toString();
  }
  
  /**
   * @return brief summary about this net
   */
  public String getInfo() {
    return "|P|= "+places.size()+"  |P_in|= "+0+"  |P_out|= "+0+"  |T|= "+transitions.size()+"  |F|= "+getArcs().size();
  }
  
  /**
   * Print a collection of strings into a string
   * @param strings
   * @return
   */
  public static String toString(Collection<String> strings) {
    String result = "[";
    Iterator<String> it = strings.iterator();
    while (it.hasNext()) {
      result += it.next();
      if (it.hasNext()) result += ", ";
    }
    return result+"]";
  }
  
  /**
   * Convert arbitrary identifier into LoLA-compatible format.
   * @param s
   * @return
   */
  public static String toLoLA_ident(String s) {
    if (s.lastIndexOf('_') >= 0)
      return s.substring(0,s.lastIndexOf('_'));
    else
      return s;
  }
}
