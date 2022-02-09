/**
 * @file querySolver.c
 *
 * File containing implementation of the functions that solve parametric queries
 */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

#include "types/catalog.h"
#include "utils/querySolver.h"
#include "utils/utils.h"


/**
 * @brief Struct used to represent pairs of key and respective value in hash table
 */
typedef struct keyValuePair {
    gpointer key; 	///< The key to store
    gpointer value; ///< The value of the respective key
}*KeyValuePair;

/**
 * @brief 			Saves a key and value to a Array as a pair
 *
 * @param key 		The key of the of the pair
 * @param value 	The value of the pair
 *
 * @param arr 		The array to store in
 */
void savePairToArray(gpointer key,gpointer value,gpointer arr) {
    KeyValuePair p = malloc(sizeof(struct keyValuePair));
    p->key = key;
    p->value = value;
    GArray* array = (GArray*)arr;
    g_array_append_val(array,p);
}

/**
 * @brief 			Compares two pairs by their value
 *
 * @param pair1 	The first Pair
 * @param pair2 	The second Pair
 *
 * @return True     If the value of pair1 is smaller than the value of pair2
 * @return False    Otherwise
 */
gint comparePair(gconstpointer pair1,gconstpointer pair2) {
    KeyValuePair p1=*(KeyValuePair*) pair1;
    KeyValuePair p2=*(KeyValuePair*) pair2;
    return GPOINTER_TO_INT(p1->value)<GPOINTER_TO_INT(p2->value);
}

/**
 * @brief 		Transforms a hash table into a list of pairs organized in decreasing order of values
 *
 * @param t 	The hashtable to transform into a list
 *
 * @return 		The array of keys and corresponding values under the pair type
 */
GArray* getHashTableOrganizedyValues(GHashTable* t) {
    GArray* pairs = g_array_new(FALSE,FALSE,sizeof(KeyValuePair));
    g_hash_table_foreach(t,savePairToArray,pairs);
    g_array_sort(pairs,comparePair);
    return pairs;
}

/**
 * @brief 		Frees all pairs inside a Garray and the Garray itself
 *
 * @param g    	the Garray to be freed
 */
void freeKeyValuesGarray(GArray* g) {
    for (int i = 0;i<g->len;i++)
        free(g_array_index(g,KeyValuePair,i));

    g_array_free(g,TRUE);
}

/**
 * @brief 			Solves the second query: returns the average number of collaborators per repo
 *
 * 					Complexity: O(1)
 *
 * @param catalog 	The #Catalog of data
 *
 * @return 			The average. Calculated as the sum of all collaborators divided by the number of repos
 */
double queryTwo(Catalog catalog) {
    return getValueFromQ2(catalog);
}

/**
 * @brief 			Solves the third query: returns the number of repos with commits by a bot user
 *
 * 					Complexity: O(1)
 *
 * @param catalog 	The #Catalog of data
 *
 * @return        	The number of repos with commits by a bot user
 */
int queryThree(Catalog catalog) {
    return getValueFromQ3(catalog);
}

/**
 * @brief 			Answers the fourth query based on the given catalogs
 *
 *
 * 					Complexity: O(1)
 *
 * @param catalog 	The #Catalog of data
 *
 * @return 		  	Average number of #Commit by each collaborator
 */
double queryFour(Catalog catalog) {
    return getValueFromQ4(catalog);
}


/**
 * @brief 				Executes the fifth query (N most active users in given date interval)
 *
 * 						Complexity: O(C + NLogN) average, where C is the number of commits and N is the number of users
 *
 * @param catalog       The given #Catalog
 * @param N             The number of #User to output
 * @param startDate     The start #Date of the interval
 * @param endDate       The end #Date of the interval
 * @param stream        The stream to write the ouput to
 */
void queryFive(Catalog catalog, int N, Date startDate, Date endDate, FILE* stream) {
    //Set time to end of day of final day
    setTime(endDate, 23, 59, 59);

    int differentUsers;
    //Create hash table of users for counting sort
    GHashTable* users = getHashTableOfUserWithCommitsAfter(catalog,startDate,endDate,&differentUsers);
    GArray* pairs = getHashTableOrganizedyValues(users);
    User u = initUser();
    Format comp_user_f = getCompressedUserFormat();
    Lazy user = makeLazy(NULL, 0, comp_user_f, u);

    for (int i = 0; i < MIN(N, differentUsers); i++) {
        KeyValuePair pair=g_array_index(pairs,KeyValuePair,i);
        int id=GPOINTER_TO_INT(pair->key);
        int commitCount = GPOINTER_TO_INT(pair->value);
        fprintf(stream, "%d;", id);
        printUserLoginById(catalog, id, user, stream);
        fprintf(stream,";%d\n", commitCount);
    }
    disposeFormat(comp_user_f);
    freeLazy(user);
    free(u);
    freeKeyValuesGarray(pairs);
    g_hash_table_destroy(users);
}

/**
 * @brief 				Executes the sixth query (N most active users in repos of a given language)
 *
 * 						Complexity: O(C + NLogN) average, where C is the number of commits and N is the number of users
 *
 * @param catalog       The given #Catalog
 * @param N				The number of #User to output
 * @param lang          The given language
 * @param stream        The stream to write the ouput to
 */
