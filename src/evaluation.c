#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parser.h"
#include "lexer.h"


/****************************************************************
 File: Evaluation.c
 ----------------
 Implementation for evaluation.h interface. This will evaluate
 all levels of a Scheme expression provided the cons cell
 structure is produced by the accompanying Parser.

 Note that #f is equivalent to the empty list "()" except for
 not matching in an association list. In that case, #f is
 explicitly returned as per the assignment page example.

 Besides integer operations +, -, and *, here is a list of
 currently supported functions:
    list
    length
    last
    list?
    number?
    >
    >=
    <
    <=
    car
    cdr
    cadr
    caddr
    cadddr
    caddddr
    cdar
    symbol?
    null?
    equal?
    cond + cond else
    if
    quote + '
    cons
    append
    assoc
    define


 Author: Christian Ramos
 ****************************************************************/


// Association list for variables
static List* mAssocVars = NULL;
static List* mAssocFns = NULL;

// Constants for TRUE / FALSE
Cell* TRUE = NULL;
Cell* FALSE = NULL;

// Prototypes for helpers to the main scheme functions
static List* wrapStructure(Cell*);
static Cell* iniCell();
static List* iniAssocList();
static int isEmptyStructure(Cell*);
static List* bindLocals(List*, List*, List*, List*);
static List* defineFunction(List*, List*);
static List* assocForVar(Cell*, List*);
static List* assocForFn(Cell*);
static List* recurse_eval(Cell*, List*);
static Cell* compareEqual(Cell*, Cell*);
static Cell* findAssoc(Cell*, Cell*);
static Cell* appendSubstitute(Cell*, List*);
// Prototypes for the main scheme functions the user can use
static List* quote(List*);
static List* makeList(Cell*, List*);
static List* last(List*);
static List* length(List*);
static List* add(Cell*, List*);
static List* subtract(Cell*, List*);
static List* multiply(Cell*, List*);
static List* logicAnd(Cell*, List*);
static List* logicOr(Cell*, List*);
static List* logicNot(List*);
static List* lessThan(List*, List*);
static List* greaterThan(List*, List*);
static List* lessThanOrEqualTo(List*, List*);
static List* greaterThanOrEqualTo(List*, List*);
static List* car(List*);
static List* cdr(List*);
static List* cadr(List*);
static List* caddr(List*);
static List* cadddr(List*);
static List* caddddr(List*);
static List* cdar(List*);
static List* isSymbol(List*);
static List* cons(List*, List*);
static List* isNull(List*);
static List* assoc(Cell*, List*);
static List* isEqual(List*, List*);
static List* append(List*, List*);
static List* cond(Cell*, List*);
static List* alternateIf(Cell*, List*);
static List* define(List*, List*, List*);
static List* isList(List*);
static List* isNumber(List*);

/****************************************************************
 Sets up globals such as the TRUE / FALSE "constants" to make
 #t / #f detection simpler as well as the referencing environment.
 */
static void setupGlobals()
{
    // Setup TRUE/FALSE
    if (TRUE == NULL || FALSE == NULL) {
        TRUE = iniCell();
        FALSE = iniCell();
    }

    // Setup reference variables environment
    if (mAssocVars == NULL) {
        mAssocVars = iniAssocList();
    }
    // Setup reference functions environment
    if (mAssocFns == NULL) mAssocFns = iniAssocList();
}

