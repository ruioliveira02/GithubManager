/**
 * @file repo.c
 *
 * File containing the implementation of the #Repo type.
 *
 * The #Repo type is used to store a Github repository
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <glib.h>

#include "types/date.h"
#include "types/format.h"
#include "types/repo.h"
#include "types/user.h"
#include "utils/utils.h"

/**
 * @brief Type used to define a Github repository
 */
struct repo {
    int id;                     ///< The id of the repository
    int owner_id;               ///< The id of the owner of the repository
	int full_name_len;          ///< The length of the full name of the repository
    char* full_name;            ///< The name of the repository
    int license_len;            ///< The length of the license string
	char* license;              ///< The license of the repository
    bool has_wiki;              ///< Whether or not the repository has a wiki
	int description_len;        ///< The length of the description of the repository
    char* description;          ///< The description of the repository
	int language_len;           ///< The length of the language of the repository
    char* language;             ///< The language of the repository
	int default_branch_len;     ///< The length of the name of default branch
    char* default_branch;       ///< The default branch of the repository
    Date  created_at;           ///< The date/time the repository was created at
    Date  updated_at;           ///< The date/time the repository was last updated
	Date  actually_updated_at;  ///< The date/time of the last commit to the repository
    int forks_count;            ///< The ammount of forks of the repository
    int open_issues;            ///< The ammount of open issues of the repository
    int stargazers_count;       ///< The ammount of stargazers of the repository
    int size;                   ///< The size of the repository
};

/**
 * @brief       Sets the date/time of creation of a #Repo by receiving a compressed date
 *
 * @param repo  The given #Repo
 * @param date  The new date/time of creation of the #Repo (in compressed form)
 */
void setRepoCreationFromComp(Repo repo, int date) {
	free(repo->created_at);
	repo->created_at = getUncompactedDate(date);
}

/**
 * @brief       Sets the date/time of the last update of a #Repo by receiving a compressed date
 *
 * @param repo  The given #Repo
 * @param date  The new date/time of the last update of the #Repo (in compressed form)
 */
void setRepoUpdatedFromComp(Repo repo, int date) {
	free(repo->updated_at);
	repo->updated_at=getUncompactedDate(date);
}

/**
 * @brief       Set the date/time of the last commit to a #Repo by receiving a compressed date
 *
 * @param repo  The given #Repo
 * @param date  The new date/time of the last commit to the #Repo
 */
void setRepoLastCommitDateFromComp(Repo repo, int date) {
	free(repo->actually_updated_at);
	repo->actually_updated_at=getUncompactedDate(date);
}

/**
 * @brief       Gets the length of the full name of a #Repo
 *
 * @param repo  The given #Repo
 *
 * @return      The length of the full name of the #Repo
 */
int getRepoNameLength(Repo repo) {
    return repo->full_name_len;
}

/**
 * @brief       Gets the length of the description of a #Repo
 *
 * @param repo  The given #Repo
 *
 * @return      The length of the description of the #Repo
 */
int getRepoDescriptionLength(Repo repo) {
    return repo->description_len;
}

/**
 * @brief       Gets the length of the language of a #Repo
 *
 * @param repo  The given #Repo
 *
 * @return      The length of the language of the #Repo
 */
int getRepoLanguageLength(Repo repo) {
    return repo->language_len;
}

/**
 * @brief       Gets the length of the name of the default branch of a #Repo
 *
 * @param repo  The given #Repo
 *
 * @return      The length of the name of the default branch of the #Repo
 */
int getRepoDefaultBranchLength(Repo repo) {
    return repo->default_branch_len;
}

/**
 * @brief       Gets the date/time of the last #Commit to the #Repo
 *
 * @remark      The #Date object returned is a copy of the original
 *
 * @param repo  The given #Repo
 *
 * @return      The date/time of the last #Commit to the #Repo
 */
Date getRepoLastCommitDate(Repo repo) {
    return copyDate(repo->actually_updated_at);
}

/**
 * @brief       Sets the Repo Last Commit Date object
 *
 * @remark      The #Date object stored is a copy of the given
 *
 * @param repo  The given #Repo
 * @param date  The new date/time of the last #Commit to the #Repo
 */
