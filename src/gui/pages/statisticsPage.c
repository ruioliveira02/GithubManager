/**
 * @file statisticsPage.c
 * 
 * File containing the implementation of the statistics page
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gui/pages/statisticsPage.h"
#include "gui/page.h"
#include "gui/components/visualElement.h"
#include "gui/components/panel.h"
#include "gui/components/title.h"
#include "types/queries.h"
#include "utils/querySolver.h"


/**
 * @brief The maximum number of lines of a query file minus 1 (i.e., a file can have at most 3 lines)
 */
#define MAX_OUTPUT_LENGTH 2

/**
 * @brief The state for the statistics page
 * 
 */
typedef struct stState {
    char** output[STATISTICS_COUNT];    ///< The content of the statistic queries
    int lines[STATISTICS_COUNT];        ///< The number of lines of each statistic query
} * StState;


/**
 * @brief       Returns the name of the file the #Query will be saved to
 * 
 * @param index The id of #Query
 * 
 * @return      The name of the file
 */
char* getStatisticFileName(int index) {
    char page[100];
    sprintf(page, "saida/statistic%d.txt", index);
    return strdup(page);
}


/**
 * @brief       Frees the space allocated to the state object
 * 
 * @param st    The state of the #Page (as void*)
 */
void freeStState(void* st) {
    StState state = (StState)st;

    for(int i = 0; i < STATISTICS_COUNT; i++) {
        for(int j = 0; j < state->lines[i]; j++) {
            free(state->output[i][j]);
        }
        free(state->output[i]);
    }

    free(state);
}

/**
 * @brief Empty function as the state is applied once at the beggining of the page
 * 
 * @param page  The given #Page
 * @param st    The state of the #Page
 */
void applyStState(Page page, void* st) {

}


/**
 * @brief Returns the default state for the statistics page
 */
StState defaultStState() {
    StState state = malloc(sizeof(struct stState));

    for(int i = 0; i < STATISTICS_COUNT; i++) {
        char* filename = getStatisticFileName(i + 1);

        state->output[i] = getFileContent(filename, 0, MAX_OUTPUT_LENGTH, &state->lines[i]);

        //remove(filename);
        free(filename);
    }

    return state;
}

/**
 * @brief       Returns the default state for the statistics page
 * 
 * @param  args The arguments (NULL in this case)
 */
void* defaultStatisticsPageState(void* args[]) {
    return defaultStState();
}

/**
 * @brief       Processes the input of the statistics page.
 * 
 *              In this page, any key press will result in a return to the query menu
 * 
 * @param st    The state
 * @param key   The key pressed
 * 
 * @return      The query corresponding to returning to the query menu
 */
Query processStInput(void* st, int key) {
    if(key == 27) //ESC
        return createQueryId(LOADQUERYMENU);
    return NULL;
}

/**
 * @brief   Creates the statistics page
 * 
 * @return  The #Page
 */
Page statisticsPage() {

    Page page = newPage(4,3, NULL, NULL, defaultStatisticsPageState, freeStState, processStInput, applyStState);
    StState state = defaultStState();

    char description[4][50] = {
        "User Types:",
        "Average Collaborators per Repository:",
        "Repositories With Bots:",
        "Average Number of Commits per User:"
    };

    Panel panel = emptyPanel();

    Title title = createTitle("statistics", SIZE_SMALL);
    VisualElement titleVE = createVisualElement(TITLE, title);
    panelInsert(panel, titleVE);
    setPagePanel(page, panel, 0, 1);
    freePanel(panel);
    freeVisualElement(titleVE);
    freeTitle(title);

    for(int i = 0; i < STATISTICS_COUNT; i++) {
        panel = emptyPanel();
        char* str = strdup(description[i]);
        VisualElement ve = createVisualElement(TEXT, description[i]);
        free(str);
        panelInsert(panel, ve);
        freeVisualElement(ve);
        ve = createVisualElement(TEXT, "");
        panelInsert(panel, ve);
        freeVisualElement(ve);

        for(int j = 0; j < state->lines[i]; j++) {
            ve = createVisualElement(TEXT, state->output[i][j]);
            panelInsert(panel, ve);
            freeVisualElement(ve);
        }

        setPagePanel(page, panel, 1 + i / 2, (i % 2 == 0 ? 0 : 2));

        freePanel(panel);
    }

    panel = emptyPanel();
    VisualElement ve = createVisualElement(TEXT, "<Press ESC to exit>");
    panelInsert(panel, ve);
    setPagePanel(page, panel, 3, 1);
    freePanel(panel);
    freeStState(state);
    freeVisualElement(ve);

    return page;
}