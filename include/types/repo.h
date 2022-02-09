/**
 * @file repo.h
 * 
 * File containing declaration of the #Repo type
 */


#ifndef _REPO_H_
/**
 * @brief Include guard
 */
#define _REPO_H_

#include <glib.h>
#include <stdbool.h>

#include "utils/utils.h"
#include "types/user.h"
#include "types/format.h"


/**
 * @brief The structure representing a Github repository
 * 
 */
typedef struct repo *Repo;

/**
 * @brief The data types stored in a #Repo
 * 
 */
typedef enum crepo{
    CRID=0,                     ///< The id
    CROWNER_ID=1,               ///< The id of the owner
	CRACTUALLY_UPDATED_AT=2,    ///< The #Date of the last #Commit
	CRLANGUAGE_LEN=3,           ///< The length of the language name
    CRLANGUAGE=4,               ///< The language
	CRDESCRIPTION_LEN=5,        ///< The length of the description
    CRDESCRIPTION=6,            ///< The description
    CRHAS_WIKI=7,               ///< Whether or not it has a wiki
	CRDEFAULT_BRANCH_LEN=8,     ///< The length of the name of the default branch
    CRDEFAULT_BRANCH=9,         ///< The default branch
    CRCREATED_AT=10,            ///< The #Date of creation
    CRUPDATED_AT=11,            ///< The #Date of the last update
    CRFORKS_COUNT=12,           ///< The number of forks
    CROPEN_ISSUES=13,           ///< The number of open issues
    CRSTARGAZERS_COUNT=14,      ///< The number of stargazers
    CRSIZE=15,                  ///< The size
	CRFULL_NAME_LEN=16,         ///< The length of the name
    CRFULL_NAME=17,             ///< The name
    CRLICENSE_LEN=18,           ///< The length of the name of the license
	CRLICENSE=19                ///< The license
}CREPO;

Repo  initRepo();
Repo  copyRepo(Repo );
int getRepoSizeOf();

int getRepoId(Repo );
int getRepoOwnerId(Repo );
char* getRepoFullName(Repo );
char* getRepolicense(Repo );
bool getRepoHasWiki(Repo );
char* getRepoDescription(Repo );
char* getRepoLanguage(Repo );
char* getRepoDefaultBranch(Repo );
Date  getRepoCreationDate(Repo );
Date  getRepoUpdatedAt(Repo );
Date  getRepoLastCommitDate(Repo );
int getRepoForksCount(Repo );
int getRepoOpenIssues(Repo );
int getRepoStargazersCount(Repo );
int getRepoSize(Repo );
void setRepoId(Repo ,int );
void setRepoOwnerId(Repo ,int );
void setRepoName(Repo ,char* );
void setRepoLicense(Repo ,char* );
void setRepoHasWiki(Repo ,bool );
void setRepoDescription(Repo ,char*);
void setRepolanguage(Repo ,char*);
void repolanguageToLower(Repo);
void setRepoDefaultBranch(Repo ,char*);
void setRepoCreationDate(Repo ,Date  );
void setRepoUpdatedAt(Repo ,Date  );
void setRepoLastCommitDate(Repo , Date  );
void setRepoForksCount(Repo ,int );
void setRepoOpenIssues(Repo ,int );
void setRepoStargazersCount(Repo ,int );
void setRepoSize(Repo ,int );
User getReposOwner(GHashTable *,Repo );
void addRepoToHash(GHashTable *,Repo );

int compareRepoByCreation(Repo, Repo);

void freeRepo(Repo );
void freeRepoList(void* args[]);

Format getRepoFormat();
Format getCompressedRepoFormat();

void setRepoCreationFromComp(Repo,int);
void setRepoUpdatedFromComp(Repo,int);
void setRepoLastCommitDateFromComp(Repo,int);
int getRepoNameLength(Repo);
int getRepoDescriptionLength(Repo);
int getRepoLanguageLength(Repo);
int getRepoDefaultBranchLength(Repo);
Date getRepoLastCommitDate(Repo);
void setRepoLastCommitDate(Repo,Date);
void freeRepoContent(Repo);


#endif