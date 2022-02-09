/**
 * @file statisticsPage.h
 * 
 * File containing declaration of the function used to create the statistics page
 */

#ifndef _STATISTICS_PAGE_H_

/**
 * @brief Include guard
 */
#define _STATISTICS_PAGE_H_

#include "gui/page.h"


/**
 * @brief The number of different statistics
 */
#define STATISTICS_COUNT 4

char* getStatisticFileName(int);

Page statisticsPage();

#endif