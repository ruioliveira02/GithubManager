/**
 * @file queryPage.c
 *
 * File containing implementations of the functions used to interface with the query page for the GUI
 */

#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>


#include "gui/components/button.h"
#include "gui/components/panel.h"
#include "gui/components/visualElement.h"
#include "gui/pages/queryPage.h"
#include "io/finder.h"
#include "io/taskManager.h"
#include "types/format.h"
#include "types/queries.h"
#include "utils/querySolver.h"
#include "utils/table.h"
#include "utils/utils.h"


/**
 * @brief An upper bound for the maximum number of parameters a query can receive
 */
#define MAX_PARAMETERS 10

/**
 * @brief Structure used to represent the state of the Query Page
 */
typedef struct qpstate {
    int query;                              ///< The query the page refers to
    Format queryFormat;                     ///< The format of the query
    String parameterText[MAX_PARAMETERS];   ///< The text to place on top of the parameters' input
    String parameters[MAX_PARAMETERS];      ///< The value of the parameters inputted by the user
    int parameterCount;                     ///< The number of parameters the query receives
    bool parametersValid[MAX_PARAMETERS];   ///< Whether or not the parameters inputted by the user are valid
    int mousePosition;                      ///< The current position for input / state of the page, i.e., which input to edit: -1 for search string input, -2 for display string
    bool queryRun;                          ///< Whether or not the query was already run
    int page;                               ///< The page of results being displayed
    bool lazyPage;                          ///< Whether or not the page needs to be recomputed
    String searchString;                    ///< The string used to search
    String pageString;                      ///< The string used to store the page the user requested
    bool invalidQueryVisible;               ///< Whether or not the text displaying "The input is invalid!" is on or not
} *QpState;



/**
 * @brief       Gets the #Query the user requested from the #Page 's state
 *
 * @param state The given state of the #Page
 *
 * @return      The requested #Query
 */
Query getRequestedQuery(QpState state) {
    char number[5];
    sprintf(number, "%d ", state->query);
    String queryString = newString(number);

    for(int i = 0; i < state->parameterCount; i++) {
        char* str = getStringContent(state->parameters[i]);

        for(int j = 0; str[j]; j++) {
            stringAppend(queryString, str[j]);
        }

        if(i != state->parameterCount - 1) {
            stringAppend(queryString, ' ');
        }

        free(str);
    }

    char* content = getStringContent(queryString);

    Query query = createEmptyQuery();
    parseQuery(content, query);


    state->invalidQueryVisible = getQueryId(query) <= 0;

    free(content);
    freeString(queryString);

    return query;
}


/**
 * @brief       Executes the #Query requested by the user as per the #Page state
 *
 * @param state The state of the #Page
 */
void executeRequestedQuery(QpState state) {
    char* filename = getQueryFileName();

    Query query = getRequestedQuery(state);


    freeQuery(query);
    free(filename);
}

/**
 * @brief       Returns the text to put above the input for the index-th parameter of the query
 *
 * @param query The respective #Query
 * @param index The index of the parameter
 *
 * @return      The text to put above the input
 */
String getParameterText(int query, int index) {
    char parameterText[10][MAX_PARAMETERS][100] = {
        {"","","","","","","","","",""},
        {"","","","","","","","","",""},
        {"","","","","","","","","",""},
        {"","","","","","","","","",""},
        {"Número de utilizadores (N):","Data inicial (YYYY-MM-DD):","Data final (YYYY-MM-DD):","","","","","","",""},
        {"Numero de utilizadores (N):","Linguagem (case insensitive):","","","","","","","",""},
        {"Data (YYYY-MM-DD):","","","","","","","","",""},
        {"Numero de linguagens (N):","Data (YYYY-MM-DD):","","","","","","","",""},
        {"Numero de utilizadores (N):","","","","","","","","",""},
        {"Numero de utilizadores (N):","","","","","","","","",""}
    };

    return newString(parameterText[query - 1][index]);
}

/**
 * @brief           Returns the default state for a query page for the given #Query
 *
 * @param   query   The #Query the #Page refers to
 *
 * @return          The default state of the #Page
 */
QpState defaultQpState(int query) {
    QpState state = malloc(sizeof(struct qpstate));

    state->query = query;
    state->mousePosition = 0;
    state->queryFormat = getQueryFormat(query);
    state->parameterCount = getFormatMembers(state->queryFormat);
    state->queryRun = false;
    state->page = 0;
    state->lazyPage = false;
    state->invalidQueryVisible = false;
    state->searchString = newString("");
    state->pageString = newString("");
    for(int i = 0; i < state->parameterCount; i++) {
        state->parameters[i] = newString("");
        state->parametersValid[i] = false;
        state->parameterText[i] = getParameterText(query, i);
    }

    return state;
}


/**
 * @brief           Returns the default state for a query page for the given #Query
 *
 * @param   args    The args for the state (query id)
 *
 * @returns         The default state of the #Page
 */
void* defaultQueryPageState(void* args[]) {
    return defaultQpState(*(int*)args[0]);
}


