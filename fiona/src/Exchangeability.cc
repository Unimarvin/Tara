/*****************************************************************************
 * Copyright 2006 Kathrin Kaschner                                           *
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
 * \file    Exchangeability.cc
 *
 * \brief   functions for comparing two BDD-represented OGs for equality
 *
 * \author  responsible: Kathrin Kaschner <kathrin.kaschner@informatik.uni-rostock.de>
 *
 * \note    This file is part of the tool Fiona and was created during the
 *          project "Tools4BPEL" at the Humboldt-Universit�t zu Berlin. See
 *          http://www.informatik.hu-berlin.de/top/tools4bpel for details.
 *
 */

#include "mynew.h"
#include "options.h" 
#include "debug.h"
#include <iostream>
#include <string.h>
#include "Exchangeability.h"

DdManager* Exchangeability::mgrMp = NULL;
DdManager* Exchangeability::mgrAnn = NULL;
int Exchangeability::nbrBdd = 0;
 
Exchangeability::Exchangeability(char* filename, Cudd_ReorderingType heuristic) {
	trace(TRACE_5, "Exchangeability::Exchangeability(char* filename): begin\n");		   
    // Init cudd package when first Exchangeability object is created.
    if (nbrBdd == 0) {
        //mgrMp = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);
        //mgrAnn = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);
        mgrMp  = Cudd_Init(0, 0, 1, 1, 1);
        mgrAnn = Cudd_Init(0, 0, 1, 1, 1);
        
        //enable automatic dynamic reordering of the BDDs
		Cudd_AutodynEnable(mgrMp, heuristic);
		Cudd_AutodynEnable(mgrAnn, heuristic);		
    }
    
    names = NULL;
    nbrVarAnn = 0;
	loadBdd(filename);
	
	return;
	
	++nbrBdd;
	Cudd_ReorderingType method = (Cudd_ReorderingType)4;
	if (Cudd_ReorderingStatus(mgrMp, &method) == 1) {
		cout << "automatic reordering is enabled";
	} 
	 
	for (int i = 0; i < nbrVarAnn; ++i) {
        if (names[i][0] == '!' || names[i][0] == '?') {
           (labelList).push_back(names[i]);
        }
    }
    
/* list<char*>::iterator list_iter;
	cout << "Liste ausgeben ";
   	for( list_iter = labelList.begin(); list_iter != labelList.end(); ++list_iter) {
     	cout << *list_iter;
   	}
	cout << endl;           
*/
	trace(TRACE_5, "Exchangeability::Exchangeability(char* filename): end\n");
}

Exchangeability::~Exchangeability(){
	trace(TRACE_5, "Exchangeability::~Exchangeability(): begin\n");		   
    
    delete [] names;
    
    Cudd_RecursiveDeref(mgrAnn, bddAnn);
    Cudd_RecursiveDeref(mgrMp, bddMp);

    //checkManager(mgrAnn, "mgrAnn");
    //checkManager(mgrMp, "mgrMp");

    --nbrBdd;
    assert(nbrBdd >= 0);
    
    // Clean up cudd package when last Exchangeability object is destroyed.
    if (nbrBdd == 0) {
        Cudd_Quit(mgrAnn);
        mgrAnn = NULL;
        Cudd_Quit(mgrMp);
        mgrMp  = NULL;
    }
    
    labelList.clear();
    
	trace(TRACE_5, "Exchangeability::~Exchangeability(): end\n");
}

void Exchangeability::loadBdd(char* filename){
    trace(TRACE_5, "Exchangeability::loadBdds(char* filename): begin\n");		   
    cout << "loading BDD-representation of the operating guideline of " << filename << endl;
    
	//open cudd-files
	char bufferMp[256]; 
	char bufferAnn[256];

	sprintf(bufferMp, "%s.a.OG.BDD_MP.cudd", filename);
    sprintf(bufferAnn, "%s.a.OG.BDD_ANN.cudd", filename); 
    
    FILE* fpMp;
    fpMp = fopen(bufferMp, "r");
    
    FILE* fpAnn;
    fpAnn = fopen(bufferAnn, "r");
    
    if (fpMp == NULL || fpAnn == NULL){
    	sprintf(bufferMp, "%s.OG.BDD_MP.cudd", filename);
    	sprintf(bufferAnn, "%s.OG.BDD_ANN.cudd", filename); 
    	
    	fpMp = fopen(bufferMp, "r");
    	fpAnn = fopen(bufferAnn, "r");
    	
    	if (fpMp == NULL || fpAnn == NULL){
    		cerr << "cannot open cudd-files of " << filename << "\n";
            exit(4);
    	}
	}
	
	//load header of bddAnn
    int* permids = NULL;    //optimal variable ordering      
    loadHeader(fpAnn, &names, &nbrVarAnn, &permids);
    fseek(fpAnn, 0, SEEK_SET); //File-Pointer auf Anfang der Datei setzen, damit sp�ter BDD geladen werden kann;      
    
    //load bddAnn
    if (nbrBdd == 0) {
    	loadOptimalOrder(mgrAnn, nbrVarAnn, permids);
    }
    bddAnn = loadDiagram(fpAnn, mgrAnn);
    fclose(fpAnn);
    
    int nbrVarMp = 0;     //number of variables in bddMp
    
    delete [] permids;
    permids = NULL;
    
    //load header of bddMp
    loadHeader(fpMp, NULL, &nbrVarMp, &permids);
    fseek(fpMp, 0, SEEK_SET); //File-Pointer auf Anfang der Datei setzen, damit Bdd geladen werden kann;

    //load bddMp
    if (nbrBdd == 0) { 
    	loadOptimalOrder(mgrMp, nbrVarMp, permids);
    }
    bddMp = loadDiagram(fpMp, mgrMp);
    fclose(fpMp);
    
    trace(TRACE_5, "Exchangeability::loadBdds(char* filename): end\n");		   
}