/****************************************************************
 Evaluates the structure within the given List and produces
 a List containing the structure of the evaluated code ready
 for printing. Note that #f is equivalent to the empty
 list "()".
*/
List* eval(List* list)
{
    // Prep global members
    setupGlobals();
    return recurse_eval(list->mStructure, mAssocVars);
}
/****************************************************************
 Helper for eval(List*) to recursively evaluate the structure of
 the List given to eval(List*).
*/
static List* recurse_eval(Cell* cell, List* environment)
{
    // Detect a symbol in the cell below the current in focus
    // and check if the symbol matches a supported function.
    // After a match with the list of if/else comparisons
    // below, the corresponding function is called with
    // a recursive evaluation of the presumed next parameters
    // passed in, and where a function is only given the current
    // cell, the recursion is handled specially within the function
    List* list = NULL;
    int atomBelow = 0;
    if (cell->mSub != NULL) {
        char* sym = cell->mSub->mSymbol;
        // Drop a level since no function yet
        if (sym == NULL) {
            list = recurse_eval(cell->mSub, environment);
        // No need to recurse further if found a quote
        } else if (strcmp(sym, "quote") == 0) {
            list = malloc(sizeof(List));
            if (cell->mNext->mSub->mSub == NULL) list->mStructure = cell->mNext->mSub;
            else list->mStructure = cell->mNext->mSub;
            return quote(list);
            // Guide recursion down specific branch for cons params
        } else if (strcmp(sym, "cons") == 0) {
            return cons(recurse_eval(cell->mNext->mSub, environment), recurse_eval(cell->mNext->mNext->mSub, environment));
            // Recurse evaluate car function's param
        } else if (strcmp(sym, "list") == 0) {
            return makeList(cell, environment);
        } else if (strcmp(sym, "last") == 0) {
            return last(recurse_eval(cell->mNext->mSub, environment));
        } else if (strcmp(sym, "length") == 0) {
            return length(recurse_eval(cell->mNext->mSub, environment));
        } else if (strcmp(sym, "+") == 0) {
            return add(cell, environment);
        } else if (strcmp(sym, "-") == 0) {
            return subtract(cell, environment);
        } else if (strcmp(sym, "*") == 0) {
            return multiply(cell, environment);
        } else if ((strcmp(sym, "AND") == 0) || (strcmp(sym, "and") == 0)) {
            return logicAnd(cell, environment);
        } else if ((strcmp(sym, "OR") == 0) || (strcmp(sym, "or") == 0)) {
            return logicOr(cell, environment);
        } else if ((strcmp(sym, "NOT") == 0) || (strcmp(sym, "not") == 0)) {
            return logicNot(recurse_eval(cell->mNext->mSub, environment));
        } else if (strcmp(sym, "<") == 0) {
            return lessThan(recurse_eval(cell->mNext->mSub, environment), recurse_eval(cell->mNext->mNext->mSub, environment));
        } else if (strcmp(sym, ">") == 0) {
            return greaterThan(recurse_eval(cell->mNext->mSub, environment), recurse_eval(cell->mNext->mNext->mSub, environment));
        } else if (strcmp(sym, "<=") == 0) {
            return lessThanOrEqualTo(recurse_eval(cell->mNext->mSub, environment), recurse_eval(cell->mNext->mNext->mSub, environment));
        } else if (strcmp(sym, ">=") == 0) {
            return greaterThanOrEqualTo(recurse_eval(cell->mNext->mSub, environment), recurse_eval(cell->mNext->mNext->mSub, environment));
        } else if (strcmp(sym, "car") == 0) {
            return car(recurse_eval(cell->mNext->mSub, environment));
        // Recurse evaluate cdr function's param
        } else if (strcmp(sym, "cdr") == 0) {
            return cdr(recurse_eval(cell->mNext->mSub, environment));
        } else if (strcmp(sym, "cadr") == 0) {
            return cadr(recurse_eval(cell->mNext->mSub, environment));
        } else if (strcmp(sym, "caddr") == 0) {
            return caddr(recurse_eval(cell->mNext->mSub, environment));
        } else if (strcmp(sym, "cadddr") == 0) {
            return cadddr(recurse_eval(cell->mNext->mSub, environment));
        } else if (strcmp(sym, "caddddr") == 0) {
            return caddddr(recurse_eval(cell->mNext->mSub, environment));
        } else if (strcmp(sym, "cdar") == 0) {
            return cdar(recurse_eval(cell->mNext->mSub, environment));
        // Recurse evaluate symbol? function's param
        } else if (strcmp(sym, "symbol?") == 0) {
            return isSymbol(recurse_eval(cell->mNext->mSub, environment));
        } else if (strcmp(sym, "append") == 0) {
            return append(recurse_eval(cell->mNext->mSub, environment), recurse_eval(cell->mNext->mNext->mSub, environment));
        } else if (strcmp(sym,  "null?") == 0) {
            return isNull(recurse_eval(cell->mNext->mSub, environment));
        } else if (strcmp(sym, "equal?") == 0) {
            return isEqual(recurse_eval(cell->mNext->mSub, environment), recurse_eval(cell->mNext->mNext->mSub, environment));
        } else if (strcmp(sym, "define") == 0) {
            // Define either as a variable or a function
            List* key = recurse_eval(cell->mNext->mSub, environment);
            List* value = recurse_eval(cell->mNext->mNext->mSub, environment);
            if (isList(key)->mStructure == FALSE) {
                List* enviro = define(key, value, environment);
                // Update the global environment at first level of recursion
                if (environment == mAssocVars) mAssocVars = enviro;
                // Don't print anything - just defining
                return NULL;
            } else return defineFunction(key, wrapStructure(cell->mNext->mNext->mSub));
            //return define(recurse_eval(cell->mNext->mSub), recurse_eval(cell->mNext->mNext->mSub));
        } else if (strcmp(sym, "assoc") == 0) {
            return assoc(cell->mNext->mSub->mNext, recurse_eval(cell->mNext->mNext->mSub, environment));
        } else if (strcmp(sym, "cond") == 0) {
            return cond(cell, environment);
        } else if (strcmp(sym, "if") == 0) {
            return alternateIf(cell, environment);
        } else if (strcmp(sym, "number?") == 0) {
            return isNumber(recurse_eval(cell->mNext->mSub, environment));
        } else if (strcmp(sym, "list?") == 0) {
            return isList(recurse_eval(cell->mNext->mSub, environment));
            // Atom symbol found below current cell
        } else atomBelow = 1;
        // This case occurs during raw symbols not in a list
    } else if (cell->mSymbol != NULL){
        List* associated = assoc(cell, environment);
        // Try to associate the symbol
        if (associated->mStructure->mSymbol != NULL
            && (strcmp(associated->mStructure->mSymbol, "#f") == 0))
            return wrapStructure(cell);
        return car(cdr(associated));
    }
    // Recurse right then update last cell seen coming back left
    if (cell->mNext != NULL) {
        list = recurse_eval(cell->mNext, environment);
        list->mStructure = cell;
    } else {
        // Found a deep end of structure so go back up
        list = malloc(sizeof(List));
        list->mStructure = cell;
    }

    // Found an atom before evaluating deeper so try to associate when returning from recursion
    if (atomBelow == 1) {
        list = assocForFn(cell);

        // Different cell returned means a function was matched
        if (list->mStructure != cell) {
            // Extract the formal params
            List* formalParams = cdr(wrapStructure(list->mStructure->mSub));
            List* actualParams = cdr(wrapStructure(cell));

            // Generate local environment from user's definition and bind params
            List* localEnv = iniAssocList();
            localEnv = bindLocals(formalParams, actualParams, localEnv, environment);

            list = recurse_eval((car(cdr(list)))->mStructure, localEnv);
            // Symbol was not a function so try to identify as a variable
        } else list = assocForVar(cell, environment);
    }
    return list;
}

