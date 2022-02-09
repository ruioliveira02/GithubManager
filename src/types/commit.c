/**
 * @file commit.c
 *
 * File containing the implementation of the type #Commit
 *
 * The type #Commit represents a commit made to a Github repository
 */

#include <glib.h>

#include "types/commit.h"
#include "types/date.h"
#include "types/format.h"
#include "types/repo.h"
#include "types/user.h"

/**
 * @brief Type used to define a commit to a Github repository
 */
struct commit {
    int repo_id;        	///< The id of the repository that the commit was made to
    int author_id;      	///< The id of the author of the commit
	bool author_friend; 	///< The friendship status to the owner of the repo_id
    int committer_id;   	///< The id of the committer
	bool committer_friend;	///< The friendship status to the owner of the repo_id
    Date  commit_at;    	///< The date/time the commit was made at
	int message_len;    	///< The length of the message
    char* message;      	///< The message of the commit
};

/**
 * @brief           Sets the date/time of a #Commit by giving a compressed date
 *
 * @param commit    The given #Commit
 * @param date      The new date/time of the #Commit in compressed form
 */
void setCompressedCommitDate(Commit commit, int date) {
	free(commit->commit_at);
	commit->commit_at = getUncompactedDate(date);
}

/**
 * @brief           Gets the compressed form of the date/time of the #Commit
 *
 * @param commit    The given #Commit
 *
 * @return          The compressed date/time of the #Commit
 */
int getCompressedCommitDate(Commit commit) {
    return getCompactedDate(commit->commit_at);
}

/**
 * @brief           Creates a deep copy of the given #Commit
 *
 * @param commit    The given #Commit
 *
 * @return          The deep copy of the given #Commit
 */
Commit copyCommit(Commit commit) {
    Commit copy=malloc(sizeof(struct commit));
    *copy = *commit;

    copy->commit_at = copyDate(commit->commit_at);
    copy->message = commit->message == NULL ? NULL : strdup(commit->message);

    return copy;
}

/**
 * @brief   Gets the size (in bytes) of the struct commit
 *
 * @return  The size (in bytes) of the struct commit
 */
int getCommitSizeOf() {
    return sizeof(struct commit);
}

/**
 * @brief           Gets the id of the repository the #Commit was made to
 *
 * @param commit    The given #Commit
 *
 * @return          The id of the repository the #Commit was made to
 */
int getCommitRepoId(Commit commit) {
    return commit->repo_id;
}

/**
 * @brief           Gets the id of the author of a #Commit
 *
 * @param commit    The given #Commit
 *
 * @return          The id of the author of the #Commit
 */
int getCommitAuthorId(Commit commit) {
    return commit->author_id;
}

/**
 * @brief           Gets the id of the commiter of a #Commit
 *
 * @param commit    The given #Commit
 *
 * @return          The id of the commiter of the #Commit
 */
int getCommitCommitterId(Commit commit){
    return commit->committer_id;
}

/**
 * @brief           Gets the date/time of a #Commit
 *
 * @remark          The returned date is a copy of the original
 *
 * @param commit    The given #Commit
 *
 * @return          A copy of the date/time of the #Commit
 */
Date getCommitDate(Commit commit){
    if(commit == NULL)
        return NULL;

    return copyDate(commit->commit_at);
}

/**
 * @brief           Gets the message of a #Commit
 *
 * @remark          The returned string is a copy of the original
 *
 * @param commit    The given #Commit
 *
 * @return          A copy of the message of the #Commit
 */
char* getCommitMessage(Commit commit) {
    return commit->message == NULL ? NULL : strdup(commit->message);
}


/**
 * @brief           Sets the id of the repository a #Commit was made to
 *
 * @param commit    The given #Commit
 * @param id        The new id of the repository the #Commit was made to
 */
void setCommitRepoId(Commit commit, int id) {
    commit->repo_id = id;
}

/**
 * @brief           Sets the id of the author of a #Commit
 *
 * @param commit    The given #Commit
 * @param id        The new id of the author of the #Commit
 */
void setCommitAuthorId(Commit commit, int id) {
    commit->author_id = id;
}

/**
 * @brief           Sets the friendship status of the #Commit author with the owner of the #Repo
 * 
 * @param commit    The #Commit
 * @param val       The value to set
 */
void setCommitAuthorFriend(Commit commit, bool val) {
    commit->author_friend = val;
}

/**
 * @brief           Sets the id of the committer of a #Commit
 *
 * @param commit    The given #Commit
 * @param id        The new id of the committer of the #Commit
 */
void setCommitCommitterId(Commit commit, int id){
    commit->committer_id = id;
}

/**
 * @brief           Sets the friendship status of the #Commit committer with the owner of the #Repo
 * 
 * @param commit    The #Commit
 * @param val       The value to set
 */
void setCommitCommitterFriend(Commit commit, bool val) {
    commit->committer_friend = val;
}

/**
 * @brief           Sets the date/time a #Commit was made in
 *
 * @remark          The stored date is a copy of the given one
 *
 * @param commit    The given #Commit
 * @param date      The new date/time of the #Commit
 */
void setCommitDate(Commit commit, Date date) {
    freeDate(commit->commit_at);
    commit->commit_at = copyDate(date);
}