void setRepoLastCommitDate(Repo repo, Date date) {
	free(repo->actually_updated_at);
	repo->actually_updated_at = copyDate(date);
}

/**
 * @brief   Gets the size of the struct of a repo
 *
 * @return  The size of the struct of a repo
 */
int getRepoSizeOf() {
    return sizeof(struct repo);
}

/**
 * @brief       Creates a deep copy of a #Repo
 *
 * @param repo  The given #Repo
 *
 * @return      The deep copy of the #Repo
 */
Repo copyRepo(Repo repo) {
    if (repo==NULL)
        return NULL;

    Repo copy = malloc(sizeof(struct repo));
    *copy = *repo;

    copy->full_name = strdup(repo->full_name);
    copy->license = strdup(repo->license);
    copy->description = repo->description == NULL ? NULL : strdup(repo->description);
    copy->language = strdup(repo->language);
    copy->default_branch = strdup(repo->default_branch);
    copy->created_at = copyDate(repo->created_at);
    copy->updated_at = copyDate(repo->updated_at);
	copy->actually_updated_at = copyDate(repo->actually_updated_at);

    return copy;
}

/**
 * @brief   Creates an empty #Repo
 *
 * @return  The empty #Repo
 */
Repo initRepo() {
    Repo r = malloc(sizeof(struct repo));
	r->full_name_len = 0;
    r->full_name = NULL;
	r->license_len = 0;
    r->license = NULL;
	r->description_len = 0;
    r->description = NULL;
	r->language_len = 0;
    r->language = NULL;
	r->default_branch_len = 0;
    r->default_branch = NULL;
    r->created_at = NULL;
    r->updated_at = NULL;
	r->actually_updated_at = NULL;
    return r;
}

/**
 * @brief       Gets the id of a #Repo
 *
 * @param repo  The given #Repo
 *
 * @return      The id of the #Repo
 */
int getRepoId(Repo repo) {
    return repo->id;
}

/**
 * @brief       Gets the id of the owner of a #Repo
 *
 * @param repo  The given #Repo
 *
 * @return      The id of the owner of the #Repo
 */
int getRepoOwnerId(Repo repo) {
    return repo->owner_id;
}

/**
 * @brief       Gets the full name of a #Repo
 *
 * @remark      The returned string is a copy of the one stored in the #Repo
 *
 * @param repo  The given #Repo
 *
 * @return      A copy of the name of the #Repo
 */
char* getRepoFullName(Repo repo) {
    return strdup(repo->full_name);
}

/**
 * @brief       Gets the license of a #Repo
 *
 * @remark      The returned string is a copy of the one stored in the #Repo
 *
 * @param repo  The given #Repo
 *
 * @return      A copy of the license of the #Repo
 */
char* getRepolicense(Repo repo) {
    return strdup(repo->license);
}

/**
 * @brief           Gets whether or not a #Repo has a wiki
 *
 * @param repo      The given #Repo
 *
 * @return true     If the #Repo has a wiki
 * @return false    If the #Repo doesn't have a wiki
 */
bool getRepoHasWiki(Repo repo) {
    return repo->has_wiki;
}

/**
 * @brief       Gets the description of a #Repo
 *
 * @remark      The returned string is a copy of the one stored in the #Repo
 *
 * @param repo  The given #Repo
 *
 * @return      The description of a #Repo
 */
char* getRepoDescription(Repo repo) {
    return repo->description == NULL ? NULL : strdup(repo->description);
}

/**
 * @brief       Gets the languange of a #Repo
 *
 * @remark      The returned string is a copy of the one stored in the #Repo
 *
 * @param repo  The given #Repo
 *
 * @return      The language of the #Repo
 */
char* getRepoLanguage(Repo repo) {
    return strdup(repo->language);
}

/**
 * @brief       Gets the default branch of a #Repo
 *
 * @remark      The returned string is a copy of the one stored in the #Repo
 *
 * @param repo  The given #Repo
 *
 * @return      The default branch of a #Repo
 */
char* getRepoDefaultBranch(Repo repo) {
    return strdup(repo->default_branch);
}