void querySix(Catalog catalog, int N, char* lang, FILE* stream) {
    //Create hash table of users for counting sort
    int differentUsers;
    GHashTable* count = getHashTableOfCommitsPerLanguage(catalog,lang,&differentUsers);
    GArray* pairs = getHashTableOrganizedyValues(count);
    Format comp_user_f = getCompressedUserFormat();
    User u = initUser();
    Lazy user = makeLazy(NULL, 0, comp_user_f, u);

    for(int i = 0; i < MIN(N, differentUsers); i++) {
        KeyValuePair pair=g_array_index(pairs,KeyValuePair,i);
        int id=GPOINTER_TO_INT(pair->key);
        int commitCount = GPOINTER_TO_INT(pair->value);
        fprintf(stream, "%d;", id);
        printUserLoginById(catalog, id, user, stream);
        fprintf(stream, ";%d\n", commitCount);
    }
    disposeFormat(comp_user_f);
    freeLazy(user);
    free(u);
    freeKeyValuesGarray(pairs);
    g_hash_table_destroy(count);
}

/**
 * @brief 				Solves the eighth query, i.e., the top N languages present in repos starting from the given date
 *
 *
 *
 * @param catalog       The given #Catalog
 * @param N             The top N wanted
 * @param startDate     The starting date
 * @param stream        The stream to write the ouput to
 */
void queryEight(Catalog catalog, int N, Date  startDate, FILE* stream) {

    GHashTable* languageCount = getHashTableOfNumbersOfAperencesOfALanguageAfter(catalog,startDate);
    int c = g_hash_table_size(languageCount);
    GArray* pairs = getHashTableOrganizedyValues(languageCount);

    for(int i = 0; i < MIN(N, c); i++) {
        KeyValuePair pair=g_array_index(pairs,KeyValuePair,i);
        char* lang = (char*)pair->key;
        if (strcmp(lang, "none") == 0) {
            N++;
            continue;
        }
        else
            fprintf(stream, "%s\n", lang);
    }

    for (int i = 0; i < c; i++)
        free(g_array_index(pairs,KeyValuePair,i)->key);

    g_hash_table_destroy(languageCount);
    freeKeyValuesGarray(pairs);
}

/**
 * @brief 				Solves the ninth query (top N users with most commits in repos owned by their friends)
 *
 * 						Complexity: O(CN + N)
 *
 * @param catalog       The given #Catalog
 * @param N				The number of #User to output
 * @param stream        The stream to write the ouput to
 */

void queryNine(Catalog catalog, int N, FILE* stream) {
    GHashTable* count = getHashTableOfCommitCountInReposOfFriends(catalog);
    GArray* pairs = getHashTableOrganizedyValues(count);
    int c = pairs->len;
    Format comp_user_f = getCompressedUserFormat();
    User u = initUser();
    Lazy user = makeLazy(NULL, 0, comp_user_f, u);
    for(int i = 0; i < MIN(N, c); i++) {
        KeyValuePair pair=g_array_index(pairs,KeyValuePair,i);
        int id=GPOINTER_TO_INT(pair->key);
        fprintf(stream, "%d;",id);
        printUserLoginById(catalog, id, user, stream);
        fprintf(stream, "\n");

    }
    disposeFormat(comp_user_f);
    freeLazy(user);
    free(u);
    g_hash_table_destroy(count);
    freeKeyValuesGarray(pairs);
}

/**
 * @brief 				Solves the 10th query (Top N users with the longest to commit to each repo)
 *
 *  					Complexity: O(R + C + N)
 *
 * @param catalog       The given #Catalog
 * @param N				The number of #User to output per #Repo
 * @param stream        The stream to write the ouput to
 */
void queryTen(Catalog catalog, int N, FILE* stream) {
    int numberOfRepos=getTotalReposCount(catalog);
    Format comp_user_f = getCompressedUserFormat();
    User u = initUser();
    Lazy user = makeLazy(NULL, 0, comp_user_f, u);
    for(int i=0;i<numberOfRepos;i++){
        int repoId;
        GHashTable* UserbyCount= getHashTableOfLengthOfCommitsInArepoByRepoPositionInList(catalog,i,&repoId);
        if (UserbyCount!=NULL){
            GArray* pairs = getHashTableOrganizedyValues(UserbyCount);
            int c = pairs->len;
            for(int i = 0; i < MIN(N, c); i++) {
                KeyValuePair pair=g_array_index(pairs,KeyValuePair,i);
                int userId = GPOINTER_TO_INT(pair->key);
                fprintf(stream, "%d;", userId);
                printUserLoginById(catalog, userId, user, stream);
                fprintf(stream, ";%d;%d\n",GPOINTER_TO_INT(g_hash_table_lookup(UserbyCount,GINT_TO_POINTER(userId))),repoId);
            }
            g_hash_table_destroy(UserbyCount);
            freeKeyValuesGarray(pairs);
        }
    }
    disposeFormat(comp_user_f);
    freeLazy(user);
    free(u);
}
