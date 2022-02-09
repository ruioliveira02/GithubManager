/**
 * @file user.c
 *
 * File containing the implementation of the #User type and the api to access it.
 *
 * The #User is used to store the information of a particular Github account.
 */

#include <stdlib.h>
#include <string.h>

#include "types/date.h"
#include "types/format.h"
#include "types/user.h"


/**
 * @brief Type used to define a user
 */
struct user {
    int id;             ///< The id of a user
	int login_len;      ///< The length of the login string
    char *login;        ///< The login name of a user
    Type type;          ///< The type of a user (User, Bot, Organization)
    Date created_at;    ///< The date/time the account of the user was created at
    int followers;      ///< The number of followers of a user
    int *follower_list; ///< The list of followers id's of a user
    int following;      ///< The number of users a user is following
    int *following_list;///< The list of following id's of a user
    int public_gists;   ///< The number of public gists of a user
    int public_repos;   ///< The number of public repositories of a user
	int friends;        ///< The number of friends of the user. A friend of a user is someone who is followed by and follows them
	int *friends_list;  ///< The list of friends id's of a user. A friend of a user is someone who is followed by and follows them. Always sorted
};

/**
 * @brief       Gets the length of the user 's login
 *
 * @param user  The given #User
 *
 * @return      The length (in bytes) of the user's login
 */
int getUserLoginLength(User user) {
    return user->login_len;
}

/**
 * @brief       Returns the number of friends a #User has
 *
 * @remark      A friend of a #User is a #User who follows them and is followed by them
 *
 * @param user  The given #User
 *
 * @return      The number of friends of the #User
 */
int getUserfriends(User user) {
    return user->friends;
}

/**
 * @brief       Returns the list of friends of a #User
 *
 * @remark      A friend of a #User is a #User who follows them and is followed by them
 *
 * @remark      The returned value is a deep copy of the original
 *
 * @param user  The given #User
 *
 * @return      The list of friends of the #User
 */
int* getUserfriends_list(User user) {
    return copyIdList(user->friends_list,user->friends);
}

/**
 * @brief   Get the size (in bytes) the #user object
 *
 * @return  The size of the #user
 */
int getUserSizeOf() {
    return sizeof(struct user);
}

/**
 * @brief       Creates a deep copy of a #User
 *
 * @param user  #User to be copied
 * @return      A deep copy of the given #User
 */
User copyUser(User user) {
    if (!user)
        return NULL;

    User copy = malloc(sizeof(struct user));
    *copy = *user;

    copy->login = strdup(user->login);
    copy->created_at = copyDate(user->created_at);
    copy->follower_list = copyIdList(user->follower_list, user->followers);
    copy->following_list = copyIdList(user->following_list, user->following);
	copy->friends_list = copyIdList(user->friends_list, user->friends);

    return copy;
}

/**
 * @brief           Gets the #User at the i-th position of a #User array
 *
 * @remark          The returned value is a deep copy of the original
 *
 * @param userArray The array of users to get the value from
 * @param i         The position to return the user from
 *
 * @return User     The deep copy of the #User in that position
 */
User  getValueFromUserArray(User *userArray,int i) {
    User  copy=copyUser(userArray[i]);
    return copy;
}

/**
 * @brief   Creates a #User with default values (everything empty)
 *
 * @return  The created #User
 */
User initUser() {
    User user = malloc(sizeof(struct user));

    user->created_at = NULL;
    user->follower_list = NULL;
    user->following_list = NULL;
	user->friends_list = NULL;
    user->login = NULL;

    return user;
}

/**
 * @brief       Gets the id of the #User object
 *
 * @param user  The given #User
 *
 * @return      The id of the #User
 */
int getUserId(User user) {
    return user->id;
}

/**
 * @brief       Gets the login of the #User
 *
 * @remark      The value returned is a copy of the original string
 *
 * @param user  The given #User
 *
 * @return      The copy of the login of the #User
 */
char* getUserLogin(User user) {
    return strdup(user->login);
}

/**
 * @brief       Gets the ::Type of a #User.
 *
 *
 * @param user  The given #User
 * @return      The ::Type of the #User
 */
Type getUserType(User user) {
    return user->type;
}

/**
 * @brief       Gets the #Date of creation of the #User account
 *
 * @remark      The function returns a deep copy of the #Date
 *
 * @param user  The given #User
 *
 * @return      The #Date of creation of the #User account
 */
Date getUserCreationDate(User user) {
    return copyDate(user->created_at);
}