/**
 * @brief       Gets the date at which the #Repo was created at
 *
 * @remark      The returned #Date is a copy of the one stored by the #Repo
 *
 * @param repo  The given #Repo
 *
 * @return      The date/time of creation of the #Repo
 */
Date getRepoCreationDate(Repo repo) {
    return copyDate(repo->created_at);
}

/**
 * @brief       Gets the #Date when the #Repo was last updated at
 *
 * @remark      The returned #Date is a copy of the one stored by the #Repo
 *
 * @param repo  The given #Repo
 *
 * @return      The #Date the repo was updated at
 */
Date getRepoUpdatedAt(Repo repo) {
    return copyDate(repo->updated_at);
}

/**
 * @brief       Gets the number of forks of a #Repo
 *
 * @param repo  The given #Repo
 *
 * @return      The number of forks of the #Repo
 */
int getRepoForksCount(Repo repo){
    return repo->forks_count;
}
/**
 * @brief       Gets the number of open issues on a #Repo
 *
 * @param repo  The given #Repo
 *
 * @return      The number of open issues on a #Repo
 */
int getRepoOpenIssues(Repo repo) {
    return repo->open_issues;
}

/**
 * @brief       Gets the number of stargazers of a #Repo
 *
 * @param repo  The given #Repo
 *
 * @return      The number of stargazers of a #Repo
 */
int getRepoStargazersCount(Repo repo) {
    return repo->stargazers_count;
}

/**
 * @brief       Gets the size property of a #Repo
 *
 * @warning     This is not the size of the struct a #Repo points to. It is the size of the repository on Github
 *
 * @param repo  The given #Repo
 *
 * @return      The size of the #Repo
 */
int getRepoSize(Repo repo) {
    return repo->size;
}



/**
 * @brief       Sets the id of a #Repo
 *
 * @param repo  The given #Repo
 * @param id    The new id of the #Repo
 */
void setRepoId(Repo repo, int id) {
    repo->id = id;
}

/**
 * @brief       Sets the id of the owner of a #Repo
 *
 * @param repo  The given #Repo
 * @param id    The new id of owner of the #Repo
 */
void setRepoOwnerId(Repo repo, int id) {
    repo->owner_id = id;
}

/**
 * @brief       Sets the name of a #Repo.
 *
 * @remark      The stored string will be a copy of the given one
 *
 * @param repo  The given #Repo
 * @param name  The new name of the #Repo
 */
void setRepoName(Repo repo, char* name) {
    free(repo->full_name);
    repo->full_name = strdup(name);
    repo->full_name_len = strlen(name);
}

/**
 * @brief       Sets the license of a #Repo.
 *
 * @remark      The stored string will be a copy of the given one
 *
 * @param repo  The given #Repo
 * @param name  The new license of the #Repo
 */
void setRepoLicense(Repo repo,char* license) {
    free(repo->license);
    repo->license = strdup(license);
    repo->license_len = strlen(license);
}

/**
 * @brief Sets whether or not a #Repo has a wiki
 *
 * @param repo      The given #Repo
 * @param hasWiki   Whether or not the #Repo has a wiki
 */
void setRepoHasWiki(Repo repo, bool hasWiki) {
    repo->has_wiki = hasWiki;
}

/**
 * @brief       Sets the description of a #Repo.
 *
 * @remark      The stored string will be a copy of the given one
 *
 * @param repo  The given #Repo
 * @param name  The new description of the #Repo
 */
void setRepoDescription(Repo repo, char* description) {
    free(repo->description);
    repo->description = strdup(description);
    repo->description_len = strlen(description);
}

/**
 * @brief           Sets the language of a #Repo.
 *
 * @remark          The stored string will be a copy of the given one
 *
 * @param repo      The given #Repo
 * @param language  The new language of the #Repo
 */
void setRepolanguage(Repo repo, char* language) {
    free(repo->language);
    repo->language = strdup(language);
    repo->language_len = strlen(language);
}

/**
 * @brief       Converts the language of the #Repo to a lower case string
 *
 * @param repo  The given #Repo
 */
