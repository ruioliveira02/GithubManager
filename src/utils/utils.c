/**
 * @file utils.c
 * 
 * File containing type-specific and auxiliary functions
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <glib.h>
#include <unistd.h>
#include <sys/types.h>
#include <termios.h>

#include "utils/utils.h"


/**
 * @brief Wrapper for a dynamic string
 */
struct string {
    char* content;  ///< The content of the string
    int size;       ///< The size of the string (excluding '\0')
    int capacity;   ///< The capacity of the array
};

/**
 * @brief       Creates a #String from an array of characters
 *
 * @param str   The given string as an array of characters
 *
 * @return      The new #String
 */
String newString(char* str) {
    String res = malloc(sizeof(struct string));

    res->size = strlen(str);
    res->capacity = res->size + 1;

    res->content = malloc(sizeof(char) * res->capacity);
    strcpy(res->content, str);

    return res;
}

/**
 * @brief       Returns the length of the given #String
 *
 * @param str   The given #String
 *
 * @return      The length of the #String
 */
int stringLength(String str) {
    return strlen(str->content);
}

/**
 * @brief       Appends the given character to the end of the #String
 *
 * @param str   The given #String
 * @param ch    The given character
 */
void stringAppend(String str, char ch) {
    if(str->size == str->capacity - 1) {
        str->capacity *= 2;
        str->content = realloc(str->content, sizeof(char) * str->capacity);
    }

    str->content[str->size++] = ch;
    str->content[str->size] = '\0';
}

/**
 * @brief       Removes the last character of a #String
 *
 * @param str   The given #String
 */
void stringLastRemove(String str) {
    if(str->size != 0) {
        str->content[--str->size] = '\0';
    }
}

/**
 * @brief       Returns a copy of the content of a #String
 *
 * @param str   The given #String
 * 
 * @return      A copy of the content of the #String
 */
char* getStringContent(String str) {
    char* result = malloc(sizeof(char) * str->capacity);
    strcpy(result, str->content);
    return result;
}

/**
 * @brief       Returns whether or not the character is admissible in the input fields.
 *
 *              A character is valid if it is a digit, a letter, or a '-'
 *
 * @param ch    The given character
 *
 * @return      Whether or not the character is valid
 */
bool isValidStringInput(int ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') || (ch == '-')
        || (ch == '+') || (ch == '#');
}

/**
 * @brief       Frees the allocated memory for a #String
 *
 * @param str   The #String to free
 */
void freeString(String str) {
    free(str->content);
    free(str);
}

/**
 * @brief       Alters the given string. All caracters are transformed into their lower cased versions. Returns the same pointer
 *
 * @param s     The string to alter
 * 
 * @return      The same input pointer s
 */
char* toLower(char* s) {
    if (s != NULL)
        for (char* i = s; *i != '\0'; i++)
            *i = g_ascii_tolower(*i);
    return s;
}

/**
 * @brief       Removes '\n' and '\r' from the end of the string, if they exist
 *
 * @param s     The string to trim
 * @param len   The length of the string
 * 
 * @return      The new length of the string
 */
int trimNewLine(char* s, int len) {
    if (s != NULL) {
        if (s[len - 1] == '\n')
            s[--len] = '\0';
        if (s[len - 1] == '\r')
            s[--len] = '\0';

        return len;
    }
    else
        return 0;
}

/**
 * @brief           Determines wwether the string can be parsed to a non-negative integer
 *
 *
 * @param str       The string to test. Not necessarily null-terminated
 * @param length    The length of the string
 *
 * @return          True (1) if the string is a valid non-negative integer
 * @return          False (0) otherwise
 */
bool checkInt(char* str, int length) {
    if (!length)
        return false;

    for (; length > 0; length--, str++)
        if (*str < '0' || *str > '9')
            return false;

    return true;
}

/**
 * @brief           Safely converts a string to an integer.
 *
 *
 * @param str       The string to convert to integer. Not necessarily null-terminated
 * @param length    The length of the string
 * @param i         The variable in which to store the result of the conversion
 *
 * @return          True (1) if the string is a valid integer
 * @return          False (0) otherwise
 */
bool safeStringToInt(char* str, int length, int* i)
{
    if (!checkInt(str, length))
        return false;

    //Make sure to null terminate the string, without losing any data
    //So we keep a temporary variable to store the next character in the array
    char temp = str[length];
    str[length] = '\0';

    (*i) = atoi(str);

    //And restore it after we convert
    //We do this because we only want to convert the substring of the given length, meaning
    //the str is not null terminated by default in the length-th position
    str[length] = temp;
    return true;
}

