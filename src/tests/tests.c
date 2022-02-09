/**
 * @file tests.c
 * 
 * File implementing the testing suite of the project
 */

#include <dirent.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "io/taskManager.h"
#include "types/catalog.h"
#include "types/commit.h"
#include "types/format.h"
#include "types/queries.h"
#include "utils/querySolver.h"

/**
 * @brief Relative path of the base directory containing the tests
 * 
 */
#define TEST_DIR "tests"


/**
 * @brief Max execution time of a query in seconds
 * 
 * @note  Execution is not stopped after this time limit has been hit
 * 
 */
#define MAX_TIME 5

/**
 * @brief Maximum number of different lines shown in case of a wrong answer
 * 
 */
#define DIFS_SHOWN 3

/**
 * @brief Code to print in red
 */
#define RED "\x1b[31m"


/**
 * @brief Code to print in green
 */
#define GREEN "\x1b[32m"


/**
 * @brief Code to print in yellow
 */
#define YELLOW "\x1b[33m"


/**
 * @brief Code to reset colour scheme
 */
#define RESET "\x1b[0m"

/**
 * @brief Alias for printf
 * 
 */
#define PRINT(...) printf(__VA_ARGS__)

/**
 * @brief Max length of a query string
 * 
 */
#define MAX_QUERY_SIZE 128

/**
 * @brief               Runs the given #Query
 * 
 * @param c             The given #Catalog
 * @param q             The #Query to run
 * @param queryNumber   The number of the #Query, i.e, this #Query is the queryNumber-th first executed
 * @param path          The base relative path of the test case
 * 
 * @return true         If the #Query is correct and executes within the time limit
 * @return false        If the #Query fails
 */
bool runQuery(Catalog c, Query q, int queryNumber, char* path) {

    PRINT("---------------------------------------------------------------------\n");
    if (getQueryId(q) == -1) {

        PRINT(RED "Error: " RESET "query %d was not parsed correctly. Skipping...\n", queryNumber);
        return true;
    }

    PRINT("Starting query %d (id: %d)\n", queryNumber, getQueryId(q));

    char* temp_file = getQueryFileName();
    FILE *output = OPEN_FILE(temp_file, "w+");
    clock_t start, end;
    start = clock();

    executeQuery(output, q, c);

    end = clock();
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    bool r = cpu_time_used <= 5;
    PRINT("Query completed in %s%f" RESET " seconds\n", r ? GREEN : RED, cpu_time_used);

    //fprintf(aux_output, "%f;", cpu_time_used);

    char expected_output[256] = "";
    sprintf(expected_output, "%s/expected/command%d_output.txt", path, queryNumber);
    FILE* expected = fopen(expected_output, "r");
    bool delete_temp_file = true;

    if (expected != NULL) {

        //PRINT(YELLOW "Error: " RESET "File with expected output \"%s\" not found\n", expected_output);

        //go to beginning of file
        fseek(output, 0, SEEK_SET);

        int difs = 0;
        char *line1 = NULL, *line2 = NULL;
        int line1_size, line2_size,
            read1 = getFileLine(output, &line1, &line1_size),
            read2 = getFileLine(expected, &line2, &line2_size);

        for (int line = 1; read1 || read2; line++)
        {
            if (!read1 || !read2 || strcmp(line1, line2) != 0) {

                if (difs < DIFS_SHOWN) {
                    PRINT(RED "Dif:" RESET " line %d\n", line);
                    PRINT(RED "Output:   " RESET "%s\n", read1 ? line1 : "");
                    PRINT(RED "Expected: " RESET "%s\n", read2 ? line2 : "");
                } else if (difs == DIFS_SHOWN)
                    PRINT("Suppressing remaining dif logs...\n");

                difs++;
            }

            read1 = getFileLine(output, &line1, &line1_size);
            read2 = getFileLine(expected, &line2, &line2_size);
        }

        free(line1);
        free(line2);
        fclose(output);

        PRINT("Total difs found: %s%d" RESET "\n", difs == 0 ? GREEN : RED, difs);

        char temp[256];
        sprintf(temp, "%s/output/command%d_output.txt", path, queryNumber);

        if (difs != 0) {
            PRINT("STATUS: " RED "WRONG ANSWER " RESET "\n");
            r = false;
            delete_temp_file = rename(temp_file, temp);

            if (delete_temp_file)
                PRINT(RED "Error" RESET " saving output file as \"%s/output/command%d_output.txt\"\n", path, queryNumber);
            else
                PRINT("Output file saved as \"%s/output/command%d_output.txt\"\n", path, queryNumber);
        } else {
            remove(temp);
            if (!r)
                PRINT("STATUS: " RED "TIME LIMIT EXCEEDED" RESET "\n");
        } 

        fclose(expected);
    } else {
        fclose(output);

        if (!r)
            PRINT("STATUS: " RED "TIME LIMIT EXCEEDED" RESET "\n");
    }

    if (r)
        PRINT("STATUS: " GREEN "ACCEPTED" RESET "\n");

    if (delete_temp_file)
        remove(temp_file);

    free(temp_file);
    return r;
}

