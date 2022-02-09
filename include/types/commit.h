/**
 * @file commit.c
 * 
 * File containing declaration of the #Commit type
 */

#ifndef _COMMIT_H_

/**
 * @brief Include guard
 */
#define _COMMIT_H_

#include <stdbool.h>
#include <glib.h>

#include "utils/utils.h"
#include "types/repo.h"
#include "types/user.h"
#include "types/format.h"



int getCommitSizeOf();

/**
 * @brief The struct representing a commit to a Github repository
 * 
 */
typedef struct commit *Commit;

/**
 * @brief The data stored in the #Commit type
 * 
 */
typedef enum ccommit{
	CCREPO_ID=0,			///< The id of the #Repo
	CCAUTHOR_ID=1,			///< The id of the author
	CCAUTHOR_FRIEND=2,		///< The author's friends
	CCCOMMITTER_ID=3,		///< The id of the committer
	CCCOMMITTER_FRIEND=4,   ///< The committer's friends
    CCCOMMIT_AT=5,			///< The #Date of the #Commit
	CCMESSAGE_LEN=6,		///< The length of the message
	CCMESSAGE=7				///< The message
}CCOMMIT;



Commit copyCommit(Commit);
Commit getValueFromCommitArray(Commit*,int);
Commit initCommit();
int getCommitRepoId(Commit);
int getCommitAuthorId(Commit);
int getCommitCommitterId(Commit);
Date  getCommitDate(Commit);
char* getCommitMessage(Commit);
void setCommitRepoId(Commit,int );
void setCommitAuthorId(Commit,int );
void setCommitAuthorFriend(Commit,bool);
void setCommitCommitterId(Commit,int );
void setCommitCommitterFriend(Commit,bool);
void setCommitDate(Commit,Date );
void setCommitmessage(Commit,char*);

int getCommitMessageLenght(Commit);

User getUserBycommitterId(GHashTable *,Commit);
User getCommitAuthor(GHashTable *,Commit);
Repo  getRepoOfCommit(GHashTable *,Commit);
int compareCommitToDate(Commit,Date);
int compareCommitDates(Commit, Commit);

void freeCommit(Commit);
void freeCommitList(void* args[]);
void setCompressedCommitDate(Commit,int);
int getCompressedCommitDate(Commit);
Format getCommitFormat();
Format getCompressedCommitFormat();


#endif