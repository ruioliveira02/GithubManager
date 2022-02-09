/**
 * @file text.h
 * 
 * File containing the definition of the #Text component.
 * 
 */

#ifndef _TEXT_H_

/**
 * @brief Include guard
 */
#define _TEXT_H_

/**
 * @brief The componennt representing basic text
 * 
 */
typedef struct text* Text;

Text createText(char*, int);
Text createTextFromWide(wchar_t*, int);
Text copyText(Text);
void setText(Text, char*);
void printText(Text, int*, int*, int);
void freeText(Text);

#endif