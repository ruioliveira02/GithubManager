/**
 * @file queries.h
 * 
 * File containing declaration of functions used to parse and execute queries
 */

#ifndef _QUERIES_H_

/**
 * @brief Include guard
 */
#define _QUERIES_H_

#include "utils/utils.h"
#include "types/format.h"
#include "types/catalog.h"

/**
 * @brief The struct representing a Query
 * 
 */
typedef struct query *Query;
Query createEmptyQuery();
Query createQueryId(int);

void executeQuery( FILE* ,Query,Catalog);

void parseQuery(char*, Query);

Format getQueryFormat(int);
int getQueryId(Query );
void freeQuery(Query);


/**
 * @brief End of include guard
 */
#endif