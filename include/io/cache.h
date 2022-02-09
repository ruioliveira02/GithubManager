/**
 * @file cache.h
 * 
 * File containing declaration of functions used to create the program cache
 */

#ifndef _CACHE_H_

/**
 * @brief Include guard
 */
#define _CACHE_H_

#include "../utils/utils.h"

/**
 * @brief The struct representing the cache of the program
 * 
 */
typedef struct cache * Cache;

/**
 * @brief A position in a file (number of bytes since start of file)
 * 
 */
typedef unsigned long long pos_t;

/**
 * @brief The maximum value of @ref pos_t
 * 
 */
#define POS_T_MAX ULONG_MAX

/**
 * @brief The size of a #Line of #Cache
 * 
 */
#define CACHE_LINE_SIZE 1024

Cache getCache(int);
int getLine(Cache, FILE*, pos_t, char[], int);
int getStr(Cache, FILE*, pos_t, char[], int);
int getInt(Cache, FILE*, pos_t);
pos_t getPosT(Cache, FILE*, pos_t);
void setStr(Cache, FILE*, pos_t, char[], int);

void flushCacheFile(Cache, FILE*);
void flushCache(Cache);

void refreshCacheFile(Cache, FILE*);
void refreshCache(Cache);

void clearCacheFile(Cache, FILE*);
void clearCache(Cache);

void freeCache(Cache);

#endif