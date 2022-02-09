/**
 * @file queries.c
 * 
 * File containing the implementation of the #Query type
 * 
 * The #Query type is used to represent a query regarding the dataset
 */

#include <string.h>

#include "types/catalog.h"
#include "types/date.h"
#include "types/format.h"
#include "types/queries.h"
#include "utils/querySolver.h"
#include "utils/utils.h"


/**
 * @brief Upper bound for the max index of a query
 */
#define QUERY_COUNT 12

/**
 * @brief Upper bound for the max number of arguments of a query
 */
#define MAX_QUERY_ARGS 10

/**
 * @brief Struct used to store a query regarding the dataset
 * 
 * The query structure varies based on its id. Negative id queries refer to instructions to the #GUI, positive id queries refer to dataset queries
 *  
 * | Id            | Description                                           | Arguments                                           | Expected Output |
 * | :---:         | :-----------:                                         | :---------:                                         | :---------------: |
 * |  -25          | Load the Main MenuPage                                |   None                                              |      None                                                                |
 * |  -21          | Terminate the application                             |   None                                              |      None                                                                |
 * |  -20          | Load the Catalog Menu Page                            |   None                                              |      None                                                                |
 * |  -17          | Load the Commits Catalog Page                         |   None                                              |      None                                                                |
 * |  -16          | Load the Repositories Catalog Page                    |   None                                              |      None                                                                |
 * |  -15          | Load the User Catalog Page                            |   None                                              |      None                                                                |
 * | x in [-12,-3] | Load the Page for the query of index x+2              | None                                                |  None                                                                    |
 * |  -2           | Load the Query Menu Page                              |   None                                              |      None                                                                |
 * |  -1           | Invalid query                                         |   None                                              |      None                                                                |
 * |  0            | Empty query                                           |   None                                              |      None                                                                |
 * |  1            | Number of bots, users and organizations               |   None                                              |      Bot: x<br>Organization: y<br>User: z                                |
 * |  2            | Average number of collaborators per repository        |   None                                              |      Average (rounded to 2 decimal places)                               |
 * |  3            | Number of repositories with bots                      |   None                                              |      Total                                                               |
 * |  4            | Average number of commits per user                    |   None                                              |     Average (rounded to 2 decimal places)                                |
 * |  5            | Most active users in date interval                    |   N(users) #Date(starting date) #Date (ending date) |     ID;Login;Commmit_Quantity (N lines)                                  |
 * |  6            | Users with most commits in repositories of a language |   N(users) Language(case insensitive)               |     ID;Login;Commmit_Quantity (N lines)                                  |     
 * |  7            | Inactive repositories counting from date              |   #Date                                             |      Name;Description (one line per repository)                          |
 * |  8            | Most used languages from date                         |  N(languages) #Date                                 |      Language(case insensitive) (N lines)                                |
 * |  9            | Users with most commits in friends' repositories      |   N(users)                                          |      Id;Login (N lines)                                                  |
 * | 10            | Users with longest commit messages per repository     |   N(users)                                          |      Id;Login;Commit_Msg_Size;Repo_Id (N * Number_Of_Repositories lines) |
 */
struct query {
    int id;                         ///< The id of the query
    void* params[MAX_QUERY_ARGS];   ///< The paramaters of the query
};

/**
 * @brief       Creates an empty #Query
 * 
 * @warning     The parameters are left uninitialized
 * 
 * @return      The empty #Query
 */
Query createEmptyQuery() {
    Query result = (Query)malloc(sizeof(struct query));
    result->id = 0;
    return result;
}

/**
 * @brief       Creates a #Query with the given id
 * 
 * @warning     The parameters are left uninitialized
 * 
 * @param id    The id of the #Query
 * 
 * @return      The requested #Query
 */
Query createQueryId(int id) {
    Query result = (Query)malloc(sizeof(struct query));
    result->id = id;
    return result;
}

/**
 * @brief           Executes a given #Query, storing its output to a file
 * 
 * @param stream    The file to output to
 * @param query     The #Query to be executed
 * @param catalog   The catalog containing the dataset
 */
void executeQuery( FILE* stream,Query query,Catalog catalog) {
    //We are disabling this warning because we are only interested in reading the first 32bits of a void* to cast them to int
    //This is intended behaviour, so we are NOT LOSING INFORMATION casting from a 64 bit to a 32 bit type, as the most significant 32 bits
    //are meaningless
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpointer-to-int-cast"

    switch(query->id) {
        case 0:
            //ignore since its an empty command
            break;
        case 1:
            queryOne(catalog, stream);
            break;
        case 2:
            fprintf(stream, "%.2f\n", queryTwo(catalog));
            break;
        case 3:
            fprintf(stream, "%d\n", queryThree(catalog));
            break;
        case 4:
            fprintf(stream, "%.2f\n", queryFour(catalog));
            break;
        case 5:
            queryFive(catalog, (int)(query->params[0]), (Date )query->params[1], (Date )query->params[2], stream);
            break;
        case 6:
            querySix(catalog, (int)(query->params[0]), (char*)(query->params[1]), stream);
            break;
        case 7:
            querySeven(catalog, (Date )query->params[0], stream);
            break;
        case 8:
            queryEight(catalog, (int)query->params[0], (Date )query->params[1], stream);
            break;
        case 9:
            queryNine(catalog, (int)query->params[0], stream);
            break;
        case 10:
            queryTen(catalog, (int)query->params[0], stream);
            break;
        default:
            fprintf(stderr, "executeQuery: not supported id: %d\n", query->id);
            break;
    }

    #pragma GCC diagnostic pop
}



/**
 * @brief       Given a string representing a #Query, stores the corresponding value in the given object
 * 
 * @param str   The query string to parse
 * @param query The #Query in which to store the result
 */
void parseQuery(char* str, Query query) {
    
    char* idString = strsep(&str, " ");
    int id = atoi(idString);

    if (id<11 && id>0) {
        Format format = getQueryFormat(id);
        bool result = readFormat(format, str, query->params);
        query->id = result ? id:-1;
        disposeFormat(format);
    }
    else 
        query->id=-1;
}

/**
 * @brief       Gets the #Format of a #Query based on its id
 * 
 * @param id    The id of the #Query
 * 
 * @return      The #Format of the #Query
 */
Format getQueryFormat(int id) {
    if(id <= 0)
        return NULL;

    FormatType dataTypes[QUERY_COUNT][QUERY_COUNT] = {
        {},
        {},
        {},
        {},
        {},
        {INT, DATE, DATE},
        {INT, STRING},
        {DATE},
        {INT, DATE},
        {INT},
        {INT},
    };

    int parameterCount[QUERY_COUNT] = {0,0,0,0,0,3,2,1,2,1,1};
    
    void* parameters[parameterCount[id]];
    struct query temp;
    for(int i = 0; i < parameterCount[id]; i++) {
        parameters[i] = &temp.params[i];
    }

    Format format = makeFormat(temp.params, parameters, dataTypes[id], parameterCount[id], sizeof(temp.params), NULL, 0, ' ');
    return format;
}

/**
 * @brief       Gets the id of a #Query
 * 
 * @param query The given #Query
 * 
 * @return      The id of the #Query
 */
int getQueryId(Query query) {
    return query->id;
}

/**
 * @brief       Frees the given #Query
 * 
 * @param query The #Query to free
 */
void freeQuery(Query query) {
    Format format = getQueryFormat(query->id);

    if (format) {
        freeFormat(format, query->params);
        disposeFormat(format);
    }
        
    free(query);      
}