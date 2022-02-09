/**
 * @file page.h
 * 
 * File containing declaration of functions and the type used
 * to represent a page in the #GUI
 */

#ifndef _PAGE_H_
/**
 * @brief Include guard
 */
#define _PAGE_H_

#include <stdbool.h>

#include "utils/utils.h"
#include "types/queries.h"

/**
 * @brief Defines the id of the query used to kill the application
 * 
 */
#define KILLAPP -21

/**
 * @brief Defines the id of the query used to load the query menu
 * 
 */
#define LOADQUERYMENU -2

/**
 * @brief Defines the id of the query used to load the catalog menu
 */
#define LOADCATALOGMENU -20

/**
 * @brief Defines the id of the query used to load the main menu
 */
#define LOADMAINMENU -25

/**
 * @brief Defines the starting id of the queries used to load the query pages
 * 
 */
#define LOADQUERY -2

/**
 * @brief Defines the starting id of the catalog loading queries
 */
#define LOADCATALOG -15

/**
 * @brief Defines the id of the query used to load the query x
 * 
 */
#define LOADQUERYID(x) (LOADQUERY - x)

/**
 * @brief Defines the id of the query used to load the catalog number x
 */
#define LOADCATALOGID(x) (LOADCATALOG - x)

/**
 * @brief A Page in the #GUI
 * 
 */
typedef struct page *Page;

/**
 * @brief A panel of a #Page
 * 
 */
typedef struct panel *Panel;

void setPageButtonSelected(Page, int, int, int, bool);
void setPageText(Page, int, int, int, char*);

void freePage(Page);

Page newPage(int, int, float*, float*, void* (*)(void*[]),void (*)(void*), Query (*)(void*, int),void (*)(Page, void*));
void* (*getDefaultStateFunction(Page))(void*[]); 
void (*getFreeStateFunction(Page))(void*);
void setPagePanel(Page, Panel, int, int);
void renderPage(Page, void*);

Query processKey(Page, void*, int);

void getScreenDimensions(int*, int*);

#endif