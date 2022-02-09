/**
 * @file splashScreen.c
 * 
 * File containing the implementation of the splash screen
 * 
 */

#include <stdlib.h>

#include "gui/components/panel.h"
#include "gui/components/title.h"
#include "gui/components/visualElement.h"
#include "gui/page.h"
#include "gui/pages/splashScreen.h"


/**
 * @brief The number of elements in the splash screen
 * 
 */
#define ELEMENT_COUNT 8

/**
 * @brief The state of the splash screen
 * 
 */
typedef struct splash {
    char* text; ///< The text indicating the #Catalog is being loaded
} *Splash;


/**
 * @brief       Function which returns the default state of the splash screen
 * 
 * @param args  The arguments to the page (NULL)
 */
void* defaultSplashScreen(void* args[]) {
    Splash result = malloc(sizeof(struct splash));
    result->text = strdup("A carregar ficheiros [/]");
    return result;
}

/**
 * @brief       Frees the state of the splash screen
 * 
 * @param st    The state
 */
void freeSplashScreen(void* st) {
    Splash splash = (Splash)st;
    free(splash->text);
    free(splash);
}

/**
 * @brief       Processes the input for the splash screen ('q' to exit, '-' to change Character)
 * 
 * @param st    The state of the page
 * @param ch    The character pressed
 * 
 * @return      KILLAPP if q pressed, NULL otherwise
 */
Query processSplashInput(void* st, int ch) {
    Splash state = (Splash)st;
    char list[4] = "/-\\|";

    int index = 0;
    for(int i = 0; i < 4; i++)
        if(state->text[22] == list[i])
            index = i;

    switch(ch) {
        case 'q':
            return createQueryId(KILLAPP);
        case '-':
            state->text[22] = list[(index + 1) % 4];
            return NULL;
        default:
            return NULL;
    }
}

/**
 * @brief       Applies the state to the #Page
 * 
 * @param p     The given #Page
 * @param st    The given state
 */
void applySplashState(Page p, void* st) {
    Splash state = (Splash)st;
    setPageText(p,0,0,ELEMENT_COUNT - 1, state->text);
}

/**
 * @brief   Function which defines the splash screen
 * 
 * @returns The splash screen 
 */
Page splashScreen() {
    
    Panel panel = emptyPanel();
    Title title = createTitle("grupo 60", SIZE_BIG);
    Title subtitle = createTitle("li3 21-22", SIZE_SMALL);
    VisualElement titleVE = createVisualElement(TITLE, title);
    VisualElement subtitleVE = createVisualElement(TITLE, subtitle);
    VisualElement loadingVE = createVisualElement(TEXT, "A carregar ficheiros [/]");
    VisualElement lineVE = createVisualElement(TEXT, "");

    panelInsert(panel, lineVE);
    panelInsert(panel, lineVE);
    panelInsert(panel, titleVE);
    panelInsert(panel, lineVE);
    panelInsert(panel, lineVE);
    panelInsert(panel, subtitleVE);
    panelInsert(panel, lineVE);
    panelInsert(panel, loadingVE);

    freeVisualElement(titleVE);
    freeVisualElement(subtitleVE);
    freeVisualElement(loadingVE);
    freeVisualElement(lineVE);

    freeTitle(title);
    freeTitle(subtitle);


    Page page = newPage(1, 1, NULL, NULL, defaultSplashScreen, freeSplashScreen, processSplashInput, applySplashState);
    setPagePanel(page, panel, 0, 0);
    freePanel(panel);

    return page;
}