void Exchangeability::loadHeader(FILE* fp, char*** names, int* nVars, int** permids){
	trace(TRACE_5,"Exchangeability::loadHeader(FILE* fp, char*** names, int* nVars, int** permids): begin\n");
    Dddmp_DecompType ddType; //possible Values: DDDMP_BDD,DDDMP_ADD,DDDMP_CNF,DDDMP_NONE
    int nsuppvars;
    char** orderedVarNames;
    char** suppVarNames;
    int* ids;
    int* auxids; //Hilfsvariablen?
    int nRoots;

    Dddmp_cuddHeaderLoad (
    &ddType         /* OUT: selects the proper decomp type */,
    nVars           /* OUT: number of DD variables */,
    &nsuppvars      /* OUT: number of support variables */,
    &suppVarNames   /* OUT: array of support variable names */,
    &orderedVarNames/* OUT: array of variable names */,
    &ids            /* OUT: array of variable ids */,
    permids         /* OUT: array of permids ids */,
    &auxids         /* OUT: array of variable aux ids */,
    &nRoots         /* OUT: number of root in the file */,
    NULL            /* IN: file name */,
    fp              /* IN: file pointer */
    );
    
    if (ddType != DDDMP_BDD) {cout << "\nFehler beim Laden des BDDs: DD-Typ falsch"; exit(1);}
    if (nRoots != 1) {cout << "\nFehler beim Laden des BDDs: in Datei muss genau ein BDD enthalten sein"; exit(1);}
    if (names != NULL){
        if (suppVarNames == NULL) {cout << "FEHLER suppVarNames == NULL"; exit(1);}
        *names = suppVarNames;
    }
    if (nVars == 0) {cout << "FEHLER nVars == 0"; exit(1);}
    trace(TRACE_5,"Exchangeability::loadHeader(FILE* fp, char*** names, int* nVars, int** permids): end\n");
}

// optimale Reihenfolge der Variablen im Manager wieder bilden  
void Exchangeability::loadOptimalOrder(DdManager* mgr, int size, int* permids){ 
    int length; 
    if (size < Cudd_ReadSize(mgr)){
    	length = Cudd_ReadSize(mgr);  
    }
    else {
    	length = size;
    }
    int idOrder[length]; //The size of idOrder should be equal or greater to the number of variables currently in use in mgr
    for (int i = 0; i < size; ++i){
    	assert(permids[i] < size);
        idOrder[permids[i]] = i;
    }
    for(int i = size; i < length; ++i){
    	idOrder[i] = i;
    }

    assert (length >= Cudd_ReadSize(mgr));
    int res = Cudd_ShuffleHeap(mgr,idOrder);
    assert(res == 1);	
} 

DdNode* Exchangeability::loadDiagram(FILE* fp, DdManager* mgr){
	trace(TRACE_5,"Exchangeability::loadDiagram(FILE* fp, DdManager* mgr, int size, int* permids): begin\n");
    
    DdNode* bdd = Dddmp_cuddBddLoad (
            mgr                 /* IN: DD Manager */,
            DDDMP_VAR_MATCHIDS  /* IN: storing mode selector */,
            NULL                /* IN: array of variable names - by IDs */,
            NULL                /* IN: array of variable auxids - by IDs */,
            NULL                /* IN: array of new ids accessed - by IDs */,
            DDDMP_MODE_TEXT     /* IN: requested input file format */,
            NULL                /* IN: file name */,
            fp                  /* IN: file pointer */
    );
    
    trace(TRACE_5,"Exchangeability::loadDiagram(FILE* fp, DdManager* mgr, int size, int* permids): end\n");
    return (bdd);
}


