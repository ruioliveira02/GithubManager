/**
 * @file queryMenu.c
 * 
 * File containing implementation of all functions / types used to define the query menu page
 */

#include <ncurses.h>
#include <stdlib.h>

#include "gui/components/button.h"
#include "gui/components/panel.h"
#include "gui/components/visualElement.h"
#include "gui/page.h"
#include "gui/pages/queryMenu.h"



/**
 * @brief The structure used to represent the state of the Query Menu page
 */
typedef struct qmState {
    int selectedQuery;  ///< The currently selected query
} *QueryMenuState;


/**
 * @brief   Gets the default state of the menu
 * 
 * @return  The default state of the menu
 */
QueryMenuState defaultQmState() {
    QueryMenuState defaultState = malloc(sizeof(struct qmState));

    defaultState->selectedQuery = 1;

    return defaultState;
}


/**
 * @brief       Gets the default state of the menu
 * 
 * @param args  The arguments to the function (NULL)
 * 
 * @return      The default state of the menu
 */
void* defaultQueryMenuState(void* args[]) {
    return defaultQmState();
}

/**
 * @brief       Applys the given state to the given page
 * 
 * @param page  The page in which to appy the state
 * @param state The given state
 */ 
void applyState(Page page, void* state) {
    QueryMenuState st = (QueryMenuState)state;

    for(int i = 0; i <= 9; i++) {
        setPageButtonSelected(page, i / 5, i % 5, 0, i + 1 == st->selectedQuery);
    }
}

/**
 * @brief       Function used to process input coming from this page
 * 
 * @param state The current state
 * @param key   The key pressed
 * 
 * @return      Whether or not the application continues running
 */
Query processInput(void* state, int key) {
    QueryMenuState st = (QueryMenuState)state;
    Query q = NULL;
    switch(key) {
        case KEY_LEFT:
            if(st->selectedQuery - 1 >= 1)
                st->selectedQuery--;
            break;
        case KEY_UP:
            if(st->selectedQuery - 5 >= 1)
                st->selectedQuery -= 5;
            break;
        case KEY_RIGHT:
            if(st->selectedQuery + 1 <= 10)
                st->selectedQuery++;
            break;
        case KEY_DOWN:
            if(st->selectedQuery + 5 <= 10)
                st->selectedQuery += 5;
            break;
        case KEY_ENTER:
        case 10: //Enter
            q = createQueryId(LOADQUERYID(st->selectedQuery));
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
 * @return  The #Page of the query menu
 */
Page queryMenu() {
    char queryText[10][200] = {"1. Bots, organizacoes e utilizadores", "2. Colaboradores por repositorio",
        "3. Repositorios com bots", "4. Commits por utilizador", "5. Utilizadores mais ativos",
        "6. Utilizadores com mais commits de uma linguagem", "7. Repositorios sem commits a partir de data",
        "8. N linguagens mais utilizadas", "9. Utilizadores com mais commits em repositorios de amigos",
        "10. N maiores mensagens de commit por repositorio"};

    Page page = newPage(2,5,NULL, NULL, defaultQueryMenuState, free, processInput, applyState);


    for(int i = 0; i < 10; i++) {
        Button b = createButton(queryText[i], i == 0);
        VisualElement ve = createVisualElement(BUTTON, b);
        Panel p = emptyPanel();
        panelInsert(p, ve);
        setPagePanel(page, p, i / 5, i % 5);
        freeButton(b);
        freeVisualElement(ve);
        freePanel(p);
    }
    
    return page;
}