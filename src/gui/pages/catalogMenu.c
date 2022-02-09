/**
 * @file catalogMenu.c
 * 
 * File containing implementation of all functions / types used to define the catalog menu page
 */

#include <ncurses.h>
#include <stdlib.h>

#include "gui/components/button.h"
#include "gui/components/panel.h"
#include "gui/components/visualElement.h"
#include "gui/page.h"
#include "gui/pages/catalogMenu.h"


/**
 * @brief The structure used to represent the state of the Catalog Menu page
 */
typedef struct cmState {
    int selectedCatalog; ///< The currently selected catalog
} *CMState;


/**
 * @brief   Gets the default state of the menu
 * 
 * @return  The default state of the menu
 */
CMState defaultCmState() {
    CMState defaultState = malloc(sizeof(struct cmState));

    defaultState->selectedCatalog = 0;

    return defaultState;
}

/**
 * @brief   Gets the default state of the menu
 * 
 * @param   The arguments to the function (NULL)
 * 
 * @return  The default state of the menu
 */
void* defaultCatalogMenuState(void* args[]) {
    return defaultCmState();
}


/**
 * @brief       Applys the given state to the given page
 * 
 * @param page  The page in which to appy the state
 * @param state The given state
 */ 
void applyCmState(Page page, void* state) {
    CMState st = (CMState)state;

    for(int i = 0; i < 3; i++) {
        setPageButtonSelected(page, 1 , i, 0, i == st->selectedCatalog);
    }
}

/**
 * @brief       Function used to process input coming from this #Page
 * 
 * @param state The current state
 * @param key   The key pressed
 * 
 * @return      Whether or not the application continues running
 */
Query processCmInput(void* state, int key) {
    CMState st = (CMState)state;
    Query q = NULL;
    switch(key) {
        case KEY_LEFT:
            st->selectedCatalog = (st->selectedCatalog + 2) % 3;
            break;
        case KEY_RIGHT:
            st->selectedCatalog = (st->selectedCatalog + 1) % 3;
            break;
        case KEY_ENTER:
        case 10: //Enter
            q = createQueryId(LOADCATALOGID(st->selectedCatalog));
            break;
        case 127: //ESC
        case 'q':
            q = createQueryId(LOADMAINMENU);
            break;
        default:
            break;
    }

    return q;
}

/**
 * @brief   Function which defines the query menu #Page
 * 
 * @return  The #Page object of the query menu
 */
Page catalogMenu() {
    char queryText[3][20] = {"Utilizadores", "Repositorios", "Commits" };

    char titleStr[5][200] = {
        "=== === === === =   === =   = = ===   ===   === === === === =   === === ===",
        "=   =   =   = = =   = = =   = = = =   = =   =   = =  =  = = =   = = =   = =",
        "=== === =   = = =   = = =   === ===   = =   =   ===  =  === =   = = === = =",
        "=     = =   = = =   = = =   = = = =   = =   =   = =  =  = = =   = = = = = =",
        "=== === === === === === === = = = =   ===   === = =  =  = = === === === ==="
    };

    Page page = newPage(2,3,NULL, NULL, defaultCatalogMenuState, free, processCmInput, applyCmState);

    Panel title = emptyPanel();
    for(int i = 0; i < 5; i++) {
        VisualElement elem = createVisualElement(TEXT, titleStr[i]);
        panelInsert(title, elem);
        freeVisualElement(elem);
    }

    setPagePanel(page, title, 0, 1);
    freePanel(title);

    for(int i = 0; i < 3; i++) {
        Button b = createButton(queryText[i], i == 0);
        VisualElement ve = createVisualElement(BUTTON, b);
        Panel p = emptyPanel();
        panelInsert(p, ve);
        setPagePanel(page, p, 1, i);
        freeButton(b);
        freeVisualElement(ve);
        freePanel(p);
    }

    return page;
}