/**
 * @brief           Safely converts a string to a ::Type enum.
 *
 *
 * @param str       The string to convert to integer
 * @param length    The length of the string
 * @param t         The variable in which to store the result of the conversion
 *
 * @return          True (1) if the string is a valid integer
 * @return          False (0) otherwise
 */
bool checkType(char* str, int length, Type *t) {
    if (length == 3) {
        if (!strcmp(str,"Bot")) {
            *t=BOT;
            return true;
        }
    } else if (length == 4) {
        if (!strcmp(str,"User")) {
            *t=USER;
            return true;
        }
    } else if (length == 12) {
        if (!strcmp(str,"Organization")) {
            *t=ORGANIZATION;
            return true;
        }
    }
    return false;
}


/**
 * @brief           Safely converts a string to boolean ("True" or "False")
 *
 * @param str       The string to convert to boolean
 * @param length    The length of the string
 * @param dest      The variable in which to store the result of the conversion
 *
 * @return          True (1) if the string is a valid boolean
 * @return          False (0) otherwise
 */
bool checkBool(char* str, int length, bool* dest) {
    if (length == 4) {
        if (!strcmp(str, "True")) {
            *dest = true;
            return true;
        }
    } else if (length == 5) {
        if (!strcmp(str, "False")) {
            *dest = false;
            return true;
        }
    }
    return false;
}

/**
 * @brief           Checks if it is possible to convert the given string to a list of integers
 *
 * @param str       List of the ids as a string
 * @param length    The number of ids in the list
 * @param list_size Number of ids found (return)
 * 
 * @return true     If the string is a valid list of integers
 * @return false    Otherwise
 */
bool checkIdList(char* str, int length, int* list_size) {
    if (length < 2) return false;
    if (str[0] != '[' || str[length - 1] != ']') return false;
    bool first_digit = true;
    *list_size = 0;

    for (int i = 1; i < length - 1; i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            if (first_digit) {
                (*list_size)++;
                first_digit = false;
            }
        }
        else if (first_digit || str[i] != ',')
            return false;
        else if (str[++i] != ' ')
            return false;
        else
            first_digit = true;
    }

    return length == 2 || !first_digit;
}

/**
 * @brief           Safely converts a string to an integer list.
 *                  If successful and the size of the list is greater than 0, allocates an array
 *                  and stores its size in *list_size. If str is a valid empty list, *list is set to NULL
 *
 * @param str       The string to convert to integer list. Is altered (strstep)
 * @param length    The length of the string
 * @param list      The variable in which to store the result of the conversion
 * @param list_size The size of the converted integer list
 *
 * @return True     If the string is a valid integer
 * @return False    Otherwise
 */
bool readIdList(char *str, int length, int **list, int* list_size) {

    if (!checkIdList(str, length, list_size))
        return false;

    // case of []
    if (*list_size == 0) {
        *list = NULL;
        return true;
    }

    //guardar espaço para as strings
    *list = malloc(*list_size * sizeof(int));
    //str[length - 1] = '\0';
    int n = 0;

    while (str) {
        str++;
        char* temp=strsep(&str,",");
        (*list)[n++] = atoi(temp); //TODO: fix atoi (called without null terminator)
    }

    return true;
}

/**
 * @brief           Converts a string with the format "[1, 2, 3, ...]" into an integer array
 *       
 * @warning         This function does not check or validate the input
 *
 * @param str       The input string
 * @param list_size The size of the resulting list
 * 
 * @return NULL     If list_size is 0
 * @return          A pointer to the allocated array
 */
int* unsafeReadIdList(char* str, int str_len, int *list_size) {
    if (str_len <= 2)
        return NULL;

    //The 10 here is an estimate of how many characters each member takes up
    //With the comma and the space between each character, that leaves each integer taking up 8 characters
    int list_capacity = str_len / 10;

    int* list = malloc(list_capacity * sizeof(int));
    *list_size = 0;
    str++;

    while (*str != ']') {
        if (*list_size >= list_capacity) {
            list_capacity *= 1.5;
            list = realloc(list, list_capacity);
        }

        list[*list_size] = atoi(str);    //TODO: fix atoi (called without null terminator)
        str += (int)log10(list[(*list_size)++]) + 3; //assume the id is positive
    }

    return list;
}

/**
 * @brief       Returns a deep copy of a list
 *
 * @param list  The list to copy
 * @param size  The number of elements of the list
 * 
 * @return      The copy
 */
int* copyIdList(int* list, int size) {
    int* copy = malloc(sizeof(int) * size);
    memcpy(copy, list, sizeof(int) * size);
    return copy;
}

