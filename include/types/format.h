/**
 * @file format.h
 *
 * File containing the declaration of functions related to the #Format
 *
 */


#ifndef _FORMAT_H_
/**
 * @brief Include guard
 */
#define _FORMAT_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "utils/utils.h"

/**
 * @brief  Shorthand for the internal_pair struct
 */
typedef struct internal_pair INTERNAL_PAIR;

/**
 * @brief Shorthand for the format struct
 *
 */
typedef struct format *Format;

/**
 * @brief Shorthand for the pair struct
 */
typedef struct pair PAIR;




/**
 * @brief Enum that specifies the type of representation as string, storage in memory and
 *          validation upon reading of a format member
 *
 * @note  Numbering convention: rightmost bit is set on binary types, reference (malloc'd) types are bigger than or equal to 16
 */
typedef enum format_type
{
    BOOL = 0,               ///< bool
    BINARY_BOOL = 1,        ///< bool (binary)
    TYPE = 2,               ///< Type
    BINARY_TYPE = 3,        ///< Type (binary)
    INT = 4,                ///< int
    BINARY_INT = 5,         ///< int (binary)
	BINARY_DOUBLE = 7, 		///< double (binary)

    STRING = 17,            ///< char* (can not be null)
    STRING_NULL = 19,       ///< char* (can be null)
    INTLIST = 20,           ///< int*
    BINARY_INTLIST = 21,    ///< int* (binary ints)
    DATE = 22,              ///< Date without time
    DATE_TIME = 24,         ///< Date with time
    BINARY_DATE_TIME = 25,  ///< Date with time (binary)
} FormatType;
int getSizeOfPair();
void addToPair(PAIR*,int,int,void*);
bool isAllocd(FormatType);
int stringSize(FormatType);
int elemStringSize(FormatType);
void readBinaryMember(FormatType, char*, int, void*);
void writeBinaryMember(FormatType, void*, char*, int);

Format makeFormat(void*, void**, FormatType*, int, int, PAIR*, int, char);
Format copyFormat(Format);
bool isBinary(Format);
int getFormatSize(Format);
int getFormatMembers(Format);
FormatType getMemberType(Format, int);
int getListPairListMember(Format, int);
int getListPairLengthMember(Format, int);

void* getMember(Format, void*, int);
void freeMember(Format, void*, int);

bool checkFormat(Format, char*);
bool readFormat(Format, char*, void*);
void unsafeReadFormat(Format, char*, void*);
void printFormat(Format, void*, FILE*);
void freeFormat(Format, void*);

void disposeFormat(Format);


#endif