void repolanguageToLower(Repo repo) {
    toLower(repo->language);
}

/**
 * @brief           Sets the default branch of a #Repo.
 *
 * @remark          The stored string will be a copy of the given one
 *
 * @param repo      The given #Repo
 * @param branch    The new default branch of the #Repo
 */
void setRepoDefaultBranch(Repo repo, char* branch) {
    free(repo->default_branch);
    repo->default_branch = strdup(branch);
    repo->default_branch_len = strlen(branch);
}

/**
 * @brief               Sets the #Date of creation of a #Repo
 *
 * @remark              The stored #Date will be a copy of the given one
 *
 * @param repo          The given #Repo
 * @param creationDate  The new #Date of creation of the #Repo
 */
void setRepoCreationDate(Repo repo, Date creationDate) {
    freeDate(repo->created_at);
    repo->created_at = copyDate(creationDate);
}

/**
 * @brief               Sets the #Date of the last update of a #Repo
 *
 * @remark              The stored #Date will be a copy of the given one
 *
 * @param repo          The given #Repo
 * @param updateDate    The new #Date of of the last update of the #Repo
 */
void setRepoUpdatedAt(Repo repo, Date updateDate) {
    freeDate(repo->updated_at);
    repo->updated_at = copyDate(updateDate);
}

/**
 * @brief       Sets the number of forks of a #Repo
 *
 * @param repo  The given #Repo
 * @param forks The new number of forks of the #Repo
 */
void setRepoForksCount(Repo repo, int forks) {
    repo->forks_count = forks;
}

/**
 * @brief           Sets the number of open issues in a #Repo
 *
 * @param repo      The given #Repo
 * @param issues    The new number of open issues in the #Repo
 */
void setRepoOpenIssues(Repo repo, int issues) {
    repo->open_issues = issues;
}

/**
 * @brief               Sets the number of stargazers of a #Repo
 *
 * @param repo          The given #Repo
 * @param stargazers    The new number of stargazers of the #Repo
 */
void setRepoStargazersCount(Repo repo, int stargazers) {
    repo->stargazers_count = stargazers;
}

/**
 * @brief       Sets the size property of a #Repo
 *
 * @warning     This is not the size of the struct a #Repo points to. It is the size of the repository on Github
 *
 * @param repo  The given #Repo
 * @param size  The new size of the #Repo
 */
void setRepoSize(Repo repo, int size) {
    repo->size = size;
}


/**
 * @brief               Gets the #User who own a #Repo
 *
 * @remark              The returned #User is a copy
 *
 * @param users_table   HashTable of #User stored by id
 * @param repo          The given #Repo
 *
 * @return              The #User who owns the #Repo (same id as the owner id of the #Repo)
 */
User getReposOwner(GHashTable* users_table, Repo repo) {
    return copyUser(g_hash_table_lookup(users_table, &repo->owner_id));
}

/**
 * @brief               Adds a #Repo to a hashtable of #Repo indexed by their id
 *
 * @param repos_table   Hashtable in which to add the #Repo
 * @param repo          The #Repo to add
 */
void addRepoToHash(GHashTable* repos_table, Repo repo) {
    g_hash_table_insert(repos_table, &repo->id, (gpointer)repo);
}

/**
 * @brief   Compares two #Repo by their creation #Date
 *
 * @param a The first #Repo
 * @param b The second #Repo
 *
 * @return -1   If the first #Repo was created before
 * @return 0    If they were created at the same time
 * @return 1    If the second #Repo was created after
 */
int compareRepoByCreation(Repo a, Repo b) {
    return dateCompare(a->created_at, b->created_at);
}


/**
 * @brief           Frees the memory allocated to a #Repo
 *
 * @param repo      The #Repo to free
 */
void freeRepo(Repo repo)
{
    if (repo){
        free(repo->full_name);
        free(repo->license);
        if (repo->description) free(repo->description);
        free(repo->language);
        free(repo->default_branch);
        freeDate(repo->created_at);
        freeDate(repo->updated_at);
		freeDate(repo->actually_updated_at);
        free(repo);
    }
}