/****************************************************************
 Helper that binds a list of formal parameters with a list of
 actual parameters for a given environment. This function is
 meant to be used during evaluation of user defined functions.
*/
static List* bindLocals(List* formalParams, List* actualParams, List* newEnviro, List* oldEnviro)
{
    Cell* focus = formalParams->mStructure;
    Cell* param = actualParams->mStructure;

    while (focus != NULL) {
        // Fill the new environment with the new parameters
        newEnviro = define(wrapStructure(focus->mSub), recurse_eval(param->mSub, oldEnviro), newEnviro);
        // Move on to next parameter
        focus = focus->mNext;
        param = param->mNext;
    }
    return newEnviro;
}

/****************************************************************
 Essentially a call to assoc(Cell*, List*) but specifically
 associates with the assumption that the first of the pair
 is not a list. This function modifies the output list from
 assoc(Cell*, List*) such that not finding a match returns the
 given Cell instead of #f and only the value is returned without
 the identifier.
*/
static List* assocForVar(Cell* cell, List* environment)
{
    List* associated = assoc(cell, environment);
    // Return original cell if no association found
    if (associated->mStructure->mSymbol != NULL
        && (strcmp(associated->mStructure->mSymbol, "#f") == 0))
        return wrapStructure(cell);
    return cadr(associated);
}

/****************************************************************
 Essentially a call to assoc(Cell*, List*) but specifically
 associates with the global association list for functions. This
 function modifies the output list from assoc(Cell*, List*) such
 that not finding a match returns the given Cell instead of #f
 and only the value is returned without the identifier.
*/
static List* assocForFn(Cell* cell)
{
    // Return original cell if no association found
    List* associated = assoc(cell, mAssocFns);
    if ((associated->mStructure->mSymbol != NULL)
        && (strcmp(associated->mStructure->mSymbol, "#f") == 0))
        return wrapStructure(cell);
    return assoc(cell->mSub, mAssocFns);
}

