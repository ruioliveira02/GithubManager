/**
 * @file panel.h
 * 
 * File containing declaration of the #Panel component
 */

#ifndef _PANEL_H_

/**
 * @brief Include guard
 */
#define _PANEL_H_

#include <stdbool.h>

#include "gui/components/visualElement.h"

/**
 * @brief A panel
 * 
 * A panel corresponds to a stack of #VisualElement which are rendered top down
 * 
 */
typedef struct panel* Panel;

Panel emptyPanel();
void setPNButtonSelected(Panel, bool, int);
void setPNText(Panel, int, char*);
void panelInsert(Panel, VisualElement);
void renderPanel(Panel, int, int, int, int);
Panel copyPanel(Panel);
void freePanel(Panel);

#endif