/**
 * @brief           Sets the message of a #Commit
 *
 * @remark          The stored string is a copy of the given one
 *
 * @param commit    The given #Commit
 * @param msg       The new message of the #Commit
 */
void setCommitMessage(Commit commit, char *msg) {
    free(commit->message);
    commit->message=strdup(msg);
}




/**
 * @brief               Gets the #Repo to which the given #Commit was made to
 *
 * @remark              The returned #Repo is a copy of the original
 *
 * @param repos_table   The hashtable containing repositories stored by their id
 * @param commit        The given #Commit
 *
 * @return Repo         The #Repo to which the given #Commit was made to
 */
Repo getRepoOfCommit(GHashTable *repos_table, Commit commit) {
    return copyRepo(g_hash_table_lookup(repos_table, &commit->repo_id));
}

/**
 * @brief               Gets the #User which is the committer of a #Commit
 *
 * @remark              The returned #User is a copy of the original
 *
 * @param users_table   The hashtable containing users stored by their id
 * @param commit        The given #Commit
 *
 * @return Repo         The #User which is the committer of a #Commit
 */
User getCommitter(GHashTable *usersTable, Commit commit) {
    return copyUser(g_hash_table_lookup(usersTable, &commit->committer_id));
}

/**
 * @brief               Gets the #User which is the author of a #Commit
 *
 * @remark              The returned #User is a copy of the original
 *
 * @param users_table   The hashtable containing users stored by their id
 * @param commit        The given #Commit
 *
 * @return Repo         The #User which is the author of a #Commit
 */
User getCommitAuthor(GHashTable *usersTable, Commit commit) {
    return copyUser(g_hash_table_lookup(usersTable, &commit->author_id));
}

/**
 * @brief           Compares the date/time a #Commit was made at and another #Date
 *
 * @param commit    The given #Commit
 * @param date      The given #Date
 *
 * @return -1       if commit_date < date
 * @return 0        if commit_date == date
 * @return 1        if commit_date > date
 */
int compareCommitToDate(Commit commit , Date date) {
    return dateCompare(commit->commit_at,date);
}

/**
 * @brief           Compares the date/time of two #Commit
 *
 * @param a         The first #Commit
 * @param b         The second #Commit
 *
 * @return -1       if a_date < b_date
 * @return 0        if a_date == b_date
 * @return 1        if a_date > b_date
 */
int compareCommitDates(Commit a, Commit b) {
    return dateCompare(a->commit_at, b->commit_at);
}


/**
 * @brief   Creates and initializes a empty #Commit
 *
 * @return  An empty #Commit
 */
Commit initCommit(){
    Commit c  =malloc(sizeof(struct commit));
    c->commit_at = NULL;
    c->message = NULL;
	c->author_friend=false;
	c->committer_friend=false;
    return c;
}




/**
 * @brief           Frees the memory allocated to a #Commit
 *
 * @param commit    The #Commit to be disposed of
 */
void freeCommit(Commit commit) {
    freeDate(commit->commit_at);
    free(commit->message);
    free(commit);
}

/**
 * @brief           Frees the memory allocated to a list of #Commit
 *
 * @param args      A void pointer array containing:    A pointer to a pointer to the first Commit to free
 *                                                      A pointer to the size of the list
 */
void freeCommitList(void *args[]) {
    //Follows the routine prototype
    Commit list = *(Commit*)args[0];
    int N = *(int *)args[1];

    for (int i = 0; i < N; i++)
        freeCommit(list + i);

    free(list);
}

/**
 * @brief Gets the length of a #Commit message.
 *
 * @remark          The function assumes the #Commit exists (not NULL)
 *
 * @param commit    The given #Commit
 *
 * @return          The length of the #Commit message.
 */
int getCommitMessageLenght(Commit commit) {
    if(commit->message)
        return strlen(commit->message);
    else
        return 0;
}

/**
 *
 * @brief   Gets the #Format used for the #Commit throughout the project
 *
 * @return  The #Format of a #Commit
 *
 */
Format getCommitFormat() {
    struct commit commit;

    void* params[] = { &commit.repo_id, &commit.author_id, &commit.committer_id,
    	&commit.commit_at, &commit.message };

    FormatType types[] = { INT, INT, INT, DATE_TIME, STRING_NULL };

	PAIR *lists=malloc(getSizeOfPair());
	addToPair(lists,0,4,&commit.message_len);
	Format f=makeFormat(&commit, params, types, 5, sizeof(struct commit), lists, 1, ';');
	free(lists);
	return f;
}


/**
 *
 * @brief   Gets the compressed #Format used for the #Commit throughout the project
 *
 * @return  The compressed #Format of a #Commit
 *
 */
Format getCompressedCommitFormat() {
    struct commit commit;

    void* params[] = { &commit.repo_id, &commit.author_id,&commit.author_friend,
	&commit.committer_id,&commit.committer_friend, &commit.commit_at,&commit.message_len, &commit.message };

    FormatType types[] = { BINARY_INT, BINARY_INT,BINARY_BOOL, BINARY_INT,BINARY_BOOL, BINARY_DATE_TIME,BINARY_INT, STRING_NULL};
	PAIR *lists=malloc(getSizeOfPair());
	addToPair(lists,0,CCMESSAGE,&commit.message_len);

	Format f=makeFormat(&commit, params, types, 8, sizeof(struct commit), lists, 1, '\0');
	free(lists);
	return f;
}