/****************************************************************
 Helper function for recurse_eval(Cell*) for quoting params
 during evaluation.
*/
static List* quote(List* list)
{
    return list;
}

/****************************************************************
 Helper function for recurse_eval(Cell*) for taking the first
 element of a given List during evaluation.
*/
static List* car(List* list)
{
    list->mStructure = list->mStructure->mSub;
    return list;
}

/****************************************************************
 Helper function for recurse_eval(Cell*) for taking everything
 but the first element in a List during evaluation.
*/
static List* cdr(List* list)
{
    if (list->mStructure->mNext != NULL)
        list->mStructure = list->mStructure->mNext;
        // When there is nothing else in the list, return an empty list / #f
    else list->mStructure = iniCell();
    return list;
}

/****************************************************************
 Helper function for recurse_eval(Cell*) for checking whether or
 not the given List is a #t or #f (represented by the
 empty list "()") as a symbol during evaluation. The returned
 List pointer references a Cell which is itself either the
 global TRUE or FALSE "constant" Cell.
*/
static List* isSymbol(List* list)
{
    Cell* s = list->mStructure;
    if (s->mSymbol != NULL) return wrapStructure(TRUE);
    else return wrapStructure(FALSE);
}

/****************************************************************
 Helper function for recurse_eval(Cell*) that inserts the first
 List as the first element of the second List.
*/
static List* cons(List* la, List* lb)
{
    Cell* host = iniCell();

    // Check if list is #f (the empty list convention)
    Cell* shell = lb->mStructure;
    if ((shell->mSub != NULL) && (shell->mSub->mSymbol != NULL)
        && (strcmp(shell->mSub->mSymbol, "#f") == 0)) {
        host->mSub = iniCell();
        host->mSub->mSub = la->mStructure;
    } else {
        // Normal case consing
        host->mNext = shell;
        host->mSub = la->mStructure;
    }
    return wrapStructure(host);
}

/****************************************************************
 Helper function for recurse_eval(Cell*) that checks whether or
 not the given List resolves to null ("#f").
*/
static List* isNull(List* list)
{
    Cell* cell = list->mStructure;

    // Evaluated #t
    if (cell == TRUE || (cell->mSymbol != NULL && strcmp(cell->mSymbol, "#t") == 0)) return wrapStructure(FALSE);
        // Explicit #f encountered
    else if (cell == FALSE || (cell != NULL && cell->mSymbol != NULL
             && (strcmp(cell->mSymbol, "#f") == 0)))
        return wrapStructure(TRUE);
        // Normal case list
    else if (cell != NULL) {
        if (isEmptyStructure(cell) == 1) return wrapStructure(TRUE);
        else return wrapStructure(FALSE);
        // In case of a NULL cell structure in List
    } else return wrapStructure(TRUE);

}

/****************************************************************
 Helper function for isNull(List*) to recursively search a
 cell structure if it's empty or not (void of symbols).
*/
static int isEmptyStructure(Cell* cell)
{
    // Ignore special symbols
    if (cell->mSymbol != NULL
        && strcmp(cell->mSymbol, "quote") != 0
        && strcmp(cell->mSymbol, "()") != 0
        && strcmp(cell->mSymbol, "#f") != 0
        && strcmp(cell->mSymbol, "#t") != 0) return 0;
    int emptyBranch = 1;
    // Search down
    if (cell->mSub != NULL) emptyBranch = isEmptyStructure(cell->mSub);
    // Search on same structural level
    if (emptyBranch == 1 && cell->mNext != NULL) emptyBranch = isEmptyStructure(cell->mNext);
    return emptyBranch;
}

/****************************************************************
 Helper function for recurse_eval(Cell*) that checks a given List
 for a name value pair that matches the given identifying name.
 The pair is returned - both name and value. This function should
 never return a List* whose mStructure is NULL.
*/
static List* assoc(Cell* symbolParent, List* assocList)
{
    // Assoc list is empty so end early
    if (assocList->mStructure == NULL) return wrapStructure(symbolParent);

    // Search the immediate give Cell if it's lone (missing a sub branch)
    Cell* found = NULL;
    if (symbolParent->mSub == NULL) found = findAssoc(symbolParent, assocList->mStructure);
    else found = findAssoc(symbolParent->mSub, assocList->mStructure);

    // Return any match
    if (found != NULL) {
        return wrapStructure(found);
        // Return #f, synonymous to the empty list ()
    } else {
        Cell* empty = iniCell();
        empty->mSymbol = malloc(sizeof(char) * 20);
        strcpy(empty->mSymbol, "#f");
        return wrapStructure(empty);
    }
}

