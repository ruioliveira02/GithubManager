/**
 * @file File containing declaration of functions used to index a file
 */

/**
 * @brief Include guard
 */
#ifndef _INDEXER_H_

/**
 * @brief Include guard
 */
#define _INDEXER_H_

#include "../utils/utils.h"
#include "../types/format.h"
#include "../types/lazy.h"
#include "cache.h"

#define IMBED_INT(i) ((pos_t)i)
#define GET_IMBEDDED_INT(p) ((int)p)

typedef struct indexer * Indexer;

Indexer makeIndexer(char*, FILE*, FILE*, int (*cmpKeys)(FILE*, pos_t, FILE*, pos_t, Cache));
Indexer parseIndexer(char*, FILE*, FILE*, int (*cmpKeys)(FILE*, pos_t, FILE*, pos_t, Cache));
Indexer parseGroupedIndexer(char*, char*, FILE*, FILE*, int (*cmpKeys)(FILE*, pos_t, FILE*, pos_t, Cache));

bool hasEmbeddedKey(Indexer);
bool hasEmbeddedValue(Indexer);
int getElemNumber(Indexer);

void insertIntoIndex(Indexer, pos_t, pos_t);
void sortIndexer(Indexer, Cache);
void groupIndexer(Indexer, char*, bool, Cache);

int retrieveKey(Indexer, pos_t, Cache);
int retrieveKeyLowerBound(Indexer, pos_t, Cache);

pos_t retrieveEmbeddedKey(Indexer, int, Cache);
pos_t retrieveEmbeddedValue(Indexer, int, Cache);
void retrieveValueAsLazy(Indexer, int, Cache, Lazy);

pos_t getEmbeddedValue(Indexer, pos_t, Cache);
bool findEmbeddedValue(Indexer, pos_t, Cache, pos_t*);
bool findValueAsLazy(Indexer, pos_t, Cache, Lazy);

#define getGroup(i, p, c) getEmbeddedValue(i, p, c)
#define retrieveGroup(i, o, c) retrieveEmbeddedValue(i, o, c)
int getGroupSize(Indexer, pos_t, Cache);
pos_t getGroupElem(Indexer, pos_t, int, Cache);
void getGroupElemAsLazy(Indexer, pos_t, int, Cache, Lazy);

void freeIndexer(Indexer, Cache);

/**
 * @brief End of include guard
 */
#endif
