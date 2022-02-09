/**
 * @file mainMenu.c
 * 
 * File containing implementation of all functions / types used to define the catalog menu page
 */

#include "gui/page.h"
#include "gui/pages/catalogMenu.h"
#include "gui/components/visualElement.h"
#include "gui/components/button.h"
#include "gui/components/panel.h"
#include "gui/components/title.h"

#include <stdlib.h>
#include <ncurses.h>

/**
 * @brief The structure used to represent the state of the Catalog Menu page
 */
typedef struct mmState {
    int selectedOption; ///< The currently selected catalog
} *MMState;


/**
 * @brief   Gets the default state of the menu
 * 
 * @return  The default state of the menu
 */
MMState defaultMmState() {
    MMState defaultState = malloc(sizeof(struct mmState));

    defaultState->selectedOption = 0;

    return defaultState;
}

/**
 * @brief   Gets the default state of the menu
 * 
 * @return  The default state of the menu
 */
void* defaultMainMenuState(void* args[]) {
    return defaultMmState();
}


/**
 * @brief Applys the given state to the given #Page
 * 
 * @param page  The #Page in which to apply the state
 * @param state The given state
 */ 
void applyMmState(Page page, void* state) {
    MMState st = (MMState)state;

    for(int i = 0; i < 2; i++) {
        setPageButtonSelected(page, 1 , (i == 0) ? 0 : 2, 0, i == st->selectedOption);
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
Query processMmInput(void* state, int key) {
    MMState st = (MMState)state;
    Query q = NULL;
    switch(key) {
        case KEY_LEFT:
        case KEY_RIGHT:
            st->selectedOption = (st->selectedOption + 1) % 2;
            break;
        case KEY_ENTER:
        case 10: //Enter
            q = createQueryId((st->selectedOption == 0) ? LOADCATALOGMENU : LOADQUERYMENU);
            break;
        case 127: //ESC
        case 'q':
            q = createQueryId(KILLAPP);
            break;
        default:
            break;
    }

    return q;
}

/**
 * @brief   Function which defines the query menu #Page
 * 
 * @return  The #Page of the query menu
 */
Page mainMenu() {
    char buttonText[2][20] = {"Catalogos", "Queries" };

    Page page = newPage(2,3,NULL, NULL, defaultMainMenuState, free, processMmInput, applyMmState);

    Panel titlePanel = emptyPanel();

    Title title = createTitle("escolha", SIZE_SMALL);
    VisualElement titleVE = createVisualElement(TITLE, title);
    panelInsert(titlePanel, titleVE);
    setPagePanel(page, titlePanel, 0, 1);
    freePanel(titlePanel);
    freeVisualElement(titleVE);
    freeTitle(title);

    for(int i = 0; i < 2; i++) {
        Button b = createButton(buttonText[i], i == 0);
        VisualElement ve = createVisualElement(BUTTON, b);
        Panel p = emptyPanel();
        panelInsert(p, ve);
        setPagePanel(page, p, 1, (i == 0) ? 0 : 2);
        freeButton(b);
        freeVisualElement(ve);
        freePanel(p);
    }

    return page;
}