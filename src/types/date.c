/**
 * @file date.c
 * 
 * File containing the implementation of the #Date type
 * 
 * The #Date type is used to represent a date/time
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "types/date.h"
#include "utils/utils.h"

/**
 * @brief Shorthand for member-wise comparison
 */
#define COMPARE(x)  if(d1->x != d2->x) \
                        result = (d1->x < d2->x) ? -1 : 1;

/**
 * @brief Type used to define date and time
 */
struct date {
    int year;   ///< The year
    int month;  ///< The month
    int day;    ///< The day
    int hour;   ///< The hour
    int minute; ///< The minute
    int second; //< The second
};

/**
 * @brief   Creates an empty #Date
 * 
 * @warning All values are uninitialized
 * 
 * @return  An empty #Date
 */
Date initDate() {
    return malloc(sizeof(struct date));
}

/**
 * @brief           Sets the values a #Date
 *
 * @param date      The given date
 * @param year      The year
 * @param month     The month
 * @param day       The day of the month
 * @param hour      The hour
 * @param minute    The minute
 * @param second    The second
 */
void setDate(Date date, int year, int month, int day, int hour, int minute, int second) {
    date->year   = year;
    date->month  = month;
    date->day    = day;
    date->hour   = hour;
    date->minute = minute;
    date->second = second;
}

/**
 * @brief   Get the size of the struct used for a #Date
 *
 * @return  The size of the struct used for a #Date
 */
int getDateSizeof() {
    return sizeof(Date);
}

/**
 * @brief       Creates a deep copy of a #Date
 *
 * @param date  The #Date to copy
 * 
 * @return      A deep copy of the #Date
 */
Date copyDate(Date date) {
    Date result = (Date)malloc(sizeof(struct date));
    *result = *date;
    return result;
}

/**
 * @brief       Sets a #Date to the current date/time in UTC
 * 
 * @warning     The algorithm ignores leap seconds
 *
 * @param date  The #Date to change
 */
void setToCurrentDate(Date date) {
    /*
        Code based on http://howardhinnant.github.io/date_algorithms.html
    */
    time_t now = time(NULL); //Assume local time is UTC. Also ignore leap seconds
    date->second = now % 60;
    date->minute = (now / 60) % 60;
    date->hour = (now / 3600) % 24;

    //Number of days since 1 January 1970 00:00 (UTC) : 86The year = 24 * 60 * 60
    int days_since_epoch = now / 86400;


    //Shifting epoch from 01-01-1970 to 01-03-0000
    int z = days_since_epoch + 719468; //719468: days from 01-03-0000 to 01-01-1970

    /*
        The algorithm divides time into eras of 400 years each (1600-2000, etc) as the gregorian calendar
        repeats itself every 400 years. Therefore, the era can be factored out of the computation
     */
    int era = (z >= 0 ? z : z - 146096) / 146097; //The current era (currently equal to 5)
    //Days of current era (146097 is number of days in era)
    int doe = z - era * 146097;                                 // [0, 146096]
    //Year of current era (subtractions of doe/x is to remove leap days: there is one every 1460 days, every 3146096 days,
    //but no every 36524 days(multiples of 100 and not 400 are not leap years))
    int yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365;  // [0, 399]
    //Restore the current year
    date->year = yoe + era * 400 + 1;

    //Get the day of the year (account for leap years): Note it's day of the year counting from March 1st
    int doy = doe - (365*yoe + yoe/4 - yoe/100);                // [0, 365]
    //Get the month (counting from March)
    int mp = (5*doy + 2)/153;                                   // [0, 11]
    //Getting the correct day of the month
    date->day = doy - (153*mp+2)/5 + 1;                        // [1, 31]
    //Restoring the correct month (shifting the start back to January)
    date->month = mp + (mp < 10 ? 3 : -9);                     // [1, 12]
}

/**
 * @brief       Checks if the given year is a leap year
 *
 * @param year  The year
 *
 * @return      Whether or not the given year is a leap year
 *
 */
bool isLeapYear(int year) {
    /*
      A year is a leap year if and only if it is either:

    - A multiple of 4 and not a multiple of 100
    - A multiple of 400

    */
    return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}

/**
 * @brief       Checks if the given #Date represents a valid date/time.
 *
 * @warning     It should be noted that in the context of this application a date is said to be valid if
 *              it does not predate the 7th of April 2005 and if it is not a future date.
 *
 * @param date  The given #Date
 *
 * @return      Whether or not the given date is valid
 */
