/**
 * @file page.c
 * 
 * File containing implementation of the #Page structure used to represent
 * a page in the #GUI
 */

#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>


#include "gui/components/panel.h"
#include "gui/page.h"
#include "types/queries.h"
#include "utils/table.h"
#include "utils/utils.h"


/**
 * @brief Inline function used to clear the console
 */
#define clearScreen() printf("\033[H\033[J")


/**
 * @brief The structure used to describe a UI page
 */
struct page {
    Panel* grid;                        ///< The 2D grid of #Panel to display
    int rows;                           ///< The number of rows on the 2D grid
    int columns;                        ///< The number of columns on the 2D grid
    float* rowHeight;                   ///< Accumulated height of the rows (ex: [10,30,100] means 10% of height to 1st, 20% second, and 70% third)
    float* colWidth;                    ///< Accumulated height of the rows (ex: [10,30,100] means 10% of height to 1st, 20% second, and 70% third)
    void* (*state)(void*[]);            ///< The function used to get the default state type of the page
    void (*freeState)(void*);           ///< The function used to free the state type
    Query (*processInput)(void*, int);  ///< The function used to process the input
    void (*applyState)(Page, void*);    ///< The function used to apply the state
};

/**
 * @brief               Sets the #Button selected property on the given position of the #Page
 * 
 * @param page          The given #Page
 * @param row           The given row
 * @param column        The given column
 * @param stackIndex    The index of the #Button in the stack
 * @param selected      Whether or not the #Button is selected
 */
void setPageButtonSelected(Page page, int row, int column, int stackIndex, bool selected) {
    if(row * page->columns + column >= 0 && row * page->columns + column < page->rows * page->columns)
        setPNButtonSelected(page->grid[row * page->columns + column], selected, stackIndex);
}

/**
 * @brief               Sets the #Text on the given position of the #Page
 * 
 * @param page          The given page
 * @param row           The given row
 * @param column        The given column
 * @param stackIndex    The index of the #Text in the stack
 * @param text          Whether or not the #Text is selected
 */
void setPageText(Page page, int row, int column, int stackIndex, char* text) {
    if(row * page->columns + column >= 0 && row * page->columns + column < page->rows * page->columns)
        setPNText(page->grid[row * page->columns + column], stackIndex, text);
}

/**
 * @brief               Creates a new #Page
 * 
 * @param rows          The number of rows in the #Page's grid
 * @param cols          The number of cols in the #Page's grid
 * @param rowHeight     The heights of the rows (NULL for default height)
 * @param colWidth      The widths of the columns (NULL for default width)
 * @param state         The function used to get the default state of the #Page
 * @param freeState     The function used to free the state object
 * @param processInput  The function used to process the keyboard input
 * @param applyState    The function used to apply the state after a key press
 * 
 * @return              The desired #Page
 */
Page newPage(int rows, int cols, float* rowHeight, float* colWidth, void* (*state)(void*[]),void (*freeState)(void*), Query (*processInput)(void*, int),void (*applyState)(Page, void*)) {
    Page page = malloc(sizeof(struct page));
    page->rows = rows;
    page->columns = cols;

    page->rowHeight = malloc(sizeof(float) * rows);
    page->colWidth  = malloc(sizeof(float) * cols);
    
    for(int i = 0; i < rows; i++) {
        page->rowHeight[i] = (rowHeight) ? rowHeight[i] : (1.0f / (float)rows) * (i + 1);
    }

    for(int i = 0; i < cols; i++) {
        page->colWidth[i] = (colWidth) ? colWidth[i] : (1.0f / (float)cols) * (i + 1);
    }

    page->state = state;

    int cells = (rows * cols);
    page->grid = malloc(sizeof(Panel) * cells);

    for(int i = 0; i < cells; i++)
        page->grid[i] = emptyPanel();

    page->freeState = freeState;
    page->processInput = processInput;
    page->applyState = applyState;
    return page;
}

/**
 * @brief           Replaces the #Panel at the given position
 * 
 * @param page      The #Page to modify
 * @param panel     The #Panel to put into the #Page
 * @param row       The row of the #Panel to replace
 * @param column    The column of the #Panel to replace
 */
void setPagePanel(Page page, Panel panel, int row, int column) {
    freePanel(page->grid[row * page->columns + column]);
    page->grid[row * page->columns + column] = copyPanel(panel);
}

/**
 * @brief       Frees the given #Page
 * 
 * @param page  The #Page to free
 */
void freePage(Page page) {

    int cells = page->rows * page->columns;
    for(int i = 0; i < cells; i++) 
        freePanel(page->grid[i]);

    free(page->rowHeight);
    free(page->colWidth);
    free(page->grid);
    
    free(page);
    
}

/**
 * @brief   Returns the function used to get the default state object of a given #Page
 * 
 * @param   The given #Page
 * 
 * @return  The function for the default state of the given #Page
 */
void* (*getDefaultStateFunction(Page page))(void*[]) {
    return page->state;
}

/**
 * @brief   Returns the function used to free the state object of a given #Page
 * 
 * @param   The given #Page
 * 
 * @return  The function to free the state of the given #Page
 */
void (*getFreeStateFunction(Page page))(void*) {
    return page->freeState;
}

/**
 * @brief       Gets the current terminal screen dimensions
 * 
 * @param rows  The pointer to the variable in which to write the number of rows
 * @param cols  The pointer to the variable in which to write the number of columns
 */
void getScreenDimensions(int* rows, int* cols) {
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);

    *rows = w.ws_row;
    *cols = w.ws_col;
}



/**
 * @brief       Displays the given #Page on the terminal window
 * 
 * @param page  The #Page to display
 * @param state The state of the #Page
 */
void renderPage(Page page, void* state) {
    int rows, cols;
    getScreenDimensions(&rows, &cols);
    int x = 1, y = 1;
    float width = 0.0f, height = 0.0f;
    erase();
    page->applyState(page, state);
    for(int i = 0; i < page->rows; i++) {
        x = 1;
        width = 0.0f;
        for(int j = 0; j < page->columns; j++) {
            renderPanel(page->grid[i * page->columns + j], x, y, (page->colWidth[j] - width) * cols, (page->rowHeight[i] - height) * rows);
            x = page->colWidth[j] * (cols);
            width = page->colWidth[j];
        }
        y = rows * page->rowHeight[i] + 1;
        height = page->rowHeight[i];
    }

    refresh();
}

/**
 * @brief       Processes the key just pressed
 * 
 * @param page  The current #Page
 * @param state The state of the #Page
 * @param ch    The key just pressed
 * 
 * @return      The extended #Query to request from the #GUI object
 */
Query processKey(Page page, void* state, int ch) {
    Query q = page->processInput(state, ch);
    return q;
}