/**
 * @brief           Looks for the occurence of a substring in the given string
 * 
 * @param haystack  The given string
 * @param needle    The given substring
 * 
 * @return          The pointer to the first position of the last occurence of the substring (not null terminated)
 */
char* strrstr(char* haystack, char* needle) {

    int m = strlen(haystack);
    int n = strlen(needle);
    char X[m + 1];
    char Y[n + 1];

    for (int i = 0; i < m; i++)
        X[m - 1 - i] = haystack[i];

    X[m] = '\0';

    for (int i = 0; i < n; i++)
        Y[n - 1 - i] = needle[i];

    Y[n] = '\0';

    char *Z = strstr(X,Y);

    if (Z) {

        int ro = Z - X;
        int lo = ro + n - 1;
        int ol = m - 1 - lo;
        Z = haystack + ol;
    }

    return Z;
}

/**
 * @brief       Gets the memory usage of the application (in MB)
 * 
 * @warning     If an error occured, -1 is returned
 * 
 * @return -1   An error occured
 * @return      The memory usage of the application (in MB)
 */
int getMemUse() {

    char* buf;
    size_t buf_size;
    FILE* stream = open_memstream(&buf, &buf_size);

    if (stream == NULL)
        return -1;

    fflush(stream);
    malloc_info(0, stream);

    char substr[] = "<system type=\"current\" size=\"";
    char* pos = strrstr(buf, substr);
    long aux = -1;
    int ram = 0;

    if (pos != NULL && sscanf(pos + strlen(substr), "%ld", &aux) == 1)
        ram += (aux / (1024 * 1024));

    char substr2[] = "<total type=\"mmap\" count=\"";
    pos = strrstr(buf, substr2);
    long aux2 = 0, trash;

    if (pos != NULL && sscanf(pos + strlen(substr2), "%ld\" size=\"%ld", &trash, &aux2))
        ram += (aux2 / (1024 * 1024));

    fclose(stream);
    free(buf);

    if (aux == -1 && aux2 == -1)
        return -1;

    return ram;
}

/**
 * @brief           Runs the given test
 * 
 * @param path      The base directory of the test
 * @param users     The users.csv file
 * @param commits   The commits.csv file
 * @param repos     The repos.csv file
 * @param queries   The queries.txt file
 */
