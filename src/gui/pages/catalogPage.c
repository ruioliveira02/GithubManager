/**
 * @file catalogPage.c
 * 
 * File containing implementations of the functions used to interface with the catalog page for the GUI
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
#include "gui/components/title.h"
#include "gui/components/visualElement.h"
#include "gui/page.h"
#include "gui/pages/catalogPage.h"
#include "io/finder.h"
#include "io/taskManager.h"
#include "types/format.h"
#include "utils/table.h"
#include "utils/utils.h"

/**
 * @brief Structure used to represent the state of the Catalog Page
 */
typedef struct ctstate {
    int catalogId;          ///< The id of the catalog: 0-> Users 1 -> Repos 2-> Commits
    int status;             ///< The current status for input: 0 -> no input, 1 -> page input, 2-> search input, 3-> search entered
    int page;               ///< The page of results being displayed
    bool lazyPage;          ///< Whether or not the page needs to be recomputed
    String searchString;    ///< The strings used to search
    String pageString;      ///< The strings used to store the page the user requested
} *CtState;


/**
 * @brief       Returns the default state for a catalog #Page for the given #Catalog
 * 
 * @param   id  The #Catalog the page refers to (0 -> Users, 1 -> Repos, 2 -> Commits)
 * 
 * @return      The default state of the page
 */
CtState defaultCtState(int id) {
    CtState state = malloc(sizeof(struct ctstate));
    state->catalogId = id;
    state->status = 0;
    state->page = 0;
    state->lazyPage = true;
    state->searchString = newString("");
    state->pageString = newString("");
    
    return state;
}

/**
 * @brief       Returns the default state for a catalog #Page for the given #Catalog
 * 
 * @param  args The arguments (the #Catalog the page refers to)
 * 
 * @return      The default state of the #Page
 */
void* defaultCatalogPageState(void* args[]) {
    return defaultCtState(*(int*)args[0]);
}
/**
 * @brief Frees the allocated space for the state object
 * 
 * @param st The state to free (as void*)
 */
void freeCtState(void* st) {
    CtState state = (CtState)st;

    freeString(state->searchString);
    freeString(state->pageString);

    free(state);
}

/**
 * @brief       Applies the value of the pageString property
 * 
 * @param state The state to modify
 */
void applyCatalogPageString(CtState state) {
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
 * @returns    The #Query to send back to the GUI object
 */
Query processCtInput(void* state, int key) {
    CtState st = (CtState)state;
    Query q = NULL;
    
    switch(key) {
        case KEY_LEFT:
            if(st->page >= 1) {
                st->page--;
                st->lazyPage = true;
            }
            break;
        case KEY_RIGHT:
            st->page++;
            st->lazyPage = true;
            break;
        case 10: //Enter
            if(st->status == 1) {
                applyCatalogPageString(st);
            } else if(st->status == 2) {
                st->status = 3;
                st->lazyPage = true;
            }
            break;
        case 27: //Esc
            if(st->status == 0) {
                q = createQueryId(LOADCATALOGMENU);
            } else if(st->status == 3) {
                st->status--;
            } else {
                st->status = 0;
            }
            break;
        case KEY_BACKSPACE:
        case 127: //Backspace
            if(st->status != 2) {
                stringLastRemove(st->pageString);
                applyCatalogPageString(st);
            } else  {
                stringLastRemove(st->searchString);
            }
            break;
        case 's':
            if(st->status != 2) {
                st->status = 2;
                break;
            }
        default:
            if(st->status == 2 && isValidStringInput(key)) {
                stringAppend(st->searchString, (char)key);
            } else if(key >= '0' && key <= '9') {
                stringAppend(st->pageString, (char)key);
                applyCatalogPageString(st);
            }
            break;
    }

    return q;
}


/**
 * @brief   Gets the number of results a #Page can display
 * 
 * @return  The number of results a #Page can display
 */
int getNumberOfCatalogResultsPerPage() {
    int rows, cols;
    getScreenDimensions(&rows, &cols);

    rows = (int)((float)rows * 0.7f);
    rows -= 6;
    rows /= 2;

    return rows;
}

/**
 * @brief       Applys the given state to the given #Page
 * 
 * @param page  The #Page in which to apply the state
 * @param st    The given state as void*
 */ 
void applyCtState(Page page, void* st) {
    int resultsPerPage = getNumberOfCatalogResultsPerPage();

    int elementCount[3] = {10,14,5};

    char firstLines[3][14][200] = {      
        {"Id", "Login", "Tipo", "Data Criacao", "Seguidores", "Lista seguidores", "Seguindo", "Lista seguindo", "Gists publicos", "Repos publicos"},
        {"Id","Id Dono", "Nome", "Licenca", "Wiki", "Descricao", "Linguagem", "Ramo padrao", "Data criacao", "Data modificacao", "Forks", "Issues", "Stargazers", "Tamanho"},
        {"Repo Id","Autor Id","Committer Id","Data Commit","Messagem"}
    };

    char titles[3][20] = { "Utilizadores", "Repositorios", "Commits" };

    char files[3][100] = { USERS_IN, REPOS_IN, COMMITS_IN };
    

    CtState state = (CtState)st;

    if(state->lazyPage) {

        char* filename = (state->status == 3) ? getQueryFileName() : strdup(files[state->catalogId]);
        int size = 0;

        if(state->status == 3) {
            char* search = getStringContent(state->searchString);
            finder(files[state->catalogId], filename, search);
            free(search);
        }

        char** content = getFileContent(filename, state->page * resultsPerPage + 1, (state->page + 1) * resultsPerPage, &size);

        int size2 = size;

        wchar_t** fl = malloc(sizeof(wchar_t*) *  elementCount[state->catalogId]);
        for(int i = 0; i <  elementCount[state->catalogId]; i++)
            fl[i] = stringToWide(firstLines[state->catalogId][i]);

        wchar_t** table = queryOutputToTable(content, &size2, fl, elementCount[state->catalogId]);

        Panel textPanel = emptyPanel();

        VisualElement temp = createVisualElement(TEXT, titles[state->catalogId]);
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

        for(int i = 0; i < elementCount[state->catalogId]; i++)
            free(fl[i]);

        free(fl);

        for(int i = 0; i < size2; i++)
            free(table[i]);

        if(size2)
            free(table);

        free(filename);
    }

    char pageNo[1024];
    if(state->status != 2) {
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

/**
 * @brief           Creates the #Page used to view the #Catalog
 * 
 * @param   query   The number of the #Catalog to call (0 to 2)
 * 
 * @returns         The requested #Page
 */
Page catalogPage(int id) {

    float heights[3] = {0.15f, 0.85f, 1.0f}; 
    float widths[3] = {0.1f, 0.9f, 1.0f};
    Page page = newPage(3, 3, heights, widths, defaultCatalogPageState, freeCtState, processCtInput, applyCtState);

    Panel p = emptyPanel();
    Title title = createTitle("CATALOGO", 5);
    VisualElement elem = createVisualElement(TITLE, title);
    panelInsert(p, elem);
    setPagePanel(page, p, 0, 1);
    freePanel(p);
    freeVisualElement(elem);
    freeTitle(title);


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

    freeVisualElement(text);
    freePanel(textPanel);

    return page;
}