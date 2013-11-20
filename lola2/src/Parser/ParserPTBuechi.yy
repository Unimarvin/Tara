%{
#include <config.h>
#include <limits.h>
#include <libgen.h>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <set>
#include <cmdline.h>
#include <Core/Dimensions.h>
#include <Parser/PlaceSymbol.h>
#include <Parser/TransitionSymbol.h>
#include <Parser/SymbolTable.h>
#include <Parser/ParserPTNet.h>
#include <Parser/FairnessAssumptions.h>
#include <Parser/ArcList.h>
#include <InputOutput/Reporter.h>
#include <InputOutput/InputOutput.h>

#include <Parser/ast-system-k.h>
#include <Parser/ast-system-yystype.h>

extern ParserPTNet* symbolTables;
extern SymbolTable* buechiStateTable;

extern gengetopt_args_info args_info;

using namespace kc;

/// the current token text from Flex
extern char* ptbuechi_text;

void ptbuechi_error(char const*);
void ptbuechi_yyerrors(char* token, const char* format, ...);
%}

%error-verbose /* more verbose and specific error message string */
%defines       /* write an output file containing macro definitions for the token types */
%name-prefix="ptbuechi_"

%type <yt_integer> NUMBER
%type <yt_casestring> IDENTIFIER

%type <yt_tBuechiAutomata> buechiAutomata
%type <yt_tBuechiRules> buechiRule
%type <yt_tBuechiRules> buechiRules
%type <yt_tTransitionRules> transitionRules
%type <yt_tAcceptingSet> acceptingsets
%type <yt_tStatePredicate> statepredicate
%type <yt_tAtomicProposition> atomic_proposition
%type <yt_tTerm> term

%token IDENTIFIER          "identifier"
%token NUMBER              "number"
%token _accept_            "keyword accept"
%token _buechi_            "keyword buechi"
%token _braceleft_         "opening brace"
%token _braceright_        "closing brace"        
%token _comma_             "comma"
%token _then_              "transition =>"
%token _colon_             "colon"
%token _INITIAL_           "keyword INITIAL"
%token _AND_               "Boolean conjuction"
%token _NOT_               "Boolean negation"
%token _OR_                "Boolean disjunction"
%token _XOR_               "Boolean exclusive disjunction"
%token _iff_               "Boolean iff"
%token _notequal_          "not-equals sign"
%token _implies_           "Boolean implication"
%token _equals_            "equals sign"
%token _plus_              "plus sign"
%token _minus_             "minus sign"
%token _times_             "multiplication sign"
%token _leftparenthesis_   "opening parenthesis"
%token _rightparenthesis_  "closing parenthesis"
%token _greaterthan_       "greater-than sign"
%token _lessthan_          "less-than sign"
%token _greaterorequal_    "greater-than-or-equal sign"
%token _lessorequal_       "less-than-or-equal sign"
%token _semicolon_         "semicolon"
%token _TRUE_              "Boolean TRUE"
%token _FALSE_             "Boolean FALSE"
%token _FIREABLE_          "keyword FIREABLE"
%token _DEADLOCK_          "keyword DEADLOCK"


// precedences (lowest written first, e.g. PLUS/MINUS) and precedences
%left _OR_ _XOR_
%left _AND_
%left _implies_
%left _iff_
%left _equals_ _notequal_
%left _lessthan_ _lessorequal_ _greaterthan_ _greaterorequal_
%left _plus_ _minus_
%left _times_
%right _NOT_
%right _ALWAYS_ _EVENTUALLY_ _NEXTSTATE_
%right _UNTIL_
%right _ALLPATH_ _EXPATH_
%right _REACHABLE_ _INVARIANT_ _IMPOSSIBLE_


%{
extern YYSTYPE ptbuechi_lval;
extern int ptbuechi_lex();
extern FILE* ptbuechi_in;
extern int ptbuechi_lineno;
extern int ptbuechi_colno;

int currentNextIndex = 0;
//std::set<index_t> target_place;
//std::set<index_t> target_transition;
%}

%{
/* globals */
tBuechiAutomata TheBuechi;
uint32_t currentState;
%}

%%

buechiAutomata:
  _buechi_ _braceleft_ buechiRules _braceright_  _accept_ _braceleft_ acceptingsets _braceright_ _semicolon_
    { TheBuechi = BuechiAutomaton($3,$7); }
;

buechiRules:
  /* empty */                     { $$ = EmptyBuechiRules(); }
| buechiRule                      { $$ = $1; }
| buechiRule _comma_ buechiRules  { $$ = BuechiRules($1,$3); }
;

buechiRule:
  IDENTIFIER {
  	Symbol *t = buechiStateTable->lookup($1->name);
  	if (t == NULL){
  		t = new Symbol($1->name);
  		buechiStateTable->insert(t);
  		t->setIndex(currentNextIndex++);
  	}
  }
   _colon_ transitionRules { Symbol *t = buechiStateTable->lookup($1->name); $$ = BuechiRule((mkinteger(t->getIndex())),$4); $1->free(true);}
;

transitionRules:
  /* empty */   { $$ = EmptyTransitionRules(); }
