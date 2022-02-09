/**
 * @file table.h
 * 
 * File containing declaration of functions used to turn query output into a nice table
 * 
 */


#ifndef _TABLE_H_

/**
 * @brief Include guard
 * 
 */
#define _TABLE_H_

#include <wchar.h>

char* centerString(char*, int);
wchar_t* centerWideString(wchar_t*, int);
wchar_t** queryOutputToTable(char**, int*, wchar_t**, int);
wchar_t** formatButton(char*, int, int);
wchar_t* stringToWide(char*);

#endif