/****************************************************************
 Helper function for assoc(Cell*, List*) that recursively
 scans the association list in assoc(Cell*, List*) for a match.
*/
static Cell* findAssoc(Cell* symbol, Cell* pair)
{
    // Bail out early if NULL (in the case that the cell itself contains #f or empty list)
    if (pair->mSub == NULL) {
        return NULL;
    }

    // Find the lowest Cell* with a symbol
    Cell* focus = pair;
    while (focus->mSub != NULL)
        focus = focus->mSub;

    if (focus != pair && strcmp(symbol->mSymbol, focus->mSymbol) == 0) {
        return pair->mSub;
    } else if (pair->mNext != NULL) {
        return findAssoc(symbol, pair->mNext);
    } else return NULL;
}

/****************************************************************
 Helper function for recurse_eval(Cell*) that checks whether or
 not two Lists are equal in content. This function represents
 the keyword "equal?".
*/
static List* isEqual(List* la, List* lb)
{
    // Recursively compare the structure
    if (la->mStructure != NULL && lb->mStructure != NULL)
        return wrapStructure(compareEqual(la->mStructure, lb->mStructure));
        // NULL is equal to NULL
    else if (la->mStructure == NULL && lb->mStructure == NULL)
        return wrapStructure(TRUE);
    else // Not equal all other cases
        return wrapStructure(FALSE);
}

/****************************************************************
 Helper function for isEqual(List*, List*) to recursively
 compare the two Lists for equality.
*/
static Cell* compareEqual(Cell* c1, Cell* c2)
{
    // Compare symbol
    if (c1->mSymbol != NULL && c2->mSymbol != NULL) {
        // Check if symbols are the same
        if (strcmp(c1->mSymbol, c2->mSymbol) != 0)
            return FALSE;
    } else if (((c1->mSymbol == NULL) && (c2->mSymbol != NULL))
               || ((c1->mSymbol != NULL) && (c2->mSymbol == NULL))) {
        return FALSE;
    }

    // Recurse compare down
    Cell* equals = TRUE;
    if ((c1->mSub != NULL) && (c2->mSub != NULL))
        equals = compareEqual(c1->mSub, c2->mSub);
    // Branch existence does not match
    else if (((c1->mSub == NULL) && (c2->mSub != NULL))
            || ((c1->mSub != NULL) && (c2->mSub == NULL))) {
        return FALSE;
    }

    // Bail out early if found reason to not match
    if (equals == FALSE) return FALSE;

    // Recurse compare on same level
    if ((c1->mNext != NULL) && (c2->mNext != NULL))
        equals = compareEqual(c1->mNext, c2->mNext);
    else if (((c1->mNext != NULL) && (c2->mNext == NULL))
             || ((c1->mNext == NULL) && (c2->mNext != NULL)))
        return FALSE;
    // Assume branch is matched in every way
    return equals;
}

/****************************************************************
 Helper function for recurse_eval(Cell*) that appends two Lists
 into one. The second List is tacked onto the end of the first.
*/
static List* append(List* la, List* lb)
{
    Cell* head = appendSubstitute(la->mStructure, lb);
    return wrapStructure(head);
}

/****************************************************************
 Helper function for append(List*, List*) that joins the two
 Lists non-destructively. That is, the original cons cell
 structure remains with their pointers pointing to the original
 individual List structures.
*/
static Cell* appendSubstitute(Cell* cell, List* secondList)
{
    // Create substitute cell
    Cell* surrogate = iniCell();
    surrogate->mSub = cell->mSub;

    // Build the list along the same level
    if (cell->mNext != NULL) {
        surrogate->mNext = appendSubstitute(cell->mNext, secondList);
    } else {
        // Tack on the second list
        surrogate->mNext = secondList->mStructure;
    }

    return surrogate;
}

