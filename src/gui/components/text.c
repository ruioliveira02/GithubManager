/**
 * @file text.c
 * 
 * File containing the implementation of the #Text component.
 * 
 */

/**
 * @brief Tell the compiler we are using the wide version of ncurses
 */
#define _XOPEN_SOURCE_EXTENDED



#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "gui/components/text.h"
#include "utils/table.h"

/**
 * @brief The Text component
 * 
 * The text component consists of a wide string and an integer corresponding to the
 * background / foreground combination the string should be rendered in.
 * 
 * The text will be rendered centered to the given cell. It is a wide string because the 
 * component must support all UTF-8 characters, and not just the ASCII standard.
 * 
 * | Number | Foreground | Background |
 * | :---:  |    :---:   |  :---:     |
 * | 1      |    Black   |    White   |
 * | 2      |    White   |    Black   |
 */
struct text {
    wchar_t* string;    ///< The wide string containing the content of the text
    int colour;         ///< The foreground / background code of the colours the text should be printed in
};


/**
 * @brief           Creates the requested #Text
 * 
 *                  This function receives a "narrow" string which will be converted to a wide string
 * 
 * @param str       The given string
 * @param colour    The code of the foreground / background combo of the #Text
 * 
 * @return          The requested #Text
 */
Text createText(char* str, int colour) {
    Text result = malloc(sizeof(struct text));

    result->string = stringToWide(str);
    result->colour = colour;

    return result;
}


/**
 * @brief           Creates the requested #Text from a wide string.
 * 
 * @param str       The given string
 * @param colour    The code of the foreground / background combo of the #Text
 * 
 * @return          The requested #Text
 */
Text createTextFromWide(wchar_t* str, int colour) {
    Text result = malloc(sizeof(struct text));

    result->string = wcsdup(str);
    result->colour = colour;

    return result;
}

/**
 * @brief       Creates a deep copy of the given #Text
 *
 * @param text  The #Text to copy
 * 
 * @returns     The requested copy
 */
Text copyText(Text text) {
    Text copy = malloc(sizeof(struct text));
    copy->colour = text->colour;
    copy->string = wcsdup(text->string);
    return copy;
}

/**
 * @brief           Sets the content of a #Text
 * 
 * @remark          This function receives a "narrow" string which will be converted to a wide string
 * 
 * @param text      The given #Text
 * @param str       The given string
 * 
 */
void setText(Text text, char* str) {
    free(text->string);
    text->string = stringToWide(str);
}

/**
 * @brief       Prints the #Text to the screen
 * 
 * @param text  The #Text object to print
 * @param x     The current x-coordinate of the cursor
 * @param y     The current y-coordinate of the cursor
 * @param witdh The width of the cell to print the #Text in
*/
void printText(Text text, int* x, int* y, int width) {
    wchar_t* centeredText = centerWideString(text->string, width);

    attron(COLOR_PAIR(text->colour));
    mvaddwstr(*y, *x, centeredText);

    //Reset to default color
    attron(COLOR_PAIR(2)); 
    free(centeredText);
}


/**
 * @brief       Frees the memory allocated to a #Text
 * 
 * @param text  The given #Text
 */
void freeText(Text text) {
    free(text->string);
    free(text);
} 