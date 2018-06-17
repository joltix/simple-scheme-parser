#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parser.h"
#include "evaluation.h"
#include "lexer.h"


/****************************************************************
 File: Parser.c
 ----------------
 Implementation for parser.h interface. This will build out the
 cons cell structure for a valid scheme expression. There are
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

// Private members and "constants"
static char* mToken;

// Prototypes for private helper functions
static Cell* recurse_express();
static void recurse_print(Cell*, int);
static Cell* iniCell();

/****************************************************************
 Private helper for S_Expression that recurses and returns a
 pointer to the first cell in the structure. It will be up to
 the main S_Expression function to wrap it in a List.
*/
static Cell* recurse_express()
{
    Cell* shortHand = NULL;
    Cell* local = NULL;
    Cell* temp = NULL;

    // Structure single quote as explicit "(quote"
    if (strcmp(mToken, "'") == 0) {
        shortHand = iniCell();
        shortHand->mSub = iniCell();
        shortHand->mSub->mSymbol = "quote";
        shortHand->mNext = iniCell();
        shortHand->mNext->mSub = iniCell();
        local = shortHand->mNext->mSub;
        strcpy(mToken, getToken());
        // Not seeing an open parenthesis means single quoting standalone symbol (not a list)
        if (strcmp(mToken, "(") != 0) {
            Cell* singleSymbol = shortHand->mNext->mSub;
            singleSymbol->mSymbol = malloc(sizeof(char) * 20);
            strcpy(singleSymbol->mSymbol, mToken);
            return shortHand;
        }
    }

    if (strcmp(mToken, "(") == 0) {
        // Get next Token and iterate through rest
        strcpy(mToken, getToken());
        // In case no single quote was detected before
        if (shortHand == NULL)
            local = iniCell();

        local->mSub = recurse_express();

        // Get next Token
        strcpy(mToken, getToken());
        temp = local;
        while (strcmp(mToken, ")") != 0) {
            // Create and focus on another node
            temp->mNext = iniCell();
            temp = temp->mNext;
            temp->mSub = recurse_express();
            // Update to next token
            strcpy(mToken, getToken());
        }

        // Found end of level
        temp->mNext = NULL;
    } else {
        // Attach dynamic symbol to the local to become "first"
        local = iniCell();
        local->mSymbol = malloc(sizeof(char) * 20);
        strcpy(local->mSymbol, mToken);
    }
    if (shortHand != NULL)
        return shortHand;
    return local;
}

/****************************************************************
 Function to call for building the structure of the code input.
 A pointer to a List is returned which contains a pointer to the
 input's structure. Note: the input is not evaluated.
*/
List* S_Expression()
{
    // Pull the first token for parsing
    mToken = malloc(sizeof(char) * 20);
    strcpy(mToken, getToken());
    // Parse for structure
    List* list = malloc(sizeof(List));
    list->mStructure = recurse_express();
    return list;
}

/****************************************************************
 Prints the structure of the given List on one line.
*/
void printList(List* list)
{
    // Check for special case: true/false
    printf(" ");
    // Don't print anything for function "define" which gives no
    // feedback
    if (list != NULL) {
        if (list->mStructure == FALSE) printf("()");
        else if (list->mStructure == TRUE) printf("#t");
        // Case of single symbol
        else if (list->mStructure != NULL && list->mStructure->mSymbol != NULL) {
            printf("%s", list->mStructure->mSymbol);
        } else if (list->mStructure != NULL) {
            // Normal case structure
            printf("(");
            recurse_print(list->mStructure, 0);
            printf(")");
        }
    }
    printf("\n");
}

/****************************************************************
 Helper to recursively print the structure of a List from
 printList(List*).
*/
static void recurse_print(Cell* cell, int level)
{
    // Print the symbol
    if (cell->mSub != NULL && cell->mSub->mSymbol != NULL) {
        printf(" %s ", cell->mSub->mSymbol);
    // Recurse down and print open parenth with each level
    } else if (cell->mSub != NULL) {
        printf("(");
        recurse_print(cell->mSub, level + 1);
    }

    if (cell->mNext != NULL) recurse_print(cell->mNext, level);
    else if (level != 0) printf(")");

}

/****************************************************************
 Helper function dynamically allocating a new cons cell. All
 members of the output Cell is initialized to NULL.
*/
static Cell* iniCell()
{
    Cell* cell = malloc(sizeof(Cell));
    cell->mSub = NULL;
    cell->mNext = NULL;
    cell->mSymbol = NULL;
    return cell;
}