/**
 * @brief       Gets the followers from the #User
 *
 * @param user  The given #User
 *
 * @return      The #User 's followers count
 */
int getUserFollowers(User user) {
    return user->followers;
}

/**
 * @brief       Gets the list of followers of a #User
 *
 * @remark      The list is a copy of the original
 *
 * @param user  The given #User
 *
 * @return      A copy of the list of followers of the #User
 */
int* getUserFollowerList(User user) {
    int* out = malloc(sizeof(int) * user->followers);
    return memcpy((void*)out, (void*)user->follower_list, sizeof(int) * user->followers);
}

/**
 * @brief       Gets the number of accounts the #User follows
 *
 * @param user  The given #User
 *
 * @return      The number of accounts the #User follows
 */
int  getUserFollowing(User user) {
    return user->following;
}

/**
 * @brief       Gets the list of accounts the #User follows
 *
 * @remark      The return list is a copy of the original
 *
 * @param user  The given #User
 *
 * @return      The copy of the list of accounts the #User follows
 */
int* getUserFollowingList(User user) {
    int* out = malloc(sizeof(int) * user->following);
    return memcpy((void*)out, (void*)user->following_list, sizeof(int) * user->following);
}

/**
 * @brief       Gets the number of public Gists of a #User
 *
 * @param user  The given #User
 *
 * @return      The number of public Gists of a #User
 */
int  getUserPublicGists(User user) {
    return user->public_gists;
}

/**
 * @brief       Gets the number of public repos of a #User
 *
 * @param user  The given #User
 *
 * @return      The number of public repos of a #User
 */
int  getUserPublicRepos(User user) {
    return user->public_repos;
}



/**
 * @brief       Sets the id of a #User
 *
 * @param user  The given #User
 * @param id    The new id of the #User
 */
void  setUserId(User user, int id) {
    user->id = id;
}

/**
 * @brief       Sets the login of a #User
 *
 * @remark      The login which will be stored is a copy of the given string
 *
 * @param user  The given #User
 * @param login The new login of the #User
 */
void setUserLogin(User user, char* login) {
    user->login = strdup(login);
}

/**
 * @brief Sets the #User 's ::Type
 *
 * @param user The given #User
 * @param type The new ::Type of the #User
 */
void setUserType(User user, Type type) {
    user->type= type;
}

/**
 * @brief       Sets the #Date of creation of a #User
 *
 * @param user  The given #User
 * @param date  The new creation #Date of the #User
 */
void setUserCreationDate(User user, Date date) {
    freeDate(user->created_at);
    memcpy((void*)user->created_at, (void*)date, getDateSizeof());
}

/**
 * @brief           Sets the number of followers of the #User
 *
 * @param user      The given #User
 * @param followers The new number of followers of the #User
 */
void setUserFollowers(User user, int followers) {
    user->followers = followers;
}

/**
 * @brief       Sets the list of followers of the #User
 *
 * @remark      The stored list is a copy of the given one
 *
 * @param user  The given #User
 * @param list  The new list of followers of the #User
 */
void setUserFollowerList(User user, int* list) {
    free(user->follower_list);
    memcpy((void*)user->follower_list, (void*)list, sizeof(int) * user->followers);
}

/**
 * @brief           Sets the number of accounts the #User follows
 *
 * @param user      The given #User
 * @param following The new number of accounts the #User follows
 */
void setUserFollowing(User user, int following) {
    user->following = following;
}

/**
 * @brief           Sets the list of accounts the #User follows
 *
 * @remark          The stored list is a copy of the given one
 *
 * @param user      The given #User
 * @param list      The new list of accounts the #User follows
 */
void setUserFollowingList(User user, int* list) {
    free(user->following_list);
    memcpy((void*)user->following_list, (void*)list, sizeof(int) * user->following);
}

/**
 * @brief       Sets the number of public Gists of the #User
 *
 * @param user  The given #User
 * @param gists The new number of public Gists of the #User
 */
void  setUserPublicGists(User user,int gists) {
    user->public_gists = gists;
}

/**
 * @brief       Set the number of public repositories of the #User
 *
 * @param user  The given #User
 * @param repos The new number of public repositories of the #User
 */
void  setUserPublicRepos(User user, int repos) {
    user->public_repos = repos;
}

/**
 * @brief Frees the allocated members of the user
 *
 * @param user  The user
 */
void freeUserContent(User user) {
    free(user->login);
    free(user->follower_list);
    free(user->following_list);
    freeDate(user->created_at);
    free(user->friends_list);
}

/**
 * @brief           Frees the memory allocated to a #User
 *
 * @param user      The #User to be disposed of
 */