/****************************************************************
 Helper function for recurse_eval(Cell*) that evaluates a
 condition and returns an expression if the condition evaluates
 to TRUE. This function supports the keyword "else".
*/
static List* cond(Cell* cell, List* environment)
{
    Cell* pairParent = cell->mNext;
    while (pairParent != NULL) {

        // Check for else keyword if should directly evaluate
        if ((pairParent->mSub != NULL) && (pairParent->mSub->mSub != NULL)
            && (pairParent->mSub->mSub->mSymbol != NULL)
            && ((strcmp(pairParent->mSub->mSub->mSymbol, "else") == 0)
                || (strcmp(pairParent->mSub->mSub->mSymbol, "#t") == 0)))
            return recurse_eval(pairParent->mSub->mNext->mSub, environment);
        // Resolve condition
        List* resolution = recurse_eval(pairParent->mSub->mSub, environment);
        // Evaluate expression if condition is true
        if (resolution->mStructure == TRUE)
            return recurse_eval(pairParent->mSub->mNext->mSub, environment);
        // Focus on next predicate expression pair
        pairParent = pairParent->mNext;
    }
    // Return an empty list (equivalent to NULL and FALSE)
    return wrapStructure(FALSE);
}

/****************************************************************
 Helper function for recurse_eval(Cell*) that evaluates a
 condition and returns an expression if the condition evaluates
 to TRUE, and another expression if FALSE.
*/
static List* alternateIf(Cell* cell, List* environment)
{
    Cell* condition = cell->mNext->mSub;
    List* resolution = recurse_eval(condition, environment);
    if (resolution->mStructure == TRUE)
        return recurse_eval(cell->mNext->mNext->mSub, environment);
    else
        return recurse_eval(cell->mNext->mNext->mNext->mSub, environment);
}

/****************************************************************
 Helper function for recurse_eval(Cell*) that binds the given
 value to the given name. Currently, this function only binds
 values to names once per runtime of the program. That is,
 binding to name "dog" a second time will not change the value
 until the program is restarted.
*/
static List* define(List* symbol, List* value, List* environment)
{

    // Bury the values one level deep
    Cell* emptyList = iniCell();
    emptyList->mSymbol = malloc(sizeof(char) * 20);
    strcpy(emptyList->mSymbol, "#f");
    List* droppedLevel = cons(value, wrapStructure(emptyList));

    // Insert the symbol into the pair
    List* pair = cons(symbol, droppedLevel);

    // Add to variable association list
    environment = cons(pair, environment);
    return environment;
}

/****************************************************************
 Helper function for recurse_eval(Cell*) that binds the given
 name and formal parameters to the given expression. Currently,
 this function only binds values to names once per runtime of
 the program. That is, binding to function name "add" a second
 time will not change the function definition until the program
 is restarted.
*/
static List* defineFunction(List* nameParams, List* expression)
{
    // Bury the values one level deep
    Cell* emptyList = iniCell();
    emptyList->mSymbol = malloc(sizeof(char) * 20);
    strcpy(emptyList->mSymbol, "#f");
    List* droppedLevel = cons(expression, wrapStructure(emptyList));

    // Insert the symbol into the pair
    List* pair = cons(nameParams, droppedLevel);

    // Add to variable association list
    mAssocFns = cons(pair, mAssocFns);

    // Return nothing
    return NULL;
}

/****************************************************************
 Helper function for the shorthand support of calling cdr(List*)
 within car(List*).
*/
static List* cadr(List* list)
{
    return car(cdr(list));
}

/****************************************************************
 Helper function for the shorthand support of calling
 car(cdr(cdr(List*)))).
*/
static List* caddr(List* list)
{
    return car(cdr(cdr(list)));
}

/****************************************************************
 Helper function for the shorthand support of calling
 car(cdr(cdr(cdr(List*)))).
*/
static List* cadddr(List* list)
{
    return car(cdr(cdr(cdr(list))));
}

/****************************************************************
 Helper function for the shorthand support of calling
 car(cdr(cdr(cdr(cdr(List*))))).
*/
static List* caddddr(List* list)
{
    return car(cdr(cdr(cdr(cdr(list)))));
}


/****************************************************************
 Helper function for the shorthand support of calling car(List*)
 within cdr(List*).
*/
static List* cdar(List* list)
{
    return cdr(car(list));
}

