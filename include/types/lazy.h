/**
 * @file lazy.h
 * ~
 * File declaring the #Lazy type
 */

#ifndef _LAZY_H_

/**
 * @brief Include guard
 * 
 */
#define _LAZY_H_

#include "utils/utils.h"
#include "io/cache.h"
#include "format.h"

/**
 * @brief The struct which defines the Lazy type
 * 
 */
typedef struct lazy * Lazy;


Lazy makeLazy(FILE*, pos_t, Format, void*);
void* getLazyMember(Lazy, int, Cache);
void* setLazyMember(Lazy, int);


pos_t getPosOfLazyMember(Lazy, int, Cache);
pos_t getPosAfterLazy(Lazy, Cache);

void printLazyToFile(Lazy, Cache);
void setLazyObj(Lazy, void*);
void setLazyAddress(Lazy, FILE*, pos_t);
void freeLazy(Lazy);

#endif