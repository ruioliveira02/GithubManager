/**
 * @file button.c
 * 
 * File containing the implementation of the #Button component
 */

/**
 * @brief Tell the compiler we are using the wide version of ncurses
 */
#define _XOPEN_SOURCE_EXTENDED

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <wchar.h>

#include "gui/components/button.h"
#include "utils/table.h"

/**
 * @brief Object which represents a #GUI button
 */
struct button {
    char* text;     ///< The text of the button
    bool selected;  ///< Whether or not the button is selected
};


/**
 * @brief           Creates a new #Button
 * 
 * @param text      The text to appear in the #Button
 * @param selected  Whether or not the #Button is selected
 * 
 * @return          The requested #Button
 */
Button createButton(char* text, bool selected) {
    Button button = malloc(sizeof(struct button));
    button->text = strdup(text);
    button->selected = selected;
    return button;
}

/**
 * @brief           Sets the selected state of the #Button
 * 
 * @param button    The given #Button
 * @param selected  Whether the #Button is selected or not
 */
void setButtonSelected(Button button, bool selected) {
    button->selected = selected;
}

/**
 * @brief   Frees the given #Button
 * 
 * @param b The #Button to free
 */
void freeButton(Button b) {
    free(b->text);
    free(b);
}

/**
 * @brief   Creates a deep copy of a #Button
 * 
 * @param b The #Button to copy
 * 
 * @return  The deep copy of the #Button
 */
Button copyButton(Button b) {
    return createButton(b->text, b->selected);
}

/**
 * @brief           Renders the given #Button
 * 
 * @param b         The #Button to render
 * @param x         The original x coordinate
 * @param y         The original y coordinate
 * @param width     The width of the #Button
 * @param height    The height of the #Button
 */
void renderButton(Button b, int* __restrict__ x, int* __restrict__ y, int width, int height) {
    if(b->selected) {
        attron(COLOR_PAIR(1));
    } 

    wchar_t** terminalText = formatButton(b->text, width, height);

    for(int i = 0; i < height; i++) {

        mvaddwstr(*y, *x, terminalText[i]);
        free(terminalText[i]);
        (*y)++;
    }    
    free(terminalText);
    attron(COLOR_PAIR(2));
}