void runTest(char* path, char* users, char* commits, char* repos, char* queries) {
    PRINT("Starting test \"%s\"\n", path);
    PRINT("Loading catalogs...\n");
    clock_t start_cpu, end_cpu;
    struct timespec start, end;

    start_cpu = clock();
    clock_gettime(CLOCK_REALTIME, &start);

    Catalog catalog = newCatalog(users, commits, repos, true);

    end_cpu = clock();
    clock_gettime(CLOCK_REALTIME, &end);

    double cpu_time_used = ((double) (end_cpu - start_cpu)) / CLOCKS_PER_SEC;
    double elapsed = (end.tv_sec - start.tv_sec);
    elapsed += (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    PRINT("Catalogs loaded, %f seconds elapsed\n", elapsed);
    PRINT("CPU time: %f seconds\n", cpu_time_used);

    int ram = getMemUse(); //in MB
    if (ram == -1)
        PRINT("The program was unable to determine memory allocation\n");
    else {
        PRINT(ram == 0 ? "Less than 1 MB allocated\n" : "%d MB allocated\n", ram);

        if (ram > 8 * 1024) {
            PRINT(YELLOW "Warning:" RESET " more than 4GB of memory allocated\n");
            PRINT("You may want to consider doing your job and optimizing the program for memory usage\n");
        }
    }

    int successes = 0, i = 0;
    char buffer[MAX_QUERY_SIZE];
    FILE* file = OPEN_FILE(queries, "r");

    for (i = 0; fgets(buffer, MAX_QUERY_SIZE, file); i++) {
        Query q = createEmptyQuery();
        if (strcmp(buffer,"\n") != 0) {
            trimNewLine(buffer, strlen(buffer));
            parseQuery(buffer, q);
        }

        successes += runQuery(catalog, q, i + 1, path);
		freeQuery(q);
    }

    fclose(file);

    PRINT("---------------------------------------------------------------------\n");
    PRINT("Total: %s%d/%d\n", successes == i ? GREEN : RED, successes, i);
    PRINT("%s" RESET "\n", successes == i ? "APPROVED" : "FAILED");

    PRINT("Freeing catalogs...\n");
    freeCatalog(catalog);
    PRINT("End of test\n\n");
}

/**
 * @brief           Prints the full path of a file to a string
 *
 * @param dir       The directory
 * @param path      The base directory of the test case
 * @param filename  The name of the file
 * @param dest      The destination buffer
 * @param dest_size The size of the buffer
 * 
 * @return true     If printing successful
 * @return false    If printing unsuccessful
 */
bool printFilePath(struct dirent *dir, char* path, char* filename, char* dest, int dest_size) {
    if (!strcmp(dir->d_name, filename)) {
        sprintf(dest, "%s/", path);
        char temp[255];

        if (dir->d_type == DT_LNK)
        {
            sprintf(temp, "%s/%s", path, filename);
            int path_len = strlen(path), new_size = dest_size - path_len;
            if (readlink(temp, dest + path_len + 1, new_size) == new_size)   //the buffer is too small
                dest[0] = '\0';
        }
        else
            strcpy(dest + strlen(path) + 1, filename);

        return true;
    }

    return false;
}

/**
 * @brief Fetches and executes the given test case
 * 
 * @param path      The relative path to the directory of the test case
 * @return true     If test case was run successfully (does not mean accepted)
 * @return false    If there was an error with the test case (files missing)
 */
bool fetchTest(char* path) {
    char users_in[255] = "", commits_in[255] = "", repos_in[255] = "", queries_in[255] = "";

    char temp[255];
    sprintf(temp, "%s/", path);

    DIR * d = opendir(path); // open the path
    
    if (d == NULL) {
        PRINT(YELLOW "Test \"%s\" not found" RESET "\n\n", path);
        return false;
    }

    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        printFilePath(dir, path, "users.csv", users_in, sizeof(users_in));
        printFilePath(dir, path, "commits.csv", commits_in, sizeof(commits_in));
        printFilePath(dir, path, "repos.csv", repos_in, sizeof(repos_in));
        printFilePath(dir, path, "queries.txt", queries_in, sizeof(queries_in));
    }

    closedir(d);

    if (users_in[0] == '\0' || commits_in[0] == '\0' || repos_in[0] == '\0' || queries_in[0] == '\0') {
        PRINT(YELLOW "Input files for test \"%s\" not found" RESET "\n\n", path);
        return false;
    }

    runTest(path, users_in, commits_in, repos_in, queries_in);
    return true;
}

/**
 * @brief Testing suite's main entry point
 * 
 * @param argc  The number of arguments (at least 2)
 * @param argv  The arguments of the testing suite: relative paths to the folders containing the test cases (or "all" to run all tests)
 *
 * @return 0
 */
int main(int argc, char* argv[]) {

    if (argc == 1) {
        PRINT("No test selected. You can check the available tests inside the folder \"tests\"\n");
        PRINT("To run all available tests, call \"./test all\"\n");
        return 0;
    }

    char temp[255];
    strcpy(temp, TEST_DIR "/");

    if (argc == 2 && strcmp(argv[1], "all") == 0) {
        DIR *d  = opendir(TEST_DIR);
        struct dirent *dir;

        while ((dir = readdir(d)) != NULL) {
            if (strcmp(dir->d_name, ".") && strcmp(dir->d_name, "..")) {
                strcpy(temp + strlen(TEST_DIR) + 1, dir->d_name);
                fetchTest(temp);
            }
        }

        closedir(d);
    }
    else {
        for (int i = 1; i < argc; i++) {
            strcpy(temp + strlen(TEST_DIR) + 1, argv[i]);
            fetchTest(temp);
        }
    }

    return 0;
}