/**
 * @file main.c
 * 
 * File containing the main entry point of the application
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

#include "gui/gui.h"
#include "gui/page.h"
#include "io/taskManager.h"
#include "io/taskManager.h"
#include "types/catalog.h"
#include "types/commit.h"
#include "types/format.h"
#include "types/queries.h"
#include "utils/querySolver.h"
#include "utils/utils.h"



/**
 * @brief Tell the compiler we are using the wide version of NCurses
 */
#define _XOPEN_SOURCE_EXTENDED

/**
 * @brief   The format string for the query output files
 */
#define QUERIES_OUT "saida/command%d_output.txt"

/**
 * @brief Upper bound for the maximum length of a query string
 */
#define MAX_QUERY_SIZE 128


/**
 * @brief               Executes a task (i.e. a query)
 *
 * @param taskIndex     The number of the task
 * @param task          The #Query to be executed
 * @param state         The #Catalog passed as void*
 */
void solveTask(int taskIndex, void* task, void* state) {
    Catalog catalog = (Catalog)state;

    char stream[200]="";
    sprintf(stream, QUERIES_OUT, taskIndex + 1);
    if (getQueryId((Query)task)!=-1){
        FILE *output = OPEN_FILE(stream,"w");

        DEBUG_PRINT("Query %d begin\n", taskIndex + 1);

        executeQuery(output, (Query)task,catalog);

        DEBUG_PRINT("Query %d executed\n", taskIndex + 1);

        fclose(output);
    }
    freeQuery((Query)task);
}

/**
 * @brief           Read and executes queries from file
 *
 * @param queryFile The file to read the queries from
 * @param catalog   The #Catalog
 */
void readAndExecuteQueries(const char* queryFile, Catalog catalog) {
    char buffer[MAX_QUERY_SIZE];

    FILE* file = OPEN_FILE(queryFile, "r");

    DEBUG_PRINT("Starting queries\n");

    GArray* queries = g_array_new(FALSE, FALSE, sizeof(Query));

    for (int i = 0; fgets(buffer, MAX_QUERY_SIZE, file); i++){
        Query q = createEmptyQuery();
        if (strcmp(buffer,"\n") != 0) {
            trimNewLine(buffer, strlen(buffer));
            parseQuery(buffer, q);
        }

        g_array_append_val(queries, q);
    }

    executeTasks((void**)queries->data, queries->len,catalog, solveTask, 1);

    DEBUG_PRINT("Finished all queries\n");

    g_array_free(queries, TRUE);
    fclose(file);
}

/**
 * @brief       Application's main entry point
 * 
 *              If no arguments (apart from the name of the program) are passed, then the #GUI is run. Otherwise it grabs
 *              The queries input file and executes them
 * 
 * @param argc  The number of arguments
 * @param argv  The arguments
 * 
 * @return 0    If application ran successfully
 * @return != 0 If an error occurred
 *
 */
int main(int argc, char* argv[]) {
    if(argc == 1) {

        GUI gui = loadGUI();

        runGUI(gui);

        freeGUI(gui);

        if (system("clear && reset") == -1)
            fprintf(stderr, "main: error on call to system\n");
    }
    else if(argc == 2) {
        char *QUERIES_IN = argv[1];
		Catalog catalog = loadCatalog();
    	if (catalog == NULL)
        	catalog = newCatalog(USERS_IN, COMMITS_IN, REPOS_IN, true);
        readAndExecuteQueries(QUERIES_IN,catalog);
        freeCatalog(catalog);
    }
    else{
        printf("Wrong Number of arguments");
    }
    return 0;
}
