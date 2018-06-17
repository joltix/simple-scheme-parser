#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

/****************************************************************
 File: Parser.h
 ----------------
 Interface for Parcer, a cons cell structure builder, evaluator,
 and printer.

 There are
 three functions of interest:

 1) List* S_Expression()
 2) List* eval(List*)
 3) void printList(List*)

 When executed in the order above, while passing output List* to
 the next function, a valid scheme expression will be evaluated
 and the answer will be printed out onto the console. If
 eval(List*) is not called, printList(List*) will print the cons
 cell structure of the input similar to the style it was given.

 Author: Christian Ramos
 ****************************************************************/

/****************************************************************
 Cell to reference the next cons cell or a symbol
 ****************************************************************/
typedef struct node Cell;
struct node {
    char* mSymbol;
    // "first"
    Cell* mNext;
    // "rest"
    Cell* mSub;
};

/****************************************************************
 Wrapper for a structure of cons cells. Note that the pointer
 mStructure is used as a jumping off point to two other structures
 referenced by mStructure's mSub pointer and mNext pointer in the
 case of cons'd structures. In the case that the List must
 represent #t or #f, the mStructure itself would equate special
 globals TRUE / FALSE, and for all other cases only mSub would
 reference the structure to examine.
*/
typedef struct stringOfCells List;
struct stringOfCells {
    Cell* mStructure;
};

/****************************************************************
 Function to call for building the structure of the given code
 input.
*/
List* S_Expression();

/****************************************************************
 Prints the structure of the given List on one line.
*/
void printList(List*);

#endif