/**
 * @brief       Frees the allocated space for the state
 *
 * @param st    The state to free (as void*)
 */
void freeQpState(void* st) {
    QpState state = (QpState)st;
    disposeFormat(state->queryFormat);
    freeString(state->searchString);
    freeString(state->pageString);
    for(int i = 0; i < state->parameterCount; i++) {
        freeString(state->parameterText[i]);
        freeString(state->parameters[i]);
    }

    free(state);
}

/**
 * @brief       Applies the value of the pageString property
 *
 * @param state The state to modify
 */
void applyPageString(QpState state) {
    char* pstr = getStringContent(state->pageString);

    if(strlen(pstr) > 0) {
         int p = atoi(pstr);

        if(p == 0)
            p = 1;

        state->page = --p;
    } else {
        state->page = 0;
    }

    state->lazyPage = true;

    free(pstr);
    clear();
}


/**
 * @brief       Function used to process input coming from this page
 *
 * @param state The current state
 * @param key   The key pressed
 *
 * @return     The #Query to send back to the #GUI
 */
Query processQpInput(void* state, int key) {
    QpState st = (QpState)state;
    Query q = NULL;

    switch(key) {
        case KEY_LEFT:
            if(st->mousePosition >= 1 && !st->queryRun) {
                st->mousePosition--;
            } else if(st->page >= 1) {
                st->page--;
                st->lazyPage = true;
            }
            break;
        case KEY_RIGHT:
            if(st->mousePosition >= 0 && st->mousePosition < st->parameterCount && !st->queryRun) {
                st->mousePosition++;
            } else {
                st->page++;
                st->lazyPage = true;
            }
            break;
        case 10: //Enter
            if(st->mousePosition == st->parameterCount) {
                q = getRequestedQuery(st);
                st->queryRun = !st->invalidQueryVisible;
                st->lazyPage = true;
            } else if(st->mousePosition == - 1) { //User has finished typing the search
                st->mousePosition = -2;
                st->page = 0;
                st->lazyPage = true;
                st->queryRun = true;
            }
            break;
        case 27: //Esc
            if(st->mousePosition >= 0) {
                q = createQueryId(LOADQUERYMENU);
            } else {
                st->mousePosition++;
                st->lazyPage = true;
            }
            break;
        case KEY_BACKSPACE:
        case 127: //Backspace
            if(st->mousePosition >= 0 && st->mousePosition < st->parameterCount) {
                stringLastRemove(st->parameters[st->mousePosition]);
            } else if(st->mousePosition == -1) {
                stringLastRemove(st->searchString);
            } else {
                stringLastRemove(st->pageString);
                applyPageString(st);
            }
            break;
        case 's':
            if(st->queryRun && st->mousePosition != -1) {
                st->mousePosition = -1;
                break;
            }
        default:
            if(st->queryRun) {
                if(st->mousePosition == -1 && isValidStringInput(key)) {
                    stringAppend(st->searchString, (char)key);
                } else if(key >= '0' && key <= '9') {
                    stringAppend(st->pageString, (char)key);
                    applyPageString(st);
                }
            } else {
                if(isValidStringInput(key) && st->mousePosition >= 0 && st->mousePosition < st->parameterCount) {
                    stringAppend(st->parameters[st->mousePosition], (char)key);
                }
            }

            break;
    }

    return q;
}


/**
 * @brief Gets the number of results a page can display
 *
 * @returns The number of results a page can display
 */
int getNumberOfResultsPerPage() {
    int rows, cols;
    getScreenDimensions(&rows, &cols);

    rows = (int)((float)rows * 0.7f);
    rows -= 6;
    rows /= 2;

    return rows;
}

/**
 * @brief Applys the given state to the given page
 *
 * @param page  The page in which to appy the state
 * @param st    The given state as void*
 */
