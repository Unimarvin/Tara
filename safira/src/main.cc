#include "config.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include <cstdio>
#include <map>
#include <algorithm>
#include "Node.h"
#include "Graph.h"
#include "helpers.h"
#include "cmdline.h"
#include "verbose.h"
#include "config-log.h"

#include "testFormula.h"
#include "testNode.h"
#include "testGraph.h"

/// the command line parameters
gengetopt_args_info args_info;


using namespace std;

string invocation;

// lexer and parser
extern int og_yyparse();
extern FILE* og_yyin;
extern int og_yylex_destroy();

extern Graph * graph;

extern map<string, int> label2id;
extern map<int, string> id2label;

extern int currentIdPos;
extern int firstLabelId; //all labels including tau
extern int firstInputId; //input labels
extern int firstOutputId;//output labels

extern int lastLabelId;
extern int lastInputId;
extern int lastOutputId;

void testGraph();
void initGlobalVariables();
void evaluateParameters(int argc, char** argv);
bool fileExists(string filename);

int main(int argc, char **argv) {
	evaluateParameters(argc, argv);

//	testFormulaClass();
//	testNodeClass();
//	testGraphClass();
//	cout << "test finished" << endl << endl;

	if (args_info.inputs_num > 1) {
		//fehler
	}

	string filename = "stdin";

	if (args_info.inputs_num == 1){
		assert(args_info.inputs != NULL);
		assert(args_info.inputs[0] != NULL);

		filename = args_info.inputs[0];
		filename = filename.substr(0,filename.find_last_of(".og")-2);
		og_yyin = fopen(args_info.inputs[0], "r");

	    if (og_yyin == NULL){
	    	abort(1, "File %s not found", args_info.inputs[0]);
	    }

	}

	Graph * g = new Graph();
	graph = g;

	initGlobalVariables();

	time_t parsingTime_start = time(NULL);

    //parse
    if ( og_yyparse() != 0) {cout << PACKAGE << "\nparse error\n" << endl; exit(1);}

    fclose(og_yyin);
    og_yylex_destroy();

//    time_t parsingTime_end = time(NULL);
//    cout << "number of labels: " << label2id.size()-4 << endl;
//    cout << "number of nodes in the given extended annotated automaton: " << graph->nodes.size() << endl;
//    cout << difftime(parsingTime_end, parsingTime_start) << " s consumed for parsing the file" << endl;
//    time_t buildOG_start = time(NULL);

	g->makeTotal();
	g->makeComplete();
	g->print();

//	time_t buildOG_end = time(NULL);
//	cout << "number of nodes in the complement: " << graph->nodes.size() + graph->getSizeOfAddedNodes() << endl;
//	cout << difftime(buildOG_end, buildOG_start) << " s consumed for building the complement" << endl;
//	cout << Formula::getMinisatTime() << "s consumed by minisat" << endl;

	for (int j = 0; j<args_info.output_given; ++j){
		switch(args_info.output_arg[j]) {

		// create output using Graphviz dot
		case (output_arg_png):
		case (output_arg_eps):
		case (output_arg_pdf): {
			if (CONFIG_DOT == "not found") {
				cerr << PACKAGE << ": Graphviz dot was not found by configure script; see README" << endl;
				cerr << "       necessary for option '--output=owfn'" << endl;
				exit(EXIT_FAILURE);
			}

			string call = string(CONFIG_DOT) + " -T" + args_info.output_orig[j] + " -q -o " + filename+ "." + args_info.output_orig[j];
			FILE *s = popen(call.c_str(), "w");
			assert(s);
			//fprintf(s, "%s\n", d.str().c_str());
			string title = "Complement of " + filename + "    global formula: " + g->globalFormula->toString();
			g->toDot(s, title);
			assert(!ferror(s));

			pclose(s);
		}
		}
	}

	delete g;

//	free(args_info.minisat_arg);
	cmdline_parser_free(&args_info);
    return EXIT_SUCCESS;
}

void initGlobalVariables(){
    label2id.clear();
    id2label.clear();

    addLabel("", 0); //0 has a special meaning in minisat, therefore 0 cannot use as ID for labels
    addLabel("true", 1);
    addLabel("false", 2);
    addLabel("final", 3);
    addLabel("tau", 4);

	currentIdPos = 4;
//	currentIdPos = 3;
	firstLabelId = 4;



}


//test after parsing (for spec_klein.og)
void testGraph(){
	assert(firstLabelId == 4);
	assert(firstInputId == 5);
	assert(firstOutputId == 7);
	assert(lastLabelId == 8);
	assert(lastInputId = 6);
	assert(lastOutputId == 8);
	assert(currentIdPos == lastOutputId);
	//assert(graph->initialNodes.size() == 1);
}

bool fileExists(std::string filename) {
    FILE *tmp = fopen(filename.c_str(), "r");
    if (tmp) {
        fclose(tmp);
        return true;
    } else {
        return false;
    }
}


/// evaluate the command line parameters
void evaluateParameters(int argc, char** argv) {

/*for gengetopt: begin*/
    // overwrite invokation for consistent error messages
    argv[0] = (char*)PACKAGE;

/*
    // debug option
    if (argc > 0 and std::string(argv[1]) == "--bug") {
        FILE *debug_output = fopen("bug.log", "w");
        fprintf(debug_output, "%s\n", CONFIG_LOG);
        fclose(debug_output);
        fprintf(stderr, "Please send file 'bug.log' to %s.\n", PACKAGE_BUGREPORT);
        exit(EXIT_SUCCESS);
    }
*/

    // store invocation in a string for meta information in file output
    for (int i = 0; i < argc; ++i) {
        invocation += string(argv[i]) + " ";
    }

    // set default values
    //cmdline_parser_init(&args_info);

    // initialize the parameters structure
    struct cmdline_parser_params *params = cmdline_parser_params_create();

    // call the cmdline parser
    if (cmdline_parser(argc, argv, &args_info) != 0) {
        abort(7, "invalid command-line parameter(s)");
    }

    // read a configuration file if necessary
    if (args_info.config_given) {
        // initialize the config file parser
        params->initialize = 0;
        params->override = 0;

        // call the config file parser
        if (cmdline_parser_config_file (args_info.config_arg, &args_info, params) != 0) {
            abort(14, "error reading configuration file '%s'", args_info.config_arg);
        } else {
            status("using configuration file '%s'", args_info.config_arg);
        }
    } else {
        // check for configuration files
        string conf_filename = fileExists("safira.conf") ? "safira.conf" :
                               (fileExists(string(SYSCONFDIR) + "/safira.conf") ?
                               (string(SYSCONFDIR) + "/safira.conf") : "");

        if (conf_filename != "") {
            // initialize the config file parser
            params->initialize = 0;
            params->override = 0;
            if (cmdline_parser_config_file ((char*)conf_filename.c_str(), &args_info, params) != 0) {
                abort(14, "error reading configuration file '%s'", conf_filename.c_str());
            } else {
                status("using configuration file '%s'", conf_filename.c_str());
            }
        } else {
            status("not using a configuration file");
        }
    }

//    StoredKnowledge::reportFrequency = args_info.reportFrequency_arg;

    // check whether at most one file is given
    if (args_info.inputs_num > 1) {
        abort(4, "at most one input file must be given");
    }


    // check the message bound
//    if ((args_info.messagebound_arg < 1) or (args_info.messagebound_arg > UINT8_MAX)) {
//        abort(9, "message bound must be between 1 and %d", UINT8_MAX);
//    }

    free(params);
}

