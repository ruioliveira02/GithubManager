/**
 * @file gui.c
 * 
 * File containing declaration of functions used to interact with the #GUI
 */

#ifndef _GUI_H_
/**
 * @brief Include guard
 */
#define _GUI_H_

#include "gui/page.h"

/**
 * @brief The struct responsible for handling everything UI related
 * 
 */
typedef struct gui *GUI;

GUI loadGUI();

void freeGUI(GUI);

void runGUI(GUI);

#endif