/**
 * @brief           Frees the content of a #Repo, but not the #Repo itself
 *
 * @param repo      The #Repo to free the content of
 */
void freeRepoContent(Repo repo){
    if(repo) {
        free(repo->full_name);
        free(repo->license);
        free(repo->description);
        free(repo->language);
        free(repo->default_branch);
        freeDate(repo->created_at);
        freeDate(repo->updated_at);
        freeDate(repo->actually_updated_at);
        repo->full_name=NULL;
        repo->license=NULL;
        repo->description=NULL;
        repo->language=NULL;
        repo->default_branch=NULL;
        repo->created_at=NULL;
        repo->updated_at=NULL;
        repo->actually_updated_at=NULL;
    }
}

/**
 * @brief           Frees the memory allocated to the list of #Repo
 *
 *                  Follows the routine prototype
 *
 * @param args      A void pointer array containing:    A pointer to a pointer to the first Repo to free
 *                                                      A pointer to the size of the list
 */
void freeRepoList(void *args[]) {
    Repo list = *(Repo *)args[0];
    int N = *(int *)args[1];

    for (int i = 0; i < N; i++)
        freeRepo(list + i);

    free(list);
}


/**
 * @brief   Gets the uncompressed #Format used for the struct to store the #Repo throughout the project
 *
 * @return  The #Repo #Format
 */
Format getRepoFormat() {
    struct repo repo;

    void* params[14] = { &repo.id, &repo.owner_id, &repo.full_name, &repo.license, &repo.has_wiki,
    	&repo.description, &repo.language, &repo.default_branch, &repo.created_at,&repo.updated_at,
    	&repo.forks_count, &repo.open_issues, &repo.stargazers_count, &repo.size };

    FormatType types[14] = { INT, INT, STRING, STRING, BOOL, STRING_NULL, STRING, STRING, DATE_TIME, DATE_TIME, INT, INT, INT, INT };

   	PAIR *lists=malloc(5*getSizeOfPair());
	addToPair(lists,0,2,&repo.full_name_len);
	addToPair(lists,1,3,&repo.license_len);
	addToPair(lists,2,5,&repo.description_len);
	addToPair(lists,3,6,&repo.language_len);
	addToPair(lists,4,7,&repo.default_branch_len);

	Format f=makeFormat(&repo, params, types, 14, sizeof(struct repo), lists, 5, ';');
	free(lists);
	return f;
}

/**
 * @brief   Gets the compressed #Format used for the struct to store the #Repo throughout the project
 *
 * @return  The compressed #Repo #Format
 */
Format getCompressedRepoFormat() {
    struct repo repo;
    void* params[] = {
        &repo.id,&repo.owner_id,&repo.actually_updated_at,&repo.language_len,&repo.language,&repo.description_len,&repo.description,
        /*after this point is all unused*/&repo.has_wiki,&repo.default_branch_len,&repo.default_branch,&repo.created_at,&repo.updated_at,&repo.forks_count,
        &repo.open_issues,&repo.stargazers_count,&repo.size,&repo.full_name_len,&repo.full_name,&repo.license_len,&repo.license
    };

    FormatType types[] = { BINARY_INT,BINARY_INT,BINARY_DATE_TIME,BINARY_INT,STRING,BINARY_INT,STRING,
        /*after this point is all unused*/BINARY_BOOL,BINARY_INT,STRING,BINARY_DATE_TIME,BINARY_DATE_TIME,BINARY_INT,BINARY_INT,BINARY_INT,
        BINARY_INT,BINARY_INT,STRING,BINARY_INT,STRING
    };
	PAIR *lists=malloc(5*getSizeOfPair());
	addToPair(lists,0,CRLANGUAGE,&repo.language_len);
	addToPair(lists,1,CRDESCRIPTION,&repo.description_len);
	addToPair(lists,2,CRDEFAULT_BRANCH, &repo.default_branch_len);
	addToPair(lists,3,CRFULL_NAME, &repo.full_name_len);
	addToPair(lists,4,CRLICENSE, &repo.license_len);

	Format f=makeFormat(&repo, params, types, 20, sizeof(struct repo), lists, 5, '\0');
	free(lists);
	return f;
}
