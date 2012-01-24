/*****************************************************************************\
 * Copyright (c) 2008-2012 Dirk Fahland. EPL1.0/AGPL3.0
 * All rights reserved.
 * 
 * ServiceTechnolog.org - Greta
 *                       (Graphical Runtime Environment for Adaptive Processes) 
 * 
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License v1.0, which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 * 
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 * 
 * The Original Code is this file as it was released on December 04, 2008.
 * The Initial Developer of the Original Code are
 *    Dirk Fahland
 * 
 * Portions created by the Initial Developer are Copyright (c) 2008, 2009
 * the Initial Developer. All Rights Reserved.
 * 
 * Contributor(s):
 * 
 * Alternatively, the contents of this file may be used under the terms of
 * the GNU Affero General Public License Version 3 or later (the "GPL") in
 * which case the provisions of the AGPL are applicable instead of those above.
 * If you wish to allow use of your version of this file only under the terms
 * of the AGPL and not to allow others to use your version of this file under
 * the terms of the EPL, indicate your decision by deleting the provisions
 * above and replace them with the notice and other provisions required by the 
 * AGPL. If you do not delete the provisions above, a recipient may use your
 * version of this file under the terms of any one of the EPL or the AGPL.
\*****************************************************************************/

package hub.top.greta.cpn;

import java.util.Map;

import hub.top.uma.DNode;
import hub.top.uma.DNodeSys_AdaptiveSystem;

public class ColoredConditionVisitor {
  
  private DNodeSys_AdaptiveSystem system;
  
  public ColoredConditionVisitor(DNodeSys_AdaptiveSystem system) {
    this.system = system;
  }
  
  public boolean satisfiesColoring(Map<DNode, DNode> embedding) {
    
    // TODO: build evaluation string
    /* let
     *   val x1 = v1;
     *   val x2 = v2;
     * in
     *   constraints over x1, x2, ...
     * end; 
     */
    // TODO: evaluate
    for (Map.Entry<DNode, DNode> m : embedding.entrySet()) {
      System.out.println(m.getKey()+" --> "+system.getOriginalNode(m.getValue()));
    }
    
    return true;
  }

}