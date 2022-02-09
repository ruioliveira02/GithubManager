/**
 * @file catalog.h
 * 
 * Function that exports the functions used to interface #Catalog
 */

#ifndef _CATALOG_H_
/**
 * @brief Include guard
 */
#define _CATALOG_H_

#include <stdbool.h>

#include "types/commit.h"
#include "types/repo.h"
#include "types/user.h"
#include "types/lazy.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief The struct representing the dataset
 * 
 */
typedef struct catalog *Catalog;
Catalog newCatalog(char*, char*, char*, bool);
Catalog loadCatalog();

int getUserCountC(Catalog);
int getBotCountC(Catalog);
int getorganizationCountC(Catalog);
int getUsersCount(Catalog);
void getUserById(Catalog,int,Lazy);
void printUserLoginById(Catalog, int, Lazy, FILE*);

int getCommitsCount(Catalog);

int getTotalReposCount(Catalog);
bool getRepoById(Catalog, int, Lazy);

double getValueFromQ2(Catalog);
int getValueFromQ3(Catalog);
double getValueFromQ4(Catalog);
void queryOne(Catalog,FILE*);
void querySeven(Catalog, Date, FILE*);

void freeCatalog(Catalog);
GHashTable* getHashTableOfUserWithCommitsAfter(Catalog,Date,Date,int*);
GHashTable* getHashTableOfCommitsPerLanguage(Catalog,char*,int*);
GHashTable* getHashTableOfNumbersOfAperencesOfALanguageAfter(Catalog,Date);
GHashTable* getHashTableOfCommitCountInReposOfFriends(Catalog);
GHashTable* getHashTableOfLengthOfCommitsInArepoByRepoPositionInList(Catalog,int,int*);

Format getStaticQueriesFormat();

#endif