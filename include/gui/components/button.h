/**
 * @file button.h
 * 
 * File containing declaration of the #Button component
 */

#ifndef _BUTTON_H_

/**
 * @brief Include guard
 */
#define _BUTTON_H_

#include <stdbool.h>

/**
 * @brief The component representing a clickable button
 * 
 */
typedef struct button *Button;

Button createButton(char*, bool);
void setButtonSelected(Button, bool);
void freeButton(Button b);
Button copyButton(Button b);
void renderButton(Button, int* __restrict__ x, int* __restrict__ y, int, int);

#endif