/**
 * @file visualElement.c
 * 
 * File containing implementation of a Visual Element
 */

#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "gui/components/button.h"
#include "gui/components/text.h"
#include "gui/components/title.h"
#include "gui/components/visualElement.h"
#include "utils/table.h"


/**
 * @brief The union used to store the content to display
 */
typedef union content {
    Text text;      ///< Basic text
    Button button;  ///< Button
    Title title;    ///< Title
} Content;

/**
 * @brief A basic visual element to be rendered
 */
struct visualElement {
    Content content;            ///< The content to render
    ContentType contentType;    ///< The type of the content
};


/**
 * @brief           Sets the selected property for the #Button
 * 
 * @param elem      The given #VisualElement
 * @param selected  Whether or not the #Button is selected
 *
 */
void setVEButtonSelected(VisualElement elem, bool selected) {
    if(elem->contentType == BUTTON) {
        setButtonSelected(elem->content.button, selected);
    }
}

/**
 * @brief           Sets the text of the #VisualElement
 * 
 * @param elem      The given #VisualElement
 * @param text      The given text
 *
 */
void setVEText(VisualElement elem, char* text) {
    if(elem->contentType == TEXT) {
        freeText(elem->content.text);
        elem->content.text = createText(text, 2);
    }
}

/**
 * @brief       Creates a #VisualElement from a wide string
 * 
 * @param text  The given text
 * 
 * @return      The requested #VisualElement
 */
VisualElement createVEWideText(wchar_t* text) {
    VisualElement ve = malloc(sizeof(struct visualElement));
    ve->contentType = TEXT;
    ve->content.text = createTextFromWide(text, 2);
    return ve;
}

/**
 * @brief Creates a new #VisualElement
 * 
 * @param type      The ::ContentType
 * @param content   The content of the #VisualElement
 *
 * @return          The requested #VisualElement
 */
VisualElement createVisualElement(ContentType type, void* content) {
    VisualElement res = malloc(sizeof(struct visualElement));
    res->contentType = type;

    switch(type) {
        case TEXT:
            res->content.text = createText((char*)content, 2);
            break;
        case BUTTON:
            res->content.button = copyButton((Button)content);
            break;
        case TITLE:
            res->content.title = copyTitle((Title)content);
        default:
            break;
    }

    return res;
}

/**
 * @brief       Creates a deep copy of the #VisualElement given corresponding to a #Text
 * 
 * @param elem  The given #VisualElement (must be a #Text)
 * 
 * @return      The copy of the #VisualElement
 */
VisualElement copyVEText(VisualElement elem) {
    Text copy = copyText(elem->content.text);
    VisualElement ve = malloc(sizeof(struct visualElement));
    ve->content.text = copy;
    ve->contentType = TEXT;
    return ve;
}

/**
 * @brief       Creates a deep copy of a #VisualElement
 * 
 * @param elem  The #VisualElement to copy
 * 
 * @return      A copy of the given #VisualElement
 */
VisualElement copyVisualElement(VisualElement elem) {
    switch(elem->contentType) {
        case TEXT:
            return copyVEText(elem);
        case BUTTON:
            return createVisualElement(BUTTON, elem->content.button);
        case TITLE:
            return createVisualElement(TITLE, elem->content.title);
        default:
            return NULL;
    }
}

/**
 * @brief       Renders the #VisualElement to the screen
 * 
 * @param elem  The #VisualElement to print
 * @param x     The x coordinate of the cursor
 * @param y     The y coordinate of the cursor
 */
void renderVisualElement(VisualElement elem, int* x, int* y, int width, int height) {

    switch(elem->contentType) {
        case TEXT:
            printText(elem->content.text, x, y, width);
            break;
        case BUTTON:
            renderButton(elem->content.button, x, y, width, 5/*height*/);
            break;
        case TITLE:
            renderTitle(elem->content.title, x, y, width);
        default:
            break;
    }
    (*y)++;
}


/**
 * @brief       Frees the given #VisualElement
 * 
 * @param  v    The #VisualElement to free
 */
void freeVisualElement(VisualElement v) {
    switch(v->contentType) {
        case TEXT:
            free(v->content.text);
            break;
        case BUTTON:
            freeButton(v->content.button);
            break;
        case TITLE:
            freeTitle(v->content.title);
            break;
        default:
            break;
    }
    free(v);
}