/**
 * @file utils.h
 * 
 * File declaring auxiliary functions and types
 *
 */

#ifndef _TYPES_H_
/**
 * @brief Include guard
 */
#define _TYPES_H_


#include <stddef.h>
#include <stdio.h>
#include <glib.h>
#include <errno.h>
#include <stdbool.h>

//#define DEBUG

#ifdef DEBUG
/**
 * @brief Prints used for debugging
 * 
 */
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
/**
 * @brief Prints used for debugging
 * 
 */
#define DEBUG_PRINT(...)
#endif

/**
 * @brief #Commit input file
 */
#define COMMITS_IN "entrada/commits-g3.csv"

/**
 * @brief #Repo input file
 */
#define REPOS_IN "entrada/repos-g3.csv"

/**
 * @brief #User input file
 */
#define USERS_IN "entrada/users-g3.csv"

/**
 * @brief Tries to open a file. If the file can't be opened, an error message is displayed and the program exits
 */
#define OPEN_FILE(filename, mode) ({ errno = EFAULT; FILE* f = fopen(filename, mode); \
            if (!f) { fprintf(stderr, "Error opening file '%s'. Error code: %d\n", filename, errno); \
                  exit(EXIT_FAILURE); } f; })

/**
 * @brief Tries to open a file with both reading and writting perms. If the file exists, the cursor is set at the beginning
 *        of the file. If it doesn't exist, a new file is created.
 *        If the file is binary, pass "b" as binary. Otherwise, pass ""
 */
#define OPEN_MAYBE_FILE(filename, binary) ({FILE* f = fopen(filename, "r" binary "+"); \
            if (!f) { f = OPEN_FILE(filename, "w" binary "+"); } f; })

/**
 * @brief Type used to define a user
 */
typedef enum type{
    USER=0, //< Used for type User
    ORGANIZATION=1,//< Used for type Organization
    BOT=2//< Used for type Bot
} Type;

/**
 * @brief A dynamic string
 * 
 */
typedef struct string *String;

String newString(char*);
void stringAppend(String, char);
int stringLength(String str);

void stringLastRemove(String);
void freeString(String);
char* getStringContent(String);

bool isValidStringInput(int);

char* toLower(char* s);
int trimNewLine(char*, int);

bool checkInt(char*, int);
bool safeStringToInt(char*, int, int*);
bool checkType(char*, int, Type*);
bool checkBool(char*, int, bool*);
bool checkIdList(char*, int, int*);
bool readIdList(char*, int, int**, int*);
int* unsafeReadIdList(char*, int, int*);

int* copyIdList(int*, int);
bool containedInSortedArray(int*, int, int);

const char * printType(Type);
void printAr(int*,FILE*,int);
int safestrlen(char*);
void freeGArray(gpointer);

void removeDuplicates(GArray*);

char* getQueryFileName();
char** getFileContent(char*, int, int, int*);
int getFileLine(FILE*, char**, int*);

void writeIntToBinaryString(char*,int);
void IntListToBinaryString(char **,int,int*);
int readIntFromBinaryString(char *);
int* BinaryStringTointList(char *,int );
bool binSearchInList(int,int*,int);
void increaseNumberInHashTableIfFound(GHashTable*,gpointer,int*);
void storeNumberInHashTableIfGreater(GHashTable*,gpointer,int);

#endif