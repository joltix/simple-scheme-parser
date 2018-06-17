#ifndef EVALUATION_H_INCLUDED
#define EVALUATION_H_INCLUDED

#include "parser.h"

/****************************************************************
 File: Evaluation.h
 ----------------
 Interface for Evaluation, a package of functions for evaluating
 cons cell structures produced by Parser.

 Author: Christian Ramos
 ****************************************************************/

// TRUE and FALSE constants to be used across modules
extern Cell* TRUE;
extern Cell* FALSE;

/****************************************************************
 Evaluates the structure within the given List and produces
 a List containing the structure of the evaluated code ready
 for printing. Currently, the only functions supported are
 car, cdr, quote, and cons with #f is equivalent to the empty
 list "()".
*/
List* eval(List*);

#endif
