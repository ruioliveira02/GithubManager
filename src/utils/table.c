/**
 * @file table.c
 * 
 * File containing implementation of functions used by the GUI to prettify the output of strings
 * 
 */


#include <stdlib.h>
#include <string.h>
#include <wchar.h>


#include "utils/table.h"
#include "utils/utils.h"


/**
 * @brief       Calculates the maximum length of a string in the given array
 * 
 * @param array The given array
 * @param N     The size of the array
 * 
 * @return      The maximum length of a string in the given array
 */
int maximumLength(wchar_t** array, int N) {
    int ans = 0;
    const int maxAns = 30;
    for(int i = 0; i < N; i++) {
        ans = MAX(ans, wcslen(array[i]));
    }

    return MIN(ans, maxAns);
}

/**
 * @brief               Splits a string by the given separator
 * 
 * @param string        The given string
 * @param separator     The separator
 * @param elementCount  The number of elements
 * 
 * @returns             The array of strings (not malloc'ed)
 */
char** splitString(char* string, char separator, int elementCount) {
    char** result = malloc(sizeof(char*) * elementCount);

    char* cur = string;
    int count = 0;
    bool prev = true;

    while(*cur && *cur != '\n') {
        if(prev) {
            result[count++] = cur;
            prev = false;
        }
        if(*cur == separator) {
            *cur = '\0';
            prev = true;
        }

        ++cur;
    }

    if(*cur == '\n')
        *cur = '\0';

    return result;
}

/**
 * @brief               Splits the string into a table. Each column contains all the first elements of the output.
 *                      For example, if the querystring is "1;2;3", the string "1" will be in the first column.
 * 
 * @param output        The output of the query
 * @param size          The size of the array
 * @param elementCount  The number of elements in each query
 * @param separator     The character used to separate elements
 * 
 * @return              The divided table
 */
wchar_t*** splitQueryOutput(char** output, int size, int elementCount, char separator) {
    wchar_t*** result = malloc(sizeof(wchar_t**) * elementCount);

    for(int i = 0; i < elementCount; i++)
        result[i] = malloc(sizeof(wchar_t*) * size);

    for(int i = 0; i < size; i++) {
        char** split = splitString(output[i], separator, elementCount);

        for(int j = 0; j < elementCount; j++) {
            result[j][i] = stringToWide(split[j]); 
        }
        free(split);
    }

    return result;
}

/**
 * @brief       Centers a string to the desired length
 * 
 * @param str   The given string
 * @param N     The given length
 * 
 * @return      The resulting string 
 */
char* centerString(char* str, int N) {
    int length = strlen(str);
    int buffer = (N - length) / 2;

    char* ans = malloc(sizeof(char) * (N + 1));

    int i;
    for(i = 0; i < buffer; i++)
        ans[i] = ' ';

    for(; i - buffer >= 0 && i < N && str[i - buffer]; i++)
        ans[i] = str[i - buffer];

    for(;i < N; i++)
        ans[i] = ' ';

    ans[N] = '\0';
    return ans;
}


/**
 * @brief       Centers a wide string to the desired length
 * 
 * @param str   The given string
 * @param N     The given length
 * 
 * @return      The resulting string 
 */
wchar_t* centerWideString(wchar_t* str, int N) {
    int length = wcslen(str);
    int buffer = (N - length) / 2;

    wchar_t* ans = malloc(sizeof(wchar_t) * (N + 1));

    int i;
    for(i = 0; i < buffer; i++)
        ans[i] = ' ';

    for(; i - buffer >= 0 && i < N && str[i - buffer]; i++)
        //We remove tabs from strings as tabs are 1 character but take up 4 spaces
        ans[i] = str[i - buffer] == '\t' ? ' ' : str[i - buffer];

    for(;i < N; i++)
        ans[i] = ' ';

    ans[N] = '\0';
    return ans;
}

/**
 * @brief       Returns a string of length N consisting of only the same character
 * 
 * @param ch    The given character
 * @param N     The length of the desired string
 * 
 * @return      The given string
 */
wchar_t* repeatCharacter(char ch, int N) {
    wchar_t* str = malloc(sizeof(wchar_t) * (N + 1));

    for(int i = 0; i < N; i++)
        str[i] = ch;

    str[N] = '\0';

    return str;
}

/**
 * @brief               Create a Row for the table
 * 
 * @param list          The list of strings
 * @param elementCount  The number of elements
 * @param lengths       The lengths of every cell
 * @param length        The total length of the string
 * 
 * @return              The requested row
 */
wchar_t* createRow(wchar_t** list, int elementCount, int* lengths, int length) {
    wchar_t* result = malloc(sizeof(wchar_t) * (length + 1));

    result[0] =  '|';

    int index = 1;
    for(int i = 0; i < elementCount; i++) {
        result[index++] = ' ';

        wchar_t* centered = centerWideString(list[i], lengths[i]);

        for(int j = 0; j < lengths[i]; j++) {
            result[index++] = centered[j];
        }

        free(centered);

        result[index++] = ' ';
        result[index++] = '|';
    }

    result[length] = '\0';
    return result;
}