bool isDateValid(Date date) {
    int monthsLength[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    struct date minimumDate, now;
    setDate(&minimumDate, 2005, 4, 6, 23, 59, 59);
    setToCurrentDate(&now);

    //Add extra day to February if leap year
    if(isLeapYear(date->year))
        monthsLength[1] = 29;

    if(date->hour < 0 || date->hour > 23)
        return false;

    if(date->minute < 0 || date->minute > 59)
        return false;

    if(date->second < 0 || date->second > 59)
        return false;

    if(date->month < 1 && date->month > 12)
        return false;

    if(date->day < 1 || date->day > monthsLength[date->month - 1])
        return false;

    //Check for date boundaries
    return (dateCompare(&minimumDate, date) == -1) && (dateCompare(date, &now) == -1);
}

/**
 * @brief       Compares two #Date.
 *
 * @param d1    The first #Date
 * @param d2    The second #Date
 *
 * @return -1   If d1 < d2
 * @return 0    If d1 == d2
 * @return 1    If d1 > d2
 *
 */
int dateCompare(Date d1, Date d2) {
    int result = 0;

         COMPARE(year)
    else COMPARE(month)
    else COMPARE(day)
    else COMPARE(hour)
    else COMPARE(minute)
    else COMPARE(second)

    return result;
}

/**
 * @brief           Reads a #Date from a string
 *
 * @param str       The string containing the date
 * @param length    The length of the string
 * @param date      The #Date  object to write to
 * @param time      Whether or not the string contains time information
 * 
 * @return          Whether or not the operation was successful
 */
bool readDate(char* str, int length, Date* date, bool time) {
    Date d = (Date)malloc(sizeof(struct date));
    bool r = true;
    if (length != (time ? 19 : 10)) {
        r = false;
    } else if(str[4] != '-' || str[7] != '-' || (time && str[10] != ' ') || (time && str[13] != ':') || (time && str[16] != ':')) {
        r = false;
    } else {
        if(     safeStringToInt(str, 4, &d->year)
            &&  safeStringToInt(str + 5, 2, &d->month)
            &&  safeStringToInt(str + 8, 2, &d->day)) {

            if(time &&  safeStringToInt(str + 11, 2, &d->hour)
                &&  safeStringToInt(str + 14, 2, &d->minute)
                &&  safeStringToInt(str + 17, 2, &d->second)) {
                r = isDateValid(d);
            } else {
                //By default set the time to midnight
                d->hour = d->minute = d->second = 0;
                r = isDateValid(d);
            }
        } else {
            r = false;
        }
    }

    if (r) {
        *date = d;
    }

    return r;
}

/**
 * @brief           Checks if a string represents a valid #Date
 *
 * @param str       The given string
 * @param length    The length of the string
 *
 * @return          Whether or not the conversion was successfull (the string had the right format and the date was valid)
 */
bool checkDate(char* str, int length) {
    if (length != 19)
        return false;

    if (str[4] != '-' || str[7] != '-' || str[10] != ' ' || str[13] != ':' || str[16] != ':')
        return false;

    Date d = (Date)malloc(sizeof(struct date));

    bool res = safeStringToInt(str, 4, &d->year)
        && safeStringToInt(str + 5, 2, &d->month)
        && safeStringToInt(str + 8, 2, &d->day)
        && safeStringToInt(str + 11, 2, &d->hour)
        && safeStringToInt(str + 14, 2, &d->minute)
        && safeStringToInt(str + 17, 2, &d->second)
        && isDateValid(d);

    freeDate(d);

    return res;
}


/**
 * @brief       Converts a string into a #Date without validation checks
 *
 * @param str   The given string. The string is changed
 * 
 * @return      The converted #Date
 */
Date unsafeDateFromString(char* str) {
    Date date = (Date )malloc(sizeof(struct date));
    str[4]='\0';
    date->year   = atoi(str);
    str[7]='\0';
    date->month  = atoi(str+5);
    str[10]='\0';
    date->day    = atoi(str+8);
    str[13]='\0';
    date->hour = atoi(str+11);
    str[16]='\0';
    date->minute = atoi(str+14);
    date->second= atoi(str+17);
    return date;
}


/**
 * @brief       Prints a #Date to a file
 * 
 * @param date  The #Date to write to file
 * @param out   The file to write the #Date to
 */
void printDate(Date date, FILE* out) {
    fprintf(out,                           "%d-", date->year);
    fprintf(out, date->month  < 10 ? "0%d-" : "%d-", date->month);
    fprintf(out, date->day    < 10 ? "0%d " : "%d ", date->day);
    fprintf(out, date->hour   < 10 ? "0%d:" : "%d:", date->hour);
    fprintf(out, date->minute < 10 ? "0%d:" : "%d:", date->minute);
    fprintf(out, date->second < 10 ? "0%d"  : "%d",  date->second);
}


/**
 * @brief           Sets the time of day of a #Date
 *
 * @param date      The #Date to modify
 * @param hour      The hour of day
 * @param minute    The minute of the hour
 * @param second    The second of the minute
 */
void setTime(Date date, int hour, int minute, int second) {
    date->hour = hour;
    date->minute = minute;
    date->second = second;
}

/**
 * @brief       Gets the hour from a #Date
 *
 * @param date  The given #Date
 * 
 * @return      The hour of the #Date
 */
int getDateHour(Date date) {
    return date->hour;
}

/**
 * @brief       Gets the minute from a #Date
 *
 * @param date  The given #Date
 * 
 * @return      The minute of the #Date
 */
int getDateMinute(Date date) {
    return date->minute;
}

/**
 * @brief       Gets the second from a #Date
 *
 * @param date  The given #Date
 * 
 * @return      The second of the #Date
 */
int getDateSecond(Date date) {
    return date->second;
}

/**
 * @brief       Gets the day from a #Date
 *
 * @param date  The given #Date
 * 
 * @return      The day of the #Date
 */
int getDateDay(Date date) {
    return date->day;
}

/**
 * @brief       Gets the month from a #Date
 *
 * @param date  The given #Date
 * 
 * @return      The month of the #Date
 */
int getDateMonth(Date date) {
    return date->month;
}

/**
 * @brief       Gets the year from a #Date
 *
 * @param date  The given #Date
 * 
 * @return      The year of the #Date
 */
int getDateYear(Date date) {
    return date->year;
}


/**
 * @brief       Sets the hour of a #Date
 *
 * @param date  The given #Date
 * @param hour  The new hour of the #Date
 */
void setDateHour(Date date, int hour) {
    date->hour = hour;
}

/**
 * @brief           Sets the minute of a #Date
 *
 * @param date      The given #Date
 * @param minute    The new minute of the #Date
 */
void setDateMinute(Date date, int minute) {
    date->minute = minute;
}

/**
 * @brief           Sets the second of a #Date
 *
 * @param date      The given #Date
 * @param second    The new second of the #Date
 */
void setDateSecond(Date date, int second) {
    date->second = second;
}

/**
 * @brief           Sets the year of a #Date
 *
 * @param date      The given #Date
 * @param year      The new year of the #Date
 */
void setDateYear(Date date, int year) {
    date->year = year;
}

/**
 * @brief           Sets the month of a #Date
 *
 * @param date      The given #Date
 * @param month     The new month of the #Date
 */
void setDateMonth(Date date, int month) {
    date->month = month;
}

/**
 * @brief           Sets the day of a #Date
 *
 * @param date      The given #Date
 * @param day       The new day of the #Date
 */
void setDateDay(Date date, int day) {
    date->day = day;
}

/**
 * @brief       Compacts a #Date down to an integer
 *
 * @param date  The #Date to compact
 * 
 * @return      the compacted form of the #Date
 */
int getCompactedDate(Date date) {
    /*
        The date is compacted to a 32-bit integer. 
        First 6 bits -> year (starting from 2005 -> 2005 counts as 0)
        Next 4 bits  -> month
        Next 5 bits  -> day
        Next 5 bits  -> hour
        Next 6 bits  -> minutes
        Next 6 bits  -> seconds
    */

    //We move (<<) the binary representation of the value to their final position and join them (||) for the final representation
    int result = (date->year-2005) << 26 | date->month << 22 | date->day << 17 | date->hour << 12 | date->minute << 6 | date->second;
    return result;
}

/**
 * @brief Get the Uncompacted Date
 *
 * @param c the int to umpack
 * @return the compacted date
 */
Date getUncompactedDate(int c) {
    /*
        The date is compacted to a 32-bit integer. 
        First 6 bits -> year (starting from 2005 -> 2005 counts as 0)
        Next 4 bits  -> month
        Next 5 bits  -> day
        Next 5 bits  -> hour
        Next 6 bits  -> minutes
        Next 6 bits  -> seconds
    */

    Date d = malloc(sizeof(struct date));

    //We shift (>>) the binary representation of the compressed date so that the value we want to extract is at
    //the least significant bits. We then filter (&) all the unwanted bits (the ones to the left of the value)
    d->second = c & 63;
    d->minute = c>>6 & 63;
    d->hour = c>>12 & 31;
    d->day = c>>17 & 31;
    d->month = c>>22 & 15;
    d->year = 2005+(c>>26 & 63);

    return d;
}