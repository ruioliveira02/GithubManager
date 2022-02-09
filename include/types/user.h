/**
 * @file user.h
 * 
 * File containing declaration of the User type
 */

#ifndef _USER_H_

/**
 * @brief Include guard
 */
#define _USER_H_

#include <stdbool.h>

#include "utils/utils.h"
#include "types/date.h"
#include "types/format.h"


/**
 * @brief The struct used for the #User
 * 
 */
typedef struct user *User;

/**
 * @brief The list of data stored in a compressed #User
 * 
 */
typedef enum cuser {
	CUID=0,				///< The id
	CULOGINLEN=1,		///< The length of the login string	
	CUTYPE=2,			///< The ::Type of users
	CUFRIENDS,			///< The number of friends
	CUFRIENDS_LIST,		///< The list of friends 
	CULOGIN,			///< The login string
	/*after this is all unused*/	
	CUCREATED_AT,		///< The date of creation of the account
	CUPUBLIC_GISTS,		///< The number of public Gists
	CUPUBLIC_REPOS,		///< The number of public #Repo
	CUFOLLOWERS,		///< The number of followers
	CUFOLLOWING,		///< The number of following
	CUFOLLOWER_LIST,	///< The list of followers
	CUFOLLOWING_LIST	///< The list of following
} CUSER;


int getUserSizeOf();
User copyUser(User);
User getValueFromUserArray(User*,int);
User initUser();
int  getUserId(User);
char* getUserLogin(User);
Type getUserType(User);
Date  getUserCreationDate(User);
int  getUserFollowers(User);
int* getUserFollowerList(User);
int  getUserFollowing(User);
int*  getUserFollowingList(User);
int  getUserPublicGists(User);
int  getUserPublicRepos(User);
void  setUserId(User,int );
void setUserLogin(User,char* );
void setUserType(User,Type );
void setUserCreationDate(User,Date  );
void  setUserFollowers(User,int );
void setUserFollowerList(User,int*);
void  setUserFollowing(User,int );
void  setUserFollowingList(User,int*);
void  setUserPublicGists(User,int );
void  setUserPublicRepos(User,int );
void freeUserContent(User);
void freeUser(User);
void freeUserList(void*[]);
void calculateFriends(User);
bool areUsersFriends(User,int);
Format getUserFormat();
Format getCompressedUserFormat();

#endif