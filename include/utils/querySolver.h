/**
 * @file querySolver.h
 * 
 * File containing declaration of functions which solve a #Query
 */

#ifndef _QUERY_SOLVER_H_

/**
 * @brief Include guard
 */
#define _QUERY_SOLVER_H_

#include "types/commit.h"
#include "types/user.h"
#include "types/repo.h"
#include "types/catalog.h"

//queryOne is decalred in catalog.h

double queryTwo(Catalog);

int queryThree(Catalog);

double queryFour(Catalog);

void queryFive(Catalog,int,Date,Date,FILE*);

void querySix(Catalog,int,char*,FILE*);

//querySeven is decalred in catalog.h

void queryEight(Catalog,int,Date,FILE*);

void queryNine(Catalog,int,FILE*);

void queryTen(Catalog,int,FILE*);

#endif