/****************************************************************
 Helper function for recurse_eval(Cell*) that wraps the given
 parameters into a list.
*/
static List* makeList(Cell* cell, List* environment)
{
    Cell* parent = cell->mNext;
    List* list = malloc(sizeof(List));
    list->mStructure = iniCell();
    Cell* tail = list->mStructure;
    while (parent != NULL) {
        // Evaluate member to attach
        List* toJoin = recurse_eval(parent->mSub, environment);

        // Attach evaluation to list and move to next member
        tail->mSub = toJoin->mStructure;
        parent = parent->mNext;

        // Prep next attachment
        if (parent != NULL) {
            tail->mNext = iniCell();
            tail = tail->mNext;
        }
    }
    return list;
}

/****************************************************************
 Helper function for recurse_eval(Cell*) that gets the last
 member of a list.
*/
static List* last(List* list)
{
    Cell* focus = list->mStructure;
    while (focus != NULL) {
        if (focus->mNext != NULL) focus = focus->mNext;
        else break;
    }
    return wrapStructure(focus->mSub);
}

/****************************************************************
 Helper function for recurse_eval(Cell*) that counts the number
 of members in the given list.
*/
static List* length(List* list)
{
    int count = 0;

    // Count the length if the structure isn't empty
    if (isEmptyStructure(list->mStructure) != 1) {
        Cell* focus = list->mStructure;
        while (focus != NULL) {
            count++;
            focus = focus->mNext;
        }
    }
    // Generate List and convert number to string
    List* countList = malloc(sizeof(List));
    countList->mStructure = iniCell();
    countList->mStructure->mSymbol = malloc(sizeof(char) * 20);
    sprintf(countList->mStructure->mSymbol, "%i", count);
    return countList;
}

/****************************************************************
 Helper function for recurse_eval(Cell*) that adds any number
 of numerical atoms.
*/
static List* add(Cell* cell, List* environment)
{
    Cell* parent = cell->mNext;
    int sum = 0;
    while (parent != NULL) {
        List* member = recurse_eval(parent->mSub, environment);
        sum += atoi(member->mStructure->mSymbol);

        parent = parent->mNext;
    }

    Cell* num = iniCell();
    char* numSym = malloc(sizeof(char) * 20);
    sprintf(numSym, "%i", sum);
    num->mSymbol = numSym;

    return wrapStructure(num);
}

/****************************************************************
 Helper function for recurse_eval(Cell*) that subtracts any
 number of numerical atoms.
*/
static List* subtract(Cell* cell, List* environment)
{
    Cell* parent = cell->mNext;
    List* firstNum = recurse_eval(parent->mSub, environment);
    int difference = atoi(firstNum->mStructure->mSymbol);
    parent = parent->mNext;

    // Begin subtracting all other numbers
    while (parent != NULL) {
        List* member = recurse_eval(parent->mSub, environment);
        difference -= atoi(member->mStructure->mSymbol);

        parent = parent->mNext;
    }

    Cell* num = iniCell();
    char* numSym = malloc(sizeof(char) * 20);
    sprintf(numSym, "%i", difference);
    num->mSymbol = numSym;

    return wrapStructure(num);
}

/****************************************************************
 Helper function for recurse_eval(Cell*) that multiplies any
 number of numerical atoms.
*/
static List* multiply(Cell* cell, List* environment)
{
    Cell* parent = cell->mNext;
    int product = 1;
    while (parent != NULL) {
        List* member = recurse_eval(parent->mSub, environment);
        product *= atoi(member->mStructure->mSymbol);

        parent = parent->mNext;
    }

    Cell* num = iniCell();
    char* numSym = malloc(sizeof(char) * 20);
    sprintf(numSym, "%i", product);
    num->mSymbol = numSym;

    return wrapStructure(num);
}

/****************************************************************
 Helper function for recurse_eval(Cell*) that evaluates to TRUE
 if all given parameters also evaluate to TRUE. Otherwise, this
 function returns FALSE.
*/
static List* logicAnd(Cell* cell, List* environment)
{
    Cell* parent = cell->mNext;

    while (parent != NULL) {
        List* resolution = recurse_eval(parent->mSub, environment);
        if (resolution->mStructure == FALSE) return wrapStructure(FALSE);
        parent = parent->mNext;
    }
    return wrapStructure(TRUE);
}

/****************************************************************
 Helper function for recurse_eval(Cell*) that evaluates to TRUE
 if at least one of its given parameters evaluate to TRUE. If
 none evaluate to TRUE, this function returns FALSE.
*/
static List* logicOr(Cell* cell, List* environment)
{
    Cell* parent = cell->mNext;

    while (parent != NULL) {
        List* resolution = recurse_eval(parent->mSub, environment);
        if (resolution->mStructure == TRUE) return wrapStructure(TRUE);
        parent = parent->mNext;
    }
    return wrapStructure(FALSE);
}