void freeUser(User user) {
    freeUserContent(user);
    free(user);
}

/**
 * @brief           Frees the memory allocated to the list of users
 *                  Follows the routine prototype
 *
 * @param args      A void pointer array containing:    A pointer to a pointer to the first User to free
 *                                                      A pointer to the size of the list
 */
void freeUserList(void *args[]) {
    User list = *(User *)args[0];
    int N = *(int *)args[1];

    for (int i = 0; i < N; i++)
        freeUser(list + i);

    free(list);
}

/**
 *
 * @brief   Gets the #Format used for the uncompressed #User throughout the project
 *
 * @return  The #Format used for the uncompressed #User
 *
 */
Format getUserFormat() {
    struct user user;
    void* params[] = { &user.id, &user.login, &user.type, &user.created_at, &user.followers,
        &user.follower_list, &user.following, &user.following_list, &user.public_gists, &user.public_repos };
    FormatType types[] = { INT, STRING, TYPE, DATE_TIME, INT, INTLIST, INT, INTLIST, INT, INT };
    PAIR *lists=malloc(3*getSizeOfPair());
	addToPair(lists,0,1,&user.login_len);
	addToPair(lists,1,5,&user.followers);
	addToPair(lists,2,7,&user.following);
	Format f=makeFormat(&user, params, types, 10, sizeof(struct user), lists, 3, ';');
	free(lists);
	return f;
}


/**
 *
 * @brief   Gets the #Format used for the compressed #User throughout the project
 *
 * @return  The #Format used for the compressed #User
 *
 */
Format getCompressedUserFormat() {
    struct user user;
    void* params[] = { &user.id,&user.login_len,&user.type,&user.friends,&user.friends_list,&user.login,/*after this is all unused*/&user.created_at,&user.public_gists,&user.public_repos,&user.followers,&user.following,&user.follower_list,&user.following_list};
    FormatType types[] = { BINARY_INT,BINARY_INT,BINARY_TYPE,BINARY_INT,BINARY_INTLIST,STRING,/*after this is all unused*/BINARY_DATE_TIME,BINARY_INT,BINARY_INT,BINARY_INT,BINARY_INT,BINARY_INTLIST,BINARY_INTLIST};
    PAIR *lists=malloc(4*getSizeOfPair());
	addToPair(lists,0,CUFRIENDS_LIST,&user.friends);
	addToPair(lists,1,CULOGIN,&user.login_len);
	addToPair(lists,2,CUFOLLOWER_LIST,&user.followers);
	addToPair(lists,3,CUFOLLOWING_LIST,&user.following);
	Format f=makeFormat(&user, params, types, 13, sizeof(struct user), lists, 4, '\0');
	free(lists);
	return f;
}

/**
 * @brief       Auxilary function to @ref calculateFriends. Compares two ints.
 *
 * @param a     First number
 * @param b     Second number
 *
 * @return      The difference b - a
 */
int compareInts(const void* a, const void* b) {
    return *(int*)a - *(int*)b;
}

/**
 * @brief       Fills the fields friends and friends_list of the #User.
 *
 *              As a side effect, sorts one of the lists follower_list or following_list
 *
 * @param user  The given #User
 */
void calculateFriends(User user)
{
    if (user->followers == 0 || user->following == 0) {
        user->friends = 0;
        user->friends_list = NULL;
    } else {
        int* small = user->followers < user->following ? user->follower_list : user->following_list;
        int small_l = MIN(user->followers, user->following);
        int* big = user->followers < user->following ? user->following_list : user->follower_list;
        int big_l = MAX(user->followers, user->following);

        GArray* ans = g_array_new(FALSE, FALSE, sizeof(int));
        qsort(small, small_l, sizeof(int), compareInts);

        for (int i = 0; i < big_l; i++)
            if (containedInSortedArray(small, small_l, big[i]))
                g_array_append_val(ans, big[i]);

        user->friends = ans->len;
        if (user->friends == 0)
            user->friends_list = NULL;
        else
            user->friends_list = (int*)ans->data;

        g_array_free(ans, user->friends == 0);
    }
}


/**
 * @brief           Whether 2 #User are friends.
 *
 * @remark          Two #User are said to be friends if and only if they follow and are followed by one another
 *
 * @param a         The first #User
 * @param b_id      The id of the second #User
 *
 * @return true     If they are friends
 * @return false    If they are not friends
 */
bool areUsersFriends(User a, int b_id) {
    return containedInSortedArray(a->friends_list, a->friends, b_id);
}
