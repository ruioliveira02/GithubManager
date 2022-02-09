/**
 * @file title.c
 * 
 * File containing implementation of the #Title component
 */

#include <ctype.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#include "gui/components/title.h"
#include "utils/table.h"



/**
 * @brief Structure representing the Title component
 */
struct title {
    int size;               ///< The size of the tile (Number of lines it occupies)
    char* lines[SIZE_BIG];  ///< The content of the title
};


/**
 * @brief       Returns the array of strings corresponding to the given character in small size
 * 
 * @param ch    The given character
 * 
 * @return      The requested array of strings
 */
char** charTitleSmall(char ch) {
    char** result = malloc(sizeof(char*) * SIZE_SMALL);

    switch(tolower(ch)) {
        case 'a':
            result[0] = strdup("=====");
            result[1] = strdup("=   =");
            result[2] = strdup("=====");
            result[3] = strdup("=   =");
            result[4] = strdup("=   =");
            break;
        case 'b':
            result[0] = strdup("==== ");
            result[1] = strdup("=   =");
            result[2] = strdup("==== ");
            result[3] = strdup("=   =");
            result[4] = strdup("==== ");
            break;
        case 'c':
            result[0] = strdup("=====");
            result[1] = strdup("=    ");
            result[2] = strdup("=    ");
            result[3] = strdup("=    ");
            result[4] = strdup("=====");
            break;
        case 'd':
            result[0] = strdup("==== ");
            result[1] = strdup("=   =");
            result[2] = strdup("=   =");
            result[3] = strdup("=   =");
            result[4] = strdup("==== ");
            break;
        case 'e':
            result[0] = strdup("=====");
            result[1] = strdup("=    ");
            result[2] = strdup("=====");
            result[3] = strdup("=    ");
            result[4] = strdup("=====");
            break;
        case 'f':
            result[0] = strdup("=====");
            result[1] = strdup("=    ");
            result[2] = strdup("=====");
            result[3] = strdup("=    ");
            result[4] = strdup("=    ");
            break;
        case 'g':
            result[0] = strdup("=====");
            result[1] = strdup("=    ");
            result[2] = strdup("=====");
            result[3] = strdup("=   =");
            result[4] = strdup("=====");
            break;
        case 'h':
            result[0] = strdup("=   =");
            result[1] = strdup("=   =");
            result[2] = strdup("=====");
            result[3] = strdup("=   =");
            result[4] = strdup("=   =");
            break;
        case 'i':
            result[0] = strdup("=====");
            result[1] = strdup("  =  ");
            result[2] = strdup("  =  ");
            result[3] = strdup("  =  ");
            result[4] = strdup("=====");
            break;
        case 'j':
            result[0] = strdup("    =");
            result[1] = strdup("    =");
            result[2] = strdup("=   =");
            result[3] = strdup("=   =");
            result[4] = strdup("=====");
            break;
        case 'k':
            result[0] = strdup("=  ==");
            result[1] = strdup("= =  ");
            result[2] = strdup("==   ");
            result[3] = strdup("= =  ");
            result[4] = strdup("=  ==");
            break;
        case 'l':
            result[0] = strdup("=    ");
            result[1] = strdup("=    ");
            result[2] = strdup("=    ");
            result[3] = strdup("=    ");
            result[4] = strdup("=====");
            break;
        case 'm':
            result[0] = strdup("=   =");
            result[1] = strdup("== ==");
            result[2] = strdup("= = =");
            result[3] = strdup("=   =");
            result[4] = strdup("=   =");
            break;
        case 'n':
            result[0] = strdup("=   =");
            result[1] = strdup("==  =");
            result[2] = strdup("= = =");
            result[3] = strdup("=  ==");
            result[4] = strdup("=   =");
            break;
        case 'o':
            result[0] = strdup("=====");
            result[1] = strdup("=   =");
            result[2] = strdup("=   =");
            result[3] = strdup("=   =");
            result[4] = strdup("=====");
            break;
        case 'p':
            result[0] = strdup("=====");
            result[1] = strdup("=   =");
            result[2] = strdup("=====");
            result[3] = strdup("=    ");
            result[4] = strdup("=    ");
            break;
        case 'q':
            result[0] = strdup("=====");
            result[1] = strdup("=   =");
            result[2] = strdup("=====");
            result[3] = strdup("  == ");
            result[4] = strdup("   ==");
            break;
        case 'r':
            result[0] = strdup("=====");
            result[1] = strdup("=   =");
            result[2] = strdup("=====");
            result[3] = strdup("= =  ");
            result[4] = strdup("=  ==");
            break;
        case 's':
            result[0] = strdup("=====");
            result[1] = strdup("=    ");
            result[2] = strdup("=====");
            result[3] = strdup("    =");
            result[4] = strdup("=====");
            break;
        case 't':
            result[0] = strdup("=====");
            result[1] = strdup("  =  ");
            result[2] = strdup("  =  ");
            result[3] = strdup("  =  ");
            result[4] = strdup("  =  ");
            break;
        case 'u':
            result[0] = strdup("=   =");
            result[1] = strdup("=   =");
            result[2] = strdup("=   =");
            result[3] = strdup("=   =");
            result[4] = strdup("=====");
            break;
        case 'v':
            result[0] = strdup("=   =");
            result[1] = strdup("=   =");
            result[2] = strdup(" = = ");
            result[3] = strdup(" = = ");
            result[4] = strdup("  =  ");
            break;
        case 'w':
            result[0] = strdup("=   =");
            result[1] = strdup("=   =");
            result[2] = strdup("= = =");
            result[3] = strdup("== ==");
            result[4] = strdup("=   =");
            break;
        case 'x':
            result[0] = strdup("=   =");
            result[1] = strdup(" = = ");
            result[2] = strdup("  =  ");
            result[3] = strdup(" = = ");
            result[4] = strdup("=   =");
            break;
        case 'y':
            result[0] = strdup("=   =");
            result[1] = strdup(" = = ");
            result[2] = strdup("  =  ");
            result[3] = strdup("  =  ");
            result[4] = strdup("  =  ");
            break;
        case 'z':
            result[0] = strdup("=====");
            result[1] = strdup("   = ");
            result[2] = strdup("  =  ");
            result[3] = strdup(" =   ");
            result[4] = strdup("=====");
            break;
        case '0':
            result[0] = strdup("=====");
            result[1] = strdup("=   =");
            result[2] = strdup("=   =");
            result[3] = strdup("=   =");
            result[4] = strdup("=====");
            break;
        case '1':
            result[0] = strdup(" ==");
            result[1] = strdup("  =");
            result[2] = strdup("  =");
            result[3] = strdup("  =");
            result[4] = strdup("  =");
            break;
        case '2':
            result[0] = strdup("=====");
            result[1] = strdup("    =");
            result[2] = strdup("=====");
            result[3] = strdup("=    ");
            result[4] = strdup("=====");
            break;
        case '3':
            result[0] = strdup("=====");
            result[1] = strdup("    =");
            result[2] = strdup("=====");
            result[3] = strdup("    =");
            result[4] = strdup("=====");
            break;
        case '4':
            result[0] = strdup("=   =");
            result[1] = strdup("=   =");
            result[2] = strdup("=====");
            result[3] = strdup("    =");
            result[4] = strdup("    =");
            break;
        case '5':
            result[0] = strdup("=====");
            result[1] = strdup("=    ");
            result[2] = strdup("=====");
            result[3] = strdup("    =");
            result[4] = strdup("=====");
            break;
        case '6':
            result[0] = strdup("=====");
            result[1] = strdup("=    ");
            result[2] = strdup("=====");
            result[3] = strdup("=   =");
            result[4] = strdup("=====");
            break;
        case '7':
            result[0] = strdup("=====");
            result[1] = strdup("   = ");
            result[2] = strdup("  =  ");
            result[3] = strdup(" =   ");
            result[4] = strdup("=    ");
            break;
        case '8':
            result[0] = strdup("=====");
            result[1] = strdup("=   =");
            result[2] = strdup("=====");
            result[3] = strdup("=   =");
            result[4] = strdup("=====");
            break;
        case '9':
            result[0] = strdup("=====");
            result[1] = strdup("=   =");
            result[2] = strdup("=====");
            result[3] = strdup("    =");
            result[4] = strdup("=====");
            break;
        case ' ':
            result[0] = strdup("   ");
            result[1] = strdup("   ");
            result[2] = strdup("   ");
            result[3] = strdup("   ");
            result[4] = strdup("   ");
            break;
        case '-':
            result[0] = strdup("     ");
            result[1] = strdup("     ");
            result[2] = strdup("=====");
            result[3] = strdup("     ");
            result[4] = strdup("     ");
            break;
        default:
            result[0] = NULL;
            result[1] = NULL;
            result[2] = NULL;
            result[3] = NULL;
            result[4] = NULL;
            break;
    }

    return result;
}