| statepredicate _then_ IDENTIFIER transitionRules
    {
        Symbol *t = buechiStateTable->lookup($3->name);
	  	if (t == NULL){
	  		buechiStateTable->insert(new Symbol($3->name));
	  		t = buechiStateTable->lookup($3->name);
	  		t->setIndex(currentNextIndex++);
	  	}
	  	//$3->free(true);
        $$ = TransitionRules(TransitionRule(StatePredicateFormula($1),mkinteger(t->getIndex())),$4);
    }
;

acceptingsets:
  /* empty */     { $$ = EmptyAcceptingSet(); }
| IDENTIFIER
    {
        Symbol *t = buechiStateTable->lookup($1->name);
        if (t == NULL)
        {
            ptbuechi_yyerrors(ptbuechi_text, "state %s unknown", $1->name);
        }
        //$1->free(free);
        $$ = AcceptingState(mkinteger(t->getIndex()));
    }
| IDENTIFIER _comma_ acceptingsets
    {
        Symbol *t = buechiStateTable->lookup($1->name);
        if (t == NULL)
        {
            ptbuechi_yyerrors(ptbuechi_text, "state %s unknown", $1->name);
        }
        //$1->free(true);
        $$ = AcceptingSet(AcceptingState(mkinteger(t->getIndex())),$3);
    }
;

statepredicate:
  _leftparenthesis_ statepredicate _rightparenthesis_
    { $$ = $2; }
| atomic_proposition
    { $$ = AtomicProposition($1); }
| _NOT_ statepredicate
    { $$ = Negation($2); }
| statepredicate _AND_ statepredicate
    { $$ = Conjunction($1, $3); }
| statepredicate _OR_ statepredicate
    { $$ = Disjunction($1, $3); }
| statepredicate _XOR_ statepredicate
    { $$ = ExclusiveDisjunction($1, $3); }
| statepredicate _implies_ statepredicate
    { $$ = Implication($1, $3); }
| statepredicate _iff_ statepredicate
    { $$ = Equivalence($1, $3); }
;

atomic_proposition:
  term _equals_ term
    { $$ = EqualsAtomicProposition($1, $3); }
| term _notequal_ term
    { $$ = NotEqualsAtomicProposition($1, $3); }
| term _greaterthan_ term
    { $$ = GreaterAtomicProposition($1, $3); }
| term _greaterorequal_ term
    { $$ = GreaterEqualAtomicProposition($1, $3); }
| term _lessthan_ term
    { $$ = LessAtomicProposition($1, $3); }
| term _lessorequal_ term
    { $$ = LessEqualAtomicProposition($1, $3); }
| _TRUE_
    { $$ = True(); }
| _FALSE_
    { $$ = False(); }
| _FIREABLE_ _leftparenthesis_ IDENTIFIER _rightparenthesis_
    {
        Symbol *t = symbolTables->TransitionTable->lookup($3->name);
        if (t == NULL)
        {
            ptbuechi_yyerrors(ptbuechi_text, "transition %s unknown", $3->name);
        }
        //$3->free(true);
        $$ = Fireable(mkinteger(t->getIndex()));
        //target_transition.insert(t->getIndex());
    }
| _DEADLOCK_
    { $$ = aDeadlock(); }
| _INITIAL_
    { $$ = Initial(); }
;

term:
  _leftparenthesis_ term _rightparenthesis_
    { $$ = $2; }
| IDENTIFIER
    {
        Symbol *p = symbolTables->PlaceTable->lookup($1->name);
        if (p == NULL)
        {
            ptbuechi_yyerrors(ptbuechi_text, "place %s unknown", $1->name);
        }
        //$1->free(true);
        $$ = Node(mkinteger(p->getIndex()));
      	//target_place.insert(p->getIndex());
    }
| NUMBER
    { $$ = Number($1); }
| term _plus_ term
    { $$ = Sum($1, $3); }
| term _minus_ term
    { $$ = Difference($1, $3); }
| NUMBER _times_ term
    { $$ = Product($1, $3); }
;


%%

/// display a parser error and exit
void ptbuechi_yyerrors(char* token, const char* format, ...) __attribute__((noreturn));
void ptbuechi_yyerrors(char* token, const char* format, ...) {
    va_list args;
    va_start(args, format);
    char* errormessage = NULL;
    const int res = vasprintf(&errormessage, format, args);
    assert(res != -1);
    rep->message(errormessage);
    free(errormessage);
    va_end(args);

    rep->message("%s:%d:%d - error near '%s'", rep->markup(MARKUP_FILE, basename((char*)args_info.formula_arg)).str(), ptbuechi_lineno, ptbuechi_colno, token);
//    rep->status("%d:%d - error near '%s'", ptbuechi_lineno, ptbuechi_colno, token);

    rep->abort(ERROR_SYNTAX);
    exit(EXIT_ERROR); // needed to corrently recognize noreturn since rep->abort is virtual
}

/// display a parser error and exit
void ptbuechi_error(char const* mess) __attribute__((noreturn));
void ptbuechi_error(char const* mess) {
    ptbuechi_yyerrors(ptbuechi_text, mess);
}