/**
 * @brief           Creates an empty row for the table
 * 
 * @param lengths   The array of lengths
 * @param length    The total length of the line
 * 
 * @return          The row
 */
wchar_t* createEmptyRow(int* lengths, int length, int elementCount) {
    wchar_t* result = repeatCharacter(' ', length);

    result[0] = '|';
    int currentLength = 1;
    for(int i = 0; i < elementCount; i++) {
        currentLength += lengths[i] + 2;
        result[currentLength++] = '|';
    }

    return result;
}

/**
 * @brief               Formats the query output into a table
 * 
 * @param output        The array of strings containing the output of the query
 * @param size          The size of the array (will be modified)
 * @param firstLine     The first line of the table
 * @param elementCount  The number of elements per line
 * 
 * @return              The prettified output
 */
wchar_t** queryOutputToTable(char** output, int* size, wchar_t** firstLine, int elementCount) {
    if(*size == 0) {
        return NULL;
    }
    wchar_t*** divided = splitQueryOutput(output, *size, elementCount, ';');

    int lengths[elementCount];
    int length = 1 + 3 * elementCount;
    int lineCount = 2 * *size + 4;
    for(int i = 0; i < elementCount; i++){
        lengths[i] = MAX(wcslen(firstLine[i]), maximumLength(divided[i], *size));
        length += lengths[i];
    }

    wchar_t** result = malloc(sizeof(wchar_t*) * lineCount);

    int s = *size;
    int index = 0;

    result[index++] = repeatCharacter('=', length);
    result[index++] = createRow(firstLine, elementCount, lengths, length);
    result[index++] = repeatCharacter('=', length);
    wchar_t* buffer[elementCount];
    for(int i = 0; i < s; i++) {
        for(int j = 0; j < elementCount; j++) {
            buffer[j] = divided[j][i];
        }

        result[index++] = createRow(buffer, elementCount, lengths, length); 
        result[index++] = createEmptyRow(lengths, length, elementCount);
    }

    result[index++] = repeatCharacter('=', length);

    for(int i = 0; i < lineCount; i++) {
        result[i][0] = result[i][length - 1] = '|';
        result[i][length] = '\0';
    }

    

    for(int i = 0; i < elementCount; i++) {
        for(int j = 0; j < s; j++) {
            free(divided[i][j]);
        }
        free(divided[i]);
    }
        

    free(divided);
    *size = lineCount;
    return result;
}


/**
 * @brief       Splits the button's text into different lines with the given maximum width
 * 
 * @param str   The text to split into lines
 * @param width The max width of the text
 * @param lines The variable in which to store the number of lines
 * 
 * @return      The split text
 */
wchar_t** divideButtonText(char* __restrict__ str, int width, int* __restrict__ lines) {
    wchar_t* wstr = stringToWide(str);
    int length = wcslen(wstr);
    *lines = (length / width) + 1;
    wchar_t** ans = malloc(sizeof(wchar_t*) * *lines);

    for(int i = 0; i < *lines; i++) {
        ans[i] = malloc(sizeof(wchar_t) * (width + 1));
        int j;
        for(j = 0; j < width && wstr[i * width + j]; j++) {
            ans[i][j] = wstr[i * width + j];
        }
        ans[i][j] = '\0';
    }
    free(wstr);
    return ans;
}


/**
 * @brief           Converts the text of a button into its pretty form
 * 
 * @param text      The text of the button
 * @param width     The width of the button
 * @param height    The height of the button
 * 
 * @return          The array of strings to print the button
 */
wchar_t** formatButton(char* text, int width, int height) {
    wchar_t** result = malloc(sizeof(wchar_t*) * height);

    result[0] = repeatCharacter('=', width);
    result[height - 1] = repeatCharacter('=', width);

    int lines;
    wchar_t** textSplit = divideButtonText(text, width - 2, &lines);

    int mid = (height - lines) / 2;

    for(int i = 1; i < height - 1; i++) {
        if(i >= mid && i < mid + lines) {
            result[i] = centerWideString(textSplit[i - mid], width);
        } else {
            result[i] = repeatCharacter((i == 0 || i == height - 1) ? '=' : ' ', width);
        }
        result[i][0] = result[i][width - 1] = '=';
    }

    for(int i = 0; i < lines; i++) {
        free(textSplit[i]);
    }
    free(textSplit);

    return result;
}



/**
 * @brief       Converts a string to a wide string
 * 
 * @param str   The given string
 * 
 * @return      The corresponding wide string
 */
wchar_t* stringToWide(char* str) {
    int length = strlen(str) + 1;

    wchar_t* result = malloc(sizeof(wchar_t) * length);

    mbstowcs (result, str, length);

    return result;
}