/****************************************************************
 Helper function for recurse_eval(Cell*) that inverts the
 evaluated value of its given parameter. If its parameter's
 evaluation returns TRUE, this function returns FALSE. If the
 parameter evaluates to FALSE, this function returns TRUE.
*/
static List* logicNot(List* list)
{
    if (list->mStructure == TRUE) return wrapStructure(FALSE);
    else return wrapStructure(TRUE);
}

/****************************************************************
 Helper function for recurse_eval(Cell*) that checks whether or
 not a numerical atom is less than another numerical atom.
*/
static List* lessThan(List* la, List* lb)
{
    int num1 = atoi(la->mStructure->mSymbol);
    int num2 = atoi(lb->mStructure->mSymbol);
    if (num1 < num2) return wrapStructure(TRUE);
    else return wrapStructure(FALSE);
}

/****************************************************************
 Helper function for recurse_eval(Cell*) that checks whether or
 not a numerical atom is greater than another numerical atom.
*/
static List* greaterThan(List* la, List* lb)
{
    int num1 = atoi(la->mStructure->mSymbol);
    int num2 = atoi(lb->mStructure->mSymbol);
    if (num1 > num2) return wrapStructure(TRUE);
    else return wrapStructure(FALSE);
}

/****************************************************************
 Helper function for recurse_eval(Cell*) that checks whether or
 not a numerical atom is less than or equal to another numerical
 atom.
*/
static List* lessThanOrEqualTo(List* la, List* lb)
{
    int num1 = atoi(la->mStructure->mSymbol);
    int num2 = atoi(lb->mStructure->mSymbol);
    if (num1 <= num2) return wrapStructure(TRUE);
    else return wrapStructure(FALSE);
}

/****************************************************************
 Helper function for recurse_eval(Cell*) that checks whether or
 not a numerical atom is greater than or equal to another numerical
 atom.
*/
static List* greaterThanOrEqualTo(List* la, List* lb)
{
    int num1 = atoi(la->mStructure->mSymbol);
    int num2 = atoi(lb->mStructure->mSymbol);
    if (num1 >= num2) return wrapStructure(TRUE);
    else return wrapStructure(FALSE);
}

/****************************************************************
 Helper function for recurse_eval(Cell*) that checks whether or
 not a given List* actually refers to a list structure or just a
 standalone atom.
*/
static List* isList(List* list)
{
    if (list->mStructure == NULL || list->mStructure->mSub == NULL) return wrapStructure(FALSE);
    else return wrapStructure(TRUE);
}

/****************************************************************
 Helper function for recurse_eval(Cell*) that checks whether or
 not a given List* refers to a numerical atom.
*/
static List* isNumber(List* list)
{
    Cell* cell = list->mStructure;
    if (cell->mSub != NULL) cell = cell->mSub;

    // Cell isn't a number if a single character is not a digit
    char* symbol = cell->mSymbol;
    char ch;
    int len = strlen(symbol);
    int i;
    for (i = 0; i < len; i++) {
        ch = symbol[i];

        // Not a number the moment a non-digit char is found
        // (with the exception of '-' at the front) for negatives
        if ((ch != '0') && (ch != '1') && (ch != '2')
            && (ch != '3') && (ch != '4') && (ch != '5')
            && (ch != '6') && (ch != '7') && (ch != '8')
            && (ch != '9') && ((i != 0) || (ch != '-')))
            return wrapStructure(FALSE);
    }
    return wrapStructure(TRUE);
}

/****************************************************************
 Helper for wrapping a Cell* into a List structure. This function
 returns a pointer to the wrapping List and not the List itself.
 Note the List is dynamically allocated and the given Cell becomes
 the mStructure member of the List.
*/
static List* wrapStructure(Cell* cell)
{
    List* list = malloc(sizeof(List));
    list->mStructure = cell;
    return list;
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

/****************************************************************
 Helper function dynamically allocating a new association list.
*/
static List* iniAssocList()
{
    Cell* empty = iniCell();
    empty->mSymbol = malloc(sizeof(char) * 20);
    strcpy(empty->mSymbol, "#f");
    return wrapStructure(empty);
}
