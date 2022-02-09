/**
 * @file gui.c
 * 
 * File containing implementation of the functions used to interface sthe #GUI 
 */

#include <locale.h>
#include <pthread.h>
#include <ncurses.h>
#include <stdbool.h>
#include <unistd.h>

#include "gui/gui.h"
#include "gui/page.h"
#include "gui/pages/queryMenu.h"
#include "gui/pages/queryPage.h"
#include "gui/pages/splashScreen.h"
#include "gui/pages/statisticsPage.h"
#include "gui/pages/catalogPage.h"
#include "gui/pages/catalogMenu.h"
#include "gui/pages/mainMenu.h"
#include "io/taskManager.h"
#include "types/queries.h"
#include "types/catalog.h"
#include "utils/utils.h"



/**
 * @brief The object used to describe the state of the GUI
 */
struct gui {
    Page page;          ///< The page the application is displaying
    void* state;        ///< The state of the page
    Catalog catalog;    ///< The catalog
};


/**
 * @brief       Changes the current #Page being displayed in the #GUI
 * 
 * @param gui   The #GUI
 * @param page  The new #Page
 * @param args  The arguments to the default state
 */
void changePage(GUI gui, Page page, void* args[]) {
    void* (*defaultState)(void*[]) = getDefaultStateFunction(page);
    if(gui->state) {
        void (*freeState)(void*) = getFreeStateFunction(gui->page);
        freeState(gui->state);
    }
        
    if(gui->page)
        freePage(gui->page);
    gui->state = defaultState(args);
    gui->page = page;
}

/**
 * @brief           Executes the statistical queries
 * 
 * @param catalog   The given #Catalog
 */
void executeStatistics(Catalog catalog) {
    for(int i = 1; i <= STATISTICS_COUNT; i++) {
        char* filename = getStatisticFileName(i);
        Query query = createQueryId(i);
        FILE* file = OPEN_FILE(filename, "w+");

        executeQuery(file, query, catalog);

        fclose(file);
        freeQuery(query);
        free(filename);
    }
}


/**
 * @brief       Wrapper to the @ref loadCatalogs function. Used to change the variable which determines if 
 *              the splash screen needs to be switched or not
 * 
 * @param args  The arguments: 0 -> catalog, 1-> pointer to variable finished
 */
void loadCatalogsWrapper(void* args[]) {
    *(Catalog*)args[0] = loadCatalog();
    if (*(Catalog*)args[0] == NULL)
        *(Catalog*)args[0] = newCatalog(USERS_IN,COMMITS_IN,REPOS_IN, true);

    executeStatistics(*(Catalog*)args[0]);  //TODO: is this needed? catalog has the answers. ask vasques
    *(bool*)args[1] = true;
}


/**
 * @brief   Loads the default #GUI. Executed on startup
 *
 * @return  The default #GUI 
 */
GUI loadGUI() {
    GUI gui = malloc(sizeof(struct gui));
    setlocale(LC_ALL, "");
    initscr();	
    noecho();
    start_color();
    cbreak();
    nodelay(stdscr, TRUE);
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);
    keypad(stdscr, TRUE);

    bool finished = false;
    pthread_t loadThread;
    pthread_create(&loadThread, NULL, sequence,
            SEQ(FUNC(loadCatalogsWrapper, &gui->catalog, &finished)));
    pthread_detach(loadThread);

    gui->state = NULL;
    gui->page = NULL;
    changePage(gui, splashScreen(), NULL);
    
    while(!finished) {
        renderPage(gui->page, gui->state);
        processKey(gui->page, gui->state, '-');
        fflush(stdout);
        //Lock to 60fps
        nanosleep((const struct timespec[]){{0, 16700000L}}, NULL);
    }

    changePage(gui, mainMenu(), NULL);

    return gui;
}

/**
 * @brief       Frees the given #GUI
 * 
 * @param gui   The given #GUI
 */
void freeGUI(GUI gui) {
    endwin();	
    freeCatalog(gui->catalog);
    void (*freeState)(void*) = getFreeStateFunction(gui->page);
    freeState(gui->state);
    freePage(gui->page);
    free(gui);
}

/**
 * @brief Executes a #Query
 * 
 * @param args [0] -> catalog [1] -> query to execute
 */
void executeQueryGUI(void* args[]) {
    char* filename = getQueryFileName();

    Catalog catalog = (Catalog)args[0];
    Query query = (Query)args[1];

    FILE* file = OPEN_FILE(filename, "w+");      
    executeQuery(file, query, catalog);
    fclose(file);
    free(filename);
    freeQuery(query);
}

/**
 * @brief       Processes the key input
 * 
 * @param gui   The #GUI
 * @param ch    The character pressed
 * 
 * @returns     Whether or not the application continues running
 */
bool processKeyInput(GUI gui, int ch) {
    Query q = processKey(gui->page, gui->state, ch);
    
    bool b = true;

    if(q == NULL)
        return true;

    int id = getQueryId(q);
    if(q) {
        if(id < 0) {
            long long qid = LOADQUERYID(id);
            void* args[1] = {&qid};
            if(qid >= 5 && qid <= 10) {              
                changePage(gui, queryPage(qid), args);
            } else if(qid >= 1 && qid <= 4) {
                changePage(gui ,statisticsPage(), NULL);
            } else {
                qid = LOADCATALOGID(id);

                if(qid >= 0 && qid <= 2) {
                    changePage(gui, catalogPage(qid), args);
                } else {
                    switch(id) {
                        case KILLAPP:
                            b = false;
                            break;
                        case LOADQUERYMENU:
                            changePage(gui, queryMenu(), NULL);
                            break;
                        case LOADMAINMENU:
                            changePage(gui, mainMenu(), NULL);
                            break;
                        case LOADCATALOGMENU:
                            changePage(gui, catalogMenu(), NULL);
                            break;
                        default:
                            break;
                    }
                }
            }
            freeQuery(q);
        } else  {
            //Execute query
            pthread_t queryThread;
            pthread_create(&queryThread, NULL, sequence, SEQ(FUNC(executeQueryGUI, gui->catalog, q)));
            pthread_detach(queryThread);
            //Give thread enough time to copy data
            nanosleep((const struct timespec[]){{0, 1000000L}}, NULL);
        }
    }

    return b;
}

/**
 * @brief       Main application loop
 * 
 * @param gui   The given #GUI
 */
void runGUI(GUI gui) {
    bool running = true;

    while(running) {
        renderPage(gui->page, gui->state);
        int ch = getch();
        running = processKeyInput(gui, ch);
        //Lock to 60fps
        nanosleep((const struct timespec[]){{0, 16666666L}}, NULL);
    }
    char* filename = getQueryFileName();
    remove(filename);
    filename[0] = 'q';
    remove(filename);
    free(filename);
}