/**
 * @brief       Returns the array of strings corresponding to the given character in big size
 * 
 * @param ch    The given character
 * 
 * @return      The requested array of strings
 */
char** charTitleBig(char ch) {
    char** result = malloc(sizeof(char*) * SIZE_BIG);

    switch(tolower(ch)) {
        case 'a':
            result[0] = strdup("=========");
            result[1] = strdup("=========");
            result[2] = strdup("===   ===");
            result[3] = strdup("===   ===");
            result[4] = strdup("=========");
            result[5] = strdup("=========");
            result[6] = strdup("===   ===");
            result[7] = strdup("===   ===");
            result[8] = strdup("===   ===");
            break;
        case 'b':
            result[0] = strdup("=======");
            result[1] = strdup("======== ");
            result[2] = strdup("===   ===");
            result[3] = strdup("===   ===");
            result[4] = strdup("=======  ");
            result[5] = strdup("=======  ");
            result[6] = strdup("===   ===");
            result[7] = strdup("======== ");
            result[8] = strdup("=======  ");
            break;
        case 'c':
            result[0] = strdup("=========");
            result[1] = strdup("=========");
            result[2] = strdup("===      ");
            result[3] = strdup("===      ");
            result[4] = strdup("===      ");
            result[5] = strdup("===      ");
            result[6] = strdup("===      ");
            result[7] = strdup("=========");
            result[8] = strdup("=========");
            break;
        case 'd':
            result[0] = strdup("=======  ");
            result[1] = strdup("======== ");
            result[2] = strdup("===   ===");
            result[3] = strdup("===   ===");
            result[4] = strdup("===   ===");
            result[5] = strdup("===   ===");
            result[6] = strdup("===   ===");
            result[7] = strdup("======== ");
            result[8] = strdup("=======  ");
            break;
        case 'e':
            result[0] = strdup("=========");
            result[1] = strdup("=========");
            result[2] = strdup("===      ");
            result[3] = strdup("=========");
            result[4] = strdup("=========");
            result[5] = strdup("=========");
            result[6] = strdup("===      ");
            result[7] = strdup("=========");
            result[8] = strdup("=========");
            break;
        case 'f':
            result[0] = strdup("=========");
            result[1] = strdup("=========");
            result[2] = strdup("===      ");
            result[3] = strdup("===      ");
            result[4] = strdup("=========");
            result[5] = strdup("=========");
            result[6] = strdup("===      ");
            result[7] = strdup("===      ");
            result[8] = strdup("===      ");
            break;
        case 'g':
            result[0] = strdup("=========");
            result[1] = strdup("=========");
            result[2] = strdup("===      ");
            result[3] = strdup("===      ");
            result[4] = strdup("=========");
            result[5] = strdup("=========");
            result[6] = strdup("===   ===");
            result[7] = strdup("=========");
            result[8] = strdup("=========");
            break;
        case 'h':
            result[0] = strdup("===   ===");
            result[1] = strdup("===   ===");
            result[2] = strdup("===   ===");
            result[3] = strdup("=========");
            result[4] = strdup("=========");
            result[5] = strdup("=========");
            result[6] = strdup("===   ===");
            result[7] = strdup("===   ===");
            result[8] = strdup("===   ===");
            break;
        case 'i':
            result[0] = strdup("=========");
            result[1] = strdup("=========");
            result[2] = strdup("   ===   ");
            result[3] = strdup("   ===   ");
            result[4] = strdup("   ===   ");
            result[5] = strdup("   ===   ");
            result[6] = strdup("   ===   ");
            result[7] = strdup("=========");
            result[8] = strdup("=========");
            break;
        case 'j':
            result[0] = strdup("=========");
            result[1] = strdup("=========");
            result[2] = strdup("      ===");
            result[3] = strdup("      ===");
            result[4] = strdup("====  ===");
            result[5] = strdup("====  ===");
            result[6] = strdup("===   ===");
            result[7] = strdup("=========");
            result[8] = strdup("=========");
            break;
        case 'k':
            result[0] = strdup("=========");
            result[1] = strdup("=========");
            result[2] = strdup("===   ===");
            result[3] = strdup("===   ===");
            result[4] = strdup("===   ===");
            result[5] = strdup("===   ===");
            result[6] = strdup("===   ===");
            result[7] = strdup("=========");
            result[8] = strdup("=========");
            break;
        case 'l':
            result[0] = strdup("===      ");
            result[1] = strdup("===      ");
            result[2] = strdup("===      ");
            result[3] = strdup("===      ");
            result[4] = strdup("===      ");
            result[5] = strdup("===      ");
            result[6] = strdup("===      ");
            result[7] = strdup("=========");
            result[8] = strdup("=========");
            break;
        case 'm':
            result[0] = strdup("===   ===");
            result[1] = strdup("===   ===");
            result[2] = strdup("==== ====");
            result[3] = strdup("=========");
            result[4] = strdup("==== ====");
            result[5] = strdup("===   ===");
            result[6] = strdup("===   ===");
            result[7] = strdup("===   ===");
            result[8] = strdup("===   ===");
            break;
        case 'n':
            result[0] = strdup("===   ===");
            result[1] = strdup("====  ===");
            result[2] = strdup("====  ===");
            result[3] = strdup("=== = ===");
            result[4] = strdup("=== =====");
            result[5] = strdup("===  ====");
            result[6] = strdup("===   ===");
            result[7] = strdup("===   ===");
            result[8] = strdup("===   ===");
            break;
        case 'o':
            result[0] = strdup("=========");
            result[1] = strdup("=========");
            result[2] = strdup("===   ===");
            result[3] = strdup("===   ===");
            result[4] = strdup("===   ===");
            result[5] = strdup("===   ===");
            result[6] = strdup("===   ===");
            result[7] = strdup("=========");
            result[8] = strdup("=========");
            break;
        case 'p':
            result[0] = strdup("=========");
            result[1] = strdup("=========");
            result[2] = strdup("===   ===");
            result[3] = strdup("===   ===");
            result[4] = strdup("=========");
            result[5] = strdup("=========");
            result[6] = strdup("===      ");
            result[7] = strdup("===      ");
            result[8] = strdup("===      ");
            break;
        case 'q':
            result[0] = strdup("=========");
            result[1] = strdup("=========");
            result[2] = strdup("===   ===");
            result[3] = strdup("===   ===");
            result[4] = strdup("=========");
            result[5] = strdup("=========");
            result[6] = strdup("   ===   ");
            result[7] = strdup("    ===  ");
            result[8] = strdup("     === ");
            break;
        case 'r':
            result[0] = strdup("=========");
            result[1] = strdup("=========");
            result[2] = strdup("===   ===");
            result[3] = strdup("===   ===");
            result[4] = strdup("=========");
            result[5] = strdup("=========");
            result[6] = strdup("=== ===  ");
            result[7] = strdup("===  === ");
            result[8] = strdup("===   ===");
            break;
        case 's':
            result[0] = strdup("=========");
            result[1] = strdup("=========");
            result[2] = strdup("===      ");
            result[3] = strdup("=========");
            result[4] = strdup("=========");
            result[5] = strdup("=========");
            result[6] = strdup("      ===");
            result[7] = strdup("=========");
            result[8] = strdup("=========");
            break;
        case 't':
            result[0] = strdup("=========");
            result[1] = strdup("=========");
            result[2] = strdup("   ===   ");
            result[3] = strdup("   ===   ");
            result[4] = strdup("   ===   ");
            result[5] = strdup("   ===   ");
            result[6] = strdup("   ===   ");
            result[7] = strdup("   ===   ");
            result[8] = strdup("   ===   ");
            break;
        case 'u':
            result[0] = strdup("===   ===");
            result[1] = strdup("===   ===");
            result[2] = strdup("===   ===");
            result[3] = strdup("===   ===");
            result[4] = strdup("===   ===");
            result[5] = strdup("===   ===");
            result[6] = strdup("===   ===");
            result[7] = strdup("=========");
            result[8] = strdup("=========");
            break;
        case 'v':
            result[0] = strdup("===   ===");
            result[1] = strdup("===   ===");
            result[2] = strdup("===   ===");
            result[3] = strdup(" === === ");
            result[4] = strdup(" === === ");
            result[5] = strdup(" === === ");
            result[6] = strdup("  =====  ");
            result[7] = strdup("   ===   ");
            result[8] = strdup("    =    ");
            break;
        case 'w':
            result[0] = strdup("===   ===");
            result[1] = strdup("===   ===");
            result[2] = strdup("===   ===");
            result[3] = strdup("===   ===");
            result[4] = strdup("===   ===");
            result[5] = strdup("=========");
            result[6] = strdup("==== ====");
            result[7] = strdup("==== ====");
            result[8] = strdup("===   ===");
            break;
        case 'x':
            result[0] = strdup("===   ===");
            result[1] = strdup("===   ===");
            result[2] = strdup(" === === ");
            result[3] = strdup("  =====  ");
            result[4] = strdup("   ===   ");
            result[5] = strdup("  =====  ");
            result[6] = strdup(" === === ");
            result[7] = strdup("===   ===");
            result[8] = strdup("===   ===");
            break;
        case 'y':
            result[0] = strdup("===   ===");
            result[1] = strdup("===   ===");
            result[2] = strdup("===   ===");
            result[3] = strdup(" === === ");
            result[4] = strdup(" === === ");
            result[5] = strdup("  =====  ");
            result[6] = strdup("   ===   ");
            result[7] = strdup("   ===   ");
            result[8] = strdup("   ===   ");
            break;
        case 'z':
            result[0] = strdup("=========");
            result[1] = strdup("=========");
            result[2] = strdup("     === ");
            result[3] = strdup("    ===  ");
            result[4] = strdup("   ===   ");
            result[5] = strdup("  ===    ");
            result[6] = strdup(" ===     ");
            result[7] = strdup("=========");
            result[8] = strdup("=========");
            break;
        case '0':
            result[0] = strdup("=========");
            result[1] = strdup("=========");
            result[2] = strdup("===   ===");
            result[3] = strdup("===   ===");
            result[4] = strdup("===   ===");
            result[5] = strdup("===   ===");
            result[6] = strdup("===   ===");
            result[7] = strdup("=========");
            result[8] = strdup("=========");
            break;
        case '1':
            result[0] = strdup("   ======");
            result[1] = strdup("   ======");
            result[2] = strdup("      ===");
            result[3] = strdup("      ===");
            result[4] = strdup("      ===");
            result[5] = strdup("      ===");
            result[6] = strdup("      ===");
            result[7] = strdup("      ===");
            result[8] = strdup("      ===");
        case '2':
            result[0] = strdup("=========");
            result[1] = strdup("=========");
            result[2] = strdup("      ===");
            result[3] = strdup("=========");
            result[4] = strdup("=========");
            result[5] = strdup("=========");
            result[6] = strdup("===      ");
            result[7] = strdup("=========");
            result[8] = strdup("=========");
            break;
        case '3':
            result[0] = strdup("=========");
            result[1] = strdup("=========");
            result[2] = strdup("      ===");
            result[3] = strdup("=========");
            result[4] = strdup("=========");
            result[5] = strdup("=========");
            result[6] = strdup("      ===");
            result[7] = strdup("=========");
            result[8] = strdup("=========");
            break;
        case '4':
            result[0] = strdup("===   ===");
            result[1] = strdup("===   ===");
            result[2] = strdup("===   ===");
            result[3] = strdup("=========");
            result[4] = strdup("=========");
            result[5] = strdup("      ===");
            result[6] = strdup("      ===");
            result[7] = strdup("      ===");
            result[8] = strdup("      ===");
            break;
        case '5':
            result[0] = strdup("=========");
            result[1] = strdup("=========");
            result[2] = strdup("===      ");
            result[3] = strdup("=========");
            result[4] = strdup("=========");
            result[5] = strdup("=========");
            result[6] = strdup("      ===");
            result[7] = strdup("=========");
            result[8] = strdup("=========");
            break;
        case '6':
            result[0] = strdup("=========");
            result[1] = strdup("=========");
            result[2] = strdup("===      ");
            result[3] = strdup("===      ");
            result[4] = strdup("=========");
            result[5] = strdup("=========");
            result[6] = strdup("===   ===");
            result[7] = strdup("=========");
            result[8] = strdup("=========");
            break;
        case '7':
            result[0] = strdup("=========");
            result[1] = strdup("=========");
            result[2] = strdup("      ===");
            result[3] = strdup("     === ");
            result[4] = strdup("    ===  ");
            result[5] = strdup("   ===   ");
            result[6] = strdup("  ===    ");
            result[7] = strdup(" ===     ");
            result[8] = strdup("===      ");
            break;
        case '8':
            result[0] = strdup("=========");
            result[1] = strdup("=========");
            result[2] = strdup("===   ===");
            result[3] = strdup("=========");
            result[4] = strdup("=========");
            result[5] = strdup("=========");
            result[6] = strdup("===   ===");
            result[7] = strdup("=========");
            result[8] = strdup("=========");
            break;
        case '9':
            result[0] = strdup("=========");
            result[1] = strdup("=========");
            result[2] = strdup("===   ===");
            result[3] = strdup("=========");
            result[4] = strdup("=========");
            result[5] = strdup("      ===");
            result[6] = strdup("      ===");
            result[7] = strdup("=========");
            result[8] = strdup("=========");
            break;
        case ' ':
            result[0] = strdup("     ");
            result[1] = strdup("     ");
            result[2] = strdup("     ");
            result[3] = strdup("     ");
            result[4] = strdup("     ");
            result[5] = strdup("     ");
            result[6] = strdup("     ");
            result[7] = strdup("     ");
            result[8] = strdup("     ");
            break;
        case '-':
            result[0] = strdup("         ");
            result[1] = strdup("         ");
            result[2] = strdup("         ");
            result[3] = strdup("=========");
            result[4] = strdup("=========");
            result[5] = strdup("=========");
            result[6] = strdup("         ");
            result[7] = strdup("         ");
            result[8] = strdup("         ");
            break;
        default:
            result[0] = NULL;
            result[1] = NULL;
            result[2] = NULL;
            result[3] = NULL;
            result[4] = NULL;
            result[5] = NULL;
            result[6] = NULL;
            result[7] = NULL;
            result[8] = NULL;
            break;
    }

    return result;
}


