/**
 * @file panel.c
 * 
 * File containing implementation of the #Panel type
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "gui/components/panel.h"
#include "gui/components/visualElement.h"
#include "utils/table.h"


/**
 * @brief A panel, i.e, a grid cell
 */
struct panel {
    VisualElement* stack;   ///< A stack of the elements to render: the first is on top, the second below it, ...
    int numberElements;     ///< The number of elements in the stack
    int capacity;           ///< The capacity of the stack
};


/**
 * @brief       Creates a new empty #Panel
 *
 * @return      An empty panel
 */
Panel emptyPanel() {
    Panel p = malloc(sizeof(struct panel));

    p->numberElements = 0;
    p->capacity = 1;
    p->stack = malloc(sizeof(VisualElement) * p->capacity);

    return p;
}

/**
 * @brief       Inserts the given #VisualElement at the bottom of the #Panel
 * 
 * @param panel The #Panel in which to insert the #VisualElement
 * @param elem  The #VisualElement to insert
 */
void panelInsert(Panel panel, VisualElement elem) {
    if(panel->capacity == panel->numberElements) {
        panel->capacity *= 2;
        panel->stack = realloc(panel->stack, sizeof(VisualElement) * panel->capacity);
    }

    panel->stack[panel->numberElements++] = copyVisualElement(elem);
}

/**
 * @brief       Creates a deep copy of the given #Panel
 * 
 * @param panel The #Panel to copy
 * 
 * @return      The copy of the #Panel
 */
Panel copyPanel(Panel panel) {
    Panel copy = malloc(sizeof(struct panel));
    copy->capacity = panel->capacity;
    copy->numberElements = panel->numberElements;

    copy->stack = malloc(sizeof(VisualElement) * copy->capacity);
    for(int i = 0; i < copy->numberElements; i++)
        copy->stack[i] = copyVisualElement(panel->stack[i]);

    return copy;
}

/**
 * @brief   Frees the given #Panel
 * 
 * @param p The #Panel to free
 */
void freePanel(Panel p) {
    for(int i = 0; i < p->numberElements; i++)
        freeVisualElement(p->stack[i]);

    free(p->stack);
    free(p);
}

/**
 * @brief           Sets the selected property of the #Button in the given position
 * 
 * @param panel     The given #Panel
 * @param selected  Whether or not the #Button is selected 
 * @param index     The index of the #Button in the stack
 */
void setPNButtonSelected(Panel panel, bool selected, int index) {
    if(index >= 0 && index < panel->numberElements) {
        setVEButtonSelected(panel->stack[index], selected);
    }   
   
}

/**
 * @brief           Sets the content of the #Text in the given position
 * 
 * @param panel     The given #Panel
 * @param index     The index of the #Button in the stack
 * @param text      The given string (new content of the #Text)
 */
void setPNText(Panel panel, int index, char* text) {
    if(index >= 0 && index < panel->numberElements) {
        setVEText(panel->stack[index], text);
    }   
   
}

/**
 * @brief           Renders the given #Panel based on the given dimensions
 * 
 * @param panel     The #Panel to render
 * @param x         The starting x coordinate
 * @param y         The starting y coordinate
 * @param width     The width of the cell
 * @param height    The height of the cell
 */
void renderPanel(Panel panel, int x, int y, int width, int height) {
    for(int i = 0; i < panel->numberElements; i++) {
        renderVisualElement(panel->stack[i], &x, &y, width, height);
    }
}