void applyQpState(Page page, void* st) {
    int resultsPerPage = getNumberOfResultsPerPage();
    char firstLines[10][10][500] = {
        {"Bot","Organization","User","","","","","","",""},
        {"Average","","","","","","","","",""},
        {"Average","","","","","","","","",""},
        {"Average","","","","","","","","",""},
        {"Id","Login","Nb. Commits","","","","","","",""},
        {"Id","Login","Nb. Commits","","","","","","",""},
        {"Repo Id","Description","","","","","","","",""},
        {"Language","","","","","","","","",""},
        {"Id","Login","","","","","","","",""},
        {"Id","Login","Length of commit message","Repo Id","","","","","",""}
    };

    int elementCount[10] = {3,1,1,1,3,3,2,1,2,4};


    QpState state = (QpState)st;
    for(int i = 0; i < state->parameterCount; i++) {
        char* str = getStringContent(state->parameters[i]);
        char* topText = getStringContent(state->parameterText[i]);
        setPageText(page, 0, i, 1, str);
        setPageText(page, 0, i, 0, topText);
        free(topText);
        free(str);
    }

    setPageButtonSelected(page, 0,0,(state->parameterCount == 0 ? 0 : 2),state->mousePosition == state->parameterCount && !state->queryRun);


    if(state->mousePosition >= 0 && state->mousePosition < state->parameterCount) {
        char* str = malloc((stringLength(state->parameterText[state->mousePosition]) + 5) * sizeof(char));
        str[0] = '\0';
        char* content = getStringContent(state->parameterText[state->mousePosition]);

        strcat(str, "> ");
        strcat(str, content);
        strcat(str, " <");

        setPageText(page, 0, state->mousePosition, 0, str);

        free(str);
        free(content);
    }

    if(state->queryRun && state->lazyPage) {

        char* filename = getQueryFileName();
        int size = 0;

        if(state->mousePosition == -2) {
            char* queryFileName = strdup(filename);
           	filename[6] = 'q';
            char* search = getStringContent(state->searchString);
            finder(queryFileName, filename, search);
            free(search);
            free(queryFileName);
        }

        char** content = getFileContent(filename, state->page * resultsPerPage, (state->page + 1) * resultsPerPage - 1, &size);

        if(content == NULL || size == 0) {
            state->queryRun = true;
            state->lazyPage = true;
            return;
        }


        int size2 = size;

        wchar_t** fl = malloc(sizeof(wchar_t*) *  elementCount[state->query - 1]);
        for(int i = 0; i <  elementCount[state->query - 1]; i++)
            fl[i] = stringToWide(firstLines[state->query - 1][i]);

        wchar_t** table = queryOutputToTable(content, &size2, fl, elementCount[state->query - 1]);
        Panel textPanel = emptyPanel();

        VisualElement temp = createVisualElement(TEXT, "RESULTADOS");
        panelInsert(textPanel, temp);
        freeVisualElement(temp);
        for(int i = 0; i < size2; i++) {
            VisualElement text = createVEWideText(table[i]);
            panelInsert(textPanel, text);
            freeVisualElement(text);
        }

        state->lazyPage = false;

        setPagePanel(page, textPanel, 1, 1);
        freePanel(textPanel);

        for(int i = 0; i < size; i++)
            free(content[i]);

        free(content);

        for(int i = 0; i < elementCount[state->query - 1]; i++)
            free(fl[i]);

        free(fl);

        for(int i = 0; i < size2; i++)
            free(table[i]);

        if(size2)
            free(table);

        free(filename);
    }

    if(state->queryRun) {
        char pageNo[1024];
        if(state->mousePosition >= 0) {
            sprintf(pageNo, "Pagina %d", state->page + 1);
        } else {
            char* search = getStringContent(state->searchString);
            sprintf(pageNo, "Procura: %s", search);
            free(search);
        }
        setPageText(page, 2, 1, 0, pageNo);
        setPageText(page, 2, 0, 0, "<Pagina Anterior>");
        setPageText(page, 2, 2, 0, "<Pagina Seguinte>");
    }

    setPageText(page, 0, 1, 2, state->invalidQueryVisible ? "O input e invalido!" : "");
}

/**
 * @brief Creates the page used to call the queries
 *
 * @param   query   The number of the query to call (1 to 10)
 *
 * @returns         The requested page
 */
Page queryPage(int query) {

    float heights[3] = {0.15f, 0.85f, 1.0f};

    Page page = newPage(3, 3, heights, NULL, defaultQueryPageState, freeQpState, processQpInput, applyQpState);
    QpState state = defaultQpState(query);
    for(int i = 0; i < MAX(2, state->parameterCount); i++){
        Panel p = emptyPanel();

        //This is needed because of statistical queries with 0 arguments
        if(i < state->parameterCount) {
            char* str = getStringContent(state->parameterText[i]);

            VisualElement textAbove = createVisualElement(TEXT, str);
            VisualElement input     = createVisualElement(TEXT, "");

            free(str);

            panelInsert(p, textAbove);
            panelInsert(p, input);

            freeVisualElement(textAbove);
            freeVisualElement(input);
        }


        if(i == 0) {
            Button button = createButton("Rodar", false);
            VisualElement elem = createVisualElement(BUTTON, button);
            panelInsert(p, elem);
            freeVisualElement(elem);
            freeButton(button);
        } else if(i == 1) {
            VisualElement elem = createVisualElement(TEXT, "");
            panelInsert(p, elem);
            freeVisualElement(elem);
        }

        setPagePanel(page, p, 0, i);
        freePanel(p);
    }

    for(int i = 0; i < 3; i++) {
        Panel panel = emptyPanel();
        VisualElement ve = createVisualElement(TEXT, "");
        panelInsert(panel, ve);
        setPagePanel(page, panel, 2, i);
        freeVisualElement(ve);
        freePanel(panel);
    }

    Panel textPanel = emptyPanel();
    VisualElement text = createVisualElement(TEXT, "");
    panelInsert(textPanel, text);
    panelInsert(textPanel, text);
    setPagePanel(page, textPanel, 1, 1);

    freeQpState(state);
    freeVisualElement(text);
    freePanel(textPanel);

    return page;
}
