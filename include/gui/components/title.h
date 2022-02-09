/**
 * @file title.h
 * 
 * File containing declaration of the #Title component
 */

#ifndef _TITLE_H_

/**
 * @brief Include guard
 */
#define _TITLE_H_

/**
 * @brief The size of the small title
 */
#define SIZE_SMALL 5

/**
 * @brief The size of the big title
 */
#define SIZE_BIG 9

/**
 * @brief The component which corresponds to a title
 * 
 */
typedef struct title *Title;

Title createTitle(char*, int);
Title copyTitle(Title);
void renderTitle(Title, int*, int*, int);
void freeTitle(Title);

#endif