/**
 * @brief       Returns the array of strings corresponding to the given character at the given size
 * 
 * @param ch    The given character
 * @param size  The size of the title
 * 
 * @return      The requested array of strings
 */
char** charTitle(char ch, int size) {
    if(size == 5) {
        return charTitleSmall(ch);
    } else if(size == 9) {
        return charTitleBig(ch);
    } else {
        return NULL;
    }
}

/**
 * @brief       Creates a deep copy of the given #Title
 * 
 * @param title The #Title to copy
 * 
 * @return      The copy of the #Title
 */
Title copyTitle(Title title) {
    Title copy = malloc(sizeof(struct title));

    copy->size = title->size;

    for(int i = 0; i < title->size; i++) {
        copy->lines[i] = strdup(title->lines[i]);
    }

    return copy;
}


/**
 * @brief Creates a #Title component corresponding to the given string
 * 
 * @param str   The given string
 * @param size  The size of #Title (either 5 or 9)
 * 
 * @return      The corresponding #Title (NULL if undefined size)
 */
Title createTitle(char* str, int size) {
    if(size != 5 && size != 9)
        return NULL;

    Title title = malloc(sizeof(struct title));
    title->size = size;
    int length = strlen(str);

    for(int i = 0; i < size; i++) {
        title->lines[i] = malloc(sizeof(char) * (length * 2 * size + 1));
        title->lines[i][0] = '\0';
    }
        
    for(int i = 0; i < length; i++) {
        char** temp = charTitle(str[i], size);

        for(int j = 0; j < size; j++) {
            strcat(title->lines[j], temp[j]);
            free(temp[j]);

            if(i != length - 1)
                strcat(title->lines[j], " ");
        }
            
        free(temp);
    }

    return title;
}

/**
 * @brief       Renders the given #Title component to the screen
 * 
 * @param title The given #Title
 * @param x     The x coordinate of the cursor at the start of the rendering
 * @param y     The y coordinate of the cursor at the start of the rendering
 * @param width The width of the cell the #Title will be displayed in
 */
void renderTitle(Title title, int* x, int* y, int width) {

    for(int i = 0; i < title->size; i++) {
        char* text = centerString(title->lines[i], width);

        mvprintw(*y, *x, text);
        (*y)++;
        free(text);
    }
}

/**
 * @brief       Frees the allocated memory for a #Title component
 * 
 * @param title The given #Title
 */
void freeTitle(Title title) {
    for(int i = 0; i < title->size; i++)
        free(title->lines[i]);

    free(title);
}