/**
 * @brief           Binary searches the array for the specified element. If found, returns true
 *
 * @param array     The array
 * @param val       The specified element
 * 
 * @return          Whether or not the element exists in the array
 */
bool containedInSortedArray(int* array, int array_length, int val) {
    int l = 0, r = array_length - 1, m;

    while (m = (l + r) / 2, l <= r)
    {
        if (array[m] < val)
            l = m + 1;
        else if (array[m] > val)
            r = m - 1;
        else
            return true;
    }

    return false;
}

/**
 * @brief Returns the string representation of a ::Type
 *
 * @param a         The ::Type
 *
 * @returns         The string representation of ::Type
 */
const char* printType(Type a) {
    if (a==USER)
        return "User";
    if (a==ORGANIZATION) 
        return "Organization";
    else 
        return "Bot";
}

/**
 * @brief           Prints an integer list to a stream
 *
 * @param a         The integer list
 * @param f         The stream
 * @param n         The size of the integer list
 */
void printAr(int *a, FILE *f, int n) {
    int i;
    fprintf(f,"[");
    for (i=0;i<n-1;i++) fprintf(f,"%d, ",a[i]);
    if (i<n) fprintf(f,"%d",a[i]);
    fprintf(f,"]");
}

/**
 * @brief Wrapper of GLib's g_array_free function
 *
 * @param pointer   The array to free
 */
void freeGArray(gpointer pointer) {
    g_array_free((GArray*)pointer, TRUE);
}


/**
 * @brief       Function to safely return the length of a string
 *
 * @param str   The given string
 * 
 * @return 0    If the string is NULL
 * @return      The size of the string
 */
int safestrlen(char*str) {
    if (str) return strlen(str);
    else return 0;
}

/**
 * @brief           Removes duplicates of a GArray of ints
 *
 * @param array     The given GArray
 */
void removeDuplicates(GArray* array) {
    if(array == NULL || array->len == 0)
        return;

    int N = array->len;

    int write = 1;
    int prev = GPOINTER_TO_INT(g_array_index(array, int, 0));
    for(int i = 1; i < N; i++) {
        int cur = GPOINTER_TO_INT(g_array_index(array, int, i));

        if(cur != prev) {
            g_array_index(array, int, write) = cur;
            write++;
        }
    }

    if(write < N) {
        g_array_remove_range(array, write, N - write);
    }
}

/**
 * @brief   Gets the first available file name for query execution
 *
 * @return  The file name
 */
char* getQueryFileName() {
    char no[190];
    sprintf(no, "saida/temp%d.txt", getpid());


    return strdup(no);
}

/**
 * @brief           Prints the requested lines of a file to an array of strings
 *
 * @param filename  The path of the file to read
 * @param begin     The first line to read (0-indexed, inclusive)
 * @param end       The last line to read (0-indexed, inclusive)
 * @param size      The variable in which to return the size of the array
 *
 * @return          The contents of the file from the lines [begin, end]
 */
char** getFileContent(char* filename, int begin, int end, int* size) {
    FILE* file = fopen(filename, "r");

    if(!file)
        return NULL;

    char** result = malloc(sizeof(char*) * (end - begin + 1));

    int bufferSize = 1024;
    char* buffer = malloc(sizeof(char) * bufferSize);
    *size = 0;
    if(file) {
        int index = 0;

        while(index < begin && getFileLine(file, &buffer, &bufferSize)) {
            ++index;
        }

        while(index <= end && getFileLine(file, &buffer, &bufferSize)) {
            result[index - begin] = strdup(buffer);
            ++index;
        }
        *size = MAX(0, index - begin);
    }

    fclose(file);
    free(buffer);
    return result;
}

/**
 * @brief               Gets the line from the file. Removes the '\r' and '\n' at the end of the line
 *                      If *buffer is NULL or too small to hold the line, it is realloc'd and its size stored in *buffer_size
 *
 * @param file          The stream to read
 * @param buffer        A pointer to the buffer to store the line read. *buffer must be freed
 * @param buffer_size   A pointer to the size of the buffer, in characters
 *
 * @return              The number of characters in the line read (excluding '\r', '\n' and '\0')
 */