void Exchangeability::printDotFile(char* filename, char** varNames, DdNode* bddMp, DdNode* bddAnn){
	trace(TRACE_5,"Exchangeability::printDotFile(char* filename, char** varNames, DdNode* bddMp, DdNode* bddAnn): begin\n");
	if ((Cudd_DagSize(bddMp) < 200000) && (Cudd_DagSize(bddAnn) < 200000) ) {
	
		char bufferMp[256]; 
		char bufferAnn[256];

        sprintf(bufferMp, "%s.Exchangeability.BDD_MP.out", filename);
        sprintf(bufferAnn, "%s.Exchangeability.BDD_ANN.out", filename); 

        FILE* fpMp;
	    fpMp = fopen(bufferMp, "w");
	    Cudd_DumpDot(mgrMp, 1, &bddMp, varNames, NULL, fpMp);
	    fclose(fpMp);
	    
	    FILE* fpAnn;
	    fpAnn = fopen(bufferAnn, "w");
	    Cudd_DumpDot(mgrAnn, 1, &bddAnn, varNames, NULL, fpAnn);
	    fclose(fpAnn);
	    
	    if ((Cudd_DagSize(bddMp) < 900) && (Cudd_DagSize(bddAnn) < 900)) {
            sprintf(bufferMp, "dot -Tpng %sexchangeability.BDD_MP.out -o %sexchangeability.BDD_MP.png", filename, filename);
            sprintf(bufferAnn, "dot -Tpng %sexchangeability.BDD_ANN.out -o %sexchangeability.BDD_ANN.png", filename, filename);

            system(bufferMp);
            system(bufferAnn);
         
        } 
    }
	trace(TRACE_5,"Exchangeability::printDotFile(char* filename, char** varNames, DdNode* bddMp, DdNode* bddAnn): end\n");     
}      


bool Exchangeability::check(Exchangeability* bdd){
	trace(TRACE_5,"Exchangeability::check(Exchangeability* bdd): begin\n");
	
	if (this->labelList.size() != bdd->labelList.size()){
		return (false);
	}
	
	list<char*>::const_iterator thislist_iter = this->labelList.begin();
	list<char*>::const_iterator bddlist_iter = bdd->labelList.begin();
	while (thislist_iter != this->labelList.end()){ 	//this->labelList.size() is equal to bdd->labelList.size()
		//cout << *thislist_iter << *bddlist_iter << endl;
		if(strcmp(*thislist_iter, *bddlist_iter)){
			return(false);
		}
		++thislist_iter;
		++bddlist_iter;
	}
	
	if (Cudd_bddLeq(mgrMp, bddMp, bdd->bddMp) == 1  && Cudd_bddLeq(mgrAnn, bddAnn, bdd->bddAnn) == 1 ){
		return(true);
	}else{
		return(false);
	}
	trace(TRACE_5,"Exchangeability::check(Exchangeability* bdd): end\n");
}

void Exchangeability::reorder(Cudd_ReorderingType heuristic){

    Cudd_ReduceHeap(mgrMp, heuristic, 0);
	cout << "BDD_MP: number of nodes: " << Cudd_DagSize(bddMp);
	cout << "\t" << Cudd_ReadReorderingTime(this->mgrMp) << " ms consumed for variable reordering" << endl;
    
    Cudd_ReduceHeap(this->mgrAnn, heuristic, 0);
	cout << "BDD_ANN: number of nodes: "  << Cudd_DagSize(this->bddAnn);
	cout << "\t" << Cudd_ReadReorderingTime(this->mgrAnn) << " ms consumed for variable reordering" << endl;
}

void Exchangeability::printMemoryInUse(){
	cout << endl;
//	cout << "Number of live nodes in mgrMp: " << Cudd_ReadNodeCount(mgrMp) << endl;
//	cout << "Peak number of nodes in mgrMp: " << Cudd_ReadPeakNodeCount(mgrMp) << endl;
//	cout << "Number of dead nodes in mgrMp: " << Cudd_ReadDead(mgrMp) << endl;
//	cout << "Number of live nodes in mgrAnn: " << Cudd_ReadNodeCount(mgrAnn) << endl;
	cout << "Memory in use for mgrMp:  " << Cudd_ReadMemoryInUse(mgrMp)<< " bytes" << endl;	
	cout << "Memory in use for mgrAnn: " << Cudd_ReadMemoryInUse(mgrAnn)<<" bytes" << endl;
	cout << "Memory in use for both BDD: " << Cudd_ReadMemoryInUse(mgrMp) + Cudd_ReadMemoryInUse(mgrAnn)<< 
		    "  bytes (" << (Cudd_ReadMemoryInUse(mgrMp) + Cudd_ReadMemoryInUse(mgrAnn))/1024 << " KB; " << (Cudd_ReadMemoryInUse(mgrMp) + Cudd_ReadMemoryInUse(mgrAnn))/(1024*1024) << " MB)" << endl;
}
