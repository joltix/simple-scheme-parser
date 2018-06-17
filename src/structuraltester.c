#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "lexer.h"
#include "parser.h"
#include "evaluation.h"

// Prototype for function responsible for checking for the
// exit command (exit) from the user
static void exitCheck(List*);

/****************************************************************
 Tests the usage of the functions outlined in the parser header.
*/
int main()
{
    // Prompt according to the sample run
    printf("A prototype evaluator for Scheme.\n");
    printf("Type Scheme expressions using quote,\n");
    printf("car, cdr, cons and symbol?.\n");
    printf("The function call (exit) quits.\n");

    // Repeatedly handle scheme expressions
    startTokens(20);
    while (1) {
        printf("\nscheme> ");
        // Read and print a given expression
        List* list = S_Expression();
        // Check input for (exit) command
        exitCheck(list);
        // Evaluate the input
        List* evalList = eval(list);
        // Print the evaluated structure
        printList(evalList);
    }
}

/****************************************************************
 Private function for checking for the user's exit command,
 "(exit)". The given List is the structure of the user's input
 processed by S_Expression(List*). This function ignores input
 whose symbols just happen to include the String "exit". Instead,
 the user must explicitly have only "(exit)" as the input to end
 the program.
*/
static void exitCheck(List* list)
{
    // Catch a user's (exit) command
    if (list->mStructure->mSub != NULL) {
        Cell* sub = list->mStructure->mSub;
        Cell* next = list->mStructure->mNext;
        if (next == NULL && sub != NULL && sub->mSymbol != NULL) {
            if (strcmp(sub->mSymbol, "exit") == 0) {
                printf("Have a nice day!\n");
                exit(0);
            }
        }
    }
}
