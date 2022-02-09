/**
 * @file date.h
 * 
 * File containing the declaration of helper functions regarding the #Date type
 * 
 */


#ifndef _DATE_H_
/**
 * @brief Include guard
 */
#define _DATE_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "utils/utils.h"

/**
 * @brief The length of a #Date string without time information
 * 
 */
#define DATE_LENGTH 10

/**
 * @brief The length of a #Date string with time information
 * 
 */
#define DATE_TIME_LENGTH 19


/**
 * @brief The length of a compressed #Date (in bytes)
 * 
 */
#define BINARY_DATE_TIME_LENGTH sizeof(int)

/**
 * @brief The struct representing dates
 * 
 */
typedef struct date *Date;

Date initDate();
void setDate(Date, int, int, int, int, int, int);
void setToCurrentDate(Date);
bool isLeapYear(int);
bool isDateValid(Date );

int dateCompare(Date ,Date );
int getDateSizeof();
Date  copyDate(Date );
bool readDate(char*, int, Date *, bool);
bool checkDate(char*, int);
Date unsafeDateFromString(char*);


void printDate(Date , FILE *);
void setTime(Date , int, int, int);
int getDateHour(Date );
int getDateMinute(Date );
int getDateSecond(Date );
int getDateDay(Date );
int getDateMonth(Date );
int getDateYear(Date );
void setDateHour(Date ,int );
void setDateMinute(Date ,int );
void setDateSecond(Date ,int );
void setDateYear(Date ,int );
void setDateMonth(Date ,int );
void setDateDay(Date ,int );


int getCompactedDate(Date);
Date getUncompactedDate(int);

/**
 * @brief The function which frees a #Date
 * 
 */
#define freeDate(d) free(d)

#endif