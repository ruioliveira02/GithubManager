/**
 * @file finder.c
 *
 * File implementing the function which searches a file for a substring
 *
 */
#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "io/finder.h"
#include "utils/utils.h"

/**
 * @brief           This function will find all lines where a substring occurs and copy only those to a new file
 *
 * @param infile    The path to the input file
 * @param outfile   The path to the output file
 * @param substring The substring to search for
 */
void finder(char* infile, char* outfile, char* substring) {
    FILE* input = OPEN_FILE(infile,"r");
    FILE* output = OPEN_FILE(outfile,"w");
    char *buf = NULL;
    int buf_size = 0;

    while (getFileLine(input, &buf, &buf_size)) {
        if (strcasestr(buf,substring))
            fprintf(output,"%s\n",buf);
    }

    if (buf)
        free(buf);

    fclose(input);
    fclose(output);
}