int getFileLine(FILE* file, char** buffer, int* buffer_size) {
    if (*buffer == NULL) {
        *buffer_size = 10000;
        *buffer = malloc(*buffer_size * sizeof(char));
    }

    int old_size = 0; //we start at 0 to indicate no reallocs have been made
    (*buffer)[*buffer_size - 1] = 1;

    if (!fgets(*buffer, *buffer_size, file)) {
        (*buffer)[0] = '\0';
        return 0;
    }

    //the placeholder value we put at the end of the buffer has been overwritten
    //this means the buffer is too small and needs to be reallocated
    while ((*buffer)[*buffer_size - 1] == '\0') {
        old_size = *buffer_size;
        *buffer_size *= 2;
        *buffer = realloc(*buffer, *buffer_size * sizeof(char));
        (*buffer)[*buffer_size - 1] = 1;
        if (!fgets(*buffer + old_size - 1, *buffer_size - old_size + 1, file))
            break;
    }

    int len = old_size + strlen(*buffer + old_size);
    return trimNewLine(*buffer, len);
}

/**
 * @brief       Writes a int into a binary string
 *
 * @param bytes The binary string to write in
 * @param n     The int to write to
 */
void writeIntToBinaryString(char* bytes, int n) {
    (bytes)[0] = (n >> 24) & 0xFF;
    (bytes)[1] = (n >> 16) & 0xFF;
    (bytes)[2] = (n >> 8) & 0xFF;
    (bytes)[3] = n & 0xFF;
}

/**
 * @brief       Returns the binary string from the ints list
 *
 * @param bytes The binary string to save the ints on
 * @param N     The number of elements of the array
 * @param l     The array of ints
 */
void IntListToBinaryString(char** bytes, int N, int l[]) {
    if (!(*bytes)) 
        *bytes = malloc(N * sizeof(int) * sizeof(char));

    for(int i=0;i<N;i++) {
        int aux =  l[i];
        writeIntToBinaryString(*bytes + (sizeof(int) * i), aux);
    }
}


/**
 * @brief reads a int from the binary string
 *
 * at some point in time i completly forgot how this works but it works so...
 *
 * @param bytes the binary string
 * @return the int found in the string
 */
int readIntFromBinaryString(char *bytes){
    return ((((bytes[0]<<24) & 0xFF000000) | ((bytes[1]<<16) & 0x00FF0000) | ( (bytes[2]<<8) & 0x0000FF00) | (((bytes[3]) & 0x000000FF))));
}


/**
 * @brief           Transforms a binary string with a given number of ints to a list
 *
 * @param bytes     The string to read from
 * @param N         The number of elements in the string
 * 
 * @return          The list of ints found
 */
int* BinaryStringTointList(char *bytes, int N) {
	if (N==0) 
        return NULL;

    int *l=malloc(sizeof(int)*N);

    for (int i=0; i<N; i++)
        l[i] = readIntFromBinaryString(bytes+sizeof(int)*i);

    return l;
}

/**
 * @brief 		    Finds if a number is inside of a list using binary search
 *
 * @param key   	The number to search for
 * @param l 	    The list of numbers to seearch in
 * @param N 	    The number of elements in the list
 * 
 * @return true     If the element is in the list
 * @return false    If the element is not in the list
 */
bool binSearchInList(int key, int*l, int N) {
	int m = 0,n = N-1,p = (m + n) / 2;

	while(l[p] != key){
		if (l[p]>key) 
            n = p-1;
		else 
            m = p+1;

		p = (m + n) / 2;
	}

	return key == l[p];
}

/**
 * @brief 			Increses the value stored in a given key of a hashtable by 1 if is not already in the hastable stores it as 1
 *
 * @param users 	The hashTable to search in
 * @param id 		The key to search by
 * @param new 		Pointer to store if a new key was added to the hashtable
 */
void increaseNumberInHashTableIfFound(GHashTable* users, gpointer id, int*new) {
	gpointer searchResult = g_hash_table_lookup(users,id);
    if(searchResult != NULL) {
        int newValue = GPOINTER_TO_INT(searchResult) + 1;
        g_hash_table_replace(users,id,GINT_TO_POINTER(newValue));
    } else {
        g_hash_table_insert(users,id,GINT_TO_POINTER(1));
        *new=(*new)+1;
    }
}

/**
 * @brief 			Increses the value stored in a given key of a hashtable by a given value if this one is greater than the previous one or if its not in the hash table already.
 *
 * @param users 	The hashTable to search in
 * @param id 		The key to search by
 * @param value 	The value to store in the key
 */
void storeNumberInHashTableIfGreater(GHashTable* users, gpointer id, int value) {
	gpointer searchResult = g_hash_table_lookup(users,id);
    if(searchResult != NULL) {
		if(GPOINTER_TO_INT(searchResult)<value) g_hash_table_replace(users,id,GINT_TO_POINTER(value));
    } else {
        g_hash_table_insert(users,id,GINT_TO_POINTER(value));
    }
}
