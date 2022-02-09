/**
 * @file catalog.c
 *
 * File implementing the #Catalog type
 *
 * The #Catalog is the wrapper to the dataset the application uses to solve queries
 */

#include <glib.h>
#include <pthread.h>
#include <types/lazy.h>
#include <unistd.h>

#include "io/indexer.h"
#include "io/taskManager.h"
#include "types/catalog.h"
#include "types/commit.h"
#include "types/format.h"
#include "types/repo.h"
#include "types/user.h"
#include "utils/utils.h"

#define CAT_DIR "saida/"

#define COMPRESSED_USERS            CAT_DIR "users.dat"
#define COMPRESSED_COMMITS          CAT_DIR "commits.dat"
#define COMPRESSED_REPOS            CAT_DIR "repos.dat"
#define USERSBYID_IND               CAT_DIR "usersById.indx"
#define REPOSBYID_IND               CAT_DIR "reposById.indx"
#define COMMITSBYREPO_IND           CAT_DIR "commitsByRepo.indx"
#define COMMITSBYREPO_IND_VALS      CAT_DIR "commitsByRepo.dat"
#define REPOSBYLASTCOMMITDATE_IND   CAT_DIR "reposByLastCommitDate.indx"
#define REPOSBYLANGUAGE_IND         CAT_DIR "reposByLanguage.indx"
#define REPOSBYLANGUAGE_IND_VALS    CAT_DIR "reposByLanguage.dat"
#define COMMITSBYDATE_IND           CAT_DIR "commitsByDate.indx"
#define COLLABORATORS_IND           CAT_DIR "collaborators.indx"
#define COLLABORATORS_IND_VALS      CAT_DIR "collaborators.dat"
#define STATIC_QUERIES              CAT_DIR "staticQueries.dat"

/**
 * @brief 		Directly compares two positions in the same file
 *
 * @remark      Since this function is passed as an argument it must have the same type as stringCmp so some arguments are unused
 *
 * @param f 	The file correspondent to the postion a
 * @param a 	The position to compare in the file f
 * @param g 	The file correspondent to the postion b
 * @param b 	The position to compare in the file g
 * @param c 	The dinamic #Cache
 *
 * @return -1   If a < b
 * @return 0    If a == b
 * @return 1    If a > b
 */
int directCmp(FILE* f, pos_t a, FILE* g, pos_t b, Cache c) {
    return a < b ? -1 : a > b;
}

/**
 * @brief Alias for @ref directCmp
 *
 */
#define imbeddedDateCmp directCmp

/**
 * @brief 		Compares the strings in the given positions of the file
 *
 * 				If one of the files is NULL uses the pos_t as a string to compare
 *
 * @param f1 	The file where to search the string in
 * @param p1 	The position of the string in the file f1 (or the string itself if f1 is null)
 * @param f2 	The file where to search the string in
 * @param p2 	The position of the string in the file f1 (or the string itself if f2 is null)
 * @param c  	The #Cache
 *
 * @return -1   If the string in p1 < the string in p2
 * @return 0    If the string in p1 == the string in p2
 * @return 1    If the string in p1 > the string in p2
 */
int stringCmp(FILE* f1, pos_t p1, FILE* f2, pos_t p2, Cache c) {
    int l1, l2;
    char buff1[sizeof(int)], buff2[sizeof(int)];
    if (f1 && f2) {

        char aux[30];
        getStr(c, f2, p2, aux, 30);

        getStr(c, f1, p1, buff1, sizeof(int));
        l1 = readIntFromBinaryString(buff1);

        getStr(c, f2, p2, buff2, sizeof(int));
        l2 = readIntFromBinaryString(buff2);

        if (l1 != l2)   return l2 - l1;

        p1 += sizeof(int);
        p2 += sizeof(int);
        char c1[20], c2[20];

        for (int i = 0; i * 20 < l1; i++) {
            getStr(c, f1, p1, c1, 20);
            getStr(c, f2, p2, c2, 20);

            int cmp = strncmp(c1, c2, MIN(20, l1 - i * 20));
            if (cmp != 0)   return cmp;

            p1 += 20;
            p2 += 20;
        }
        return 0;
    } else {
        char *c1, *c2;

        if (f1 == NULL) {
            c1 = (char*)p1;
            l1 = strlen(c1);
        } else {
            getStr(c, f1, p1, buff1, sizeof(int));
            l1 = readIntFromBinaryString(buff1);

            c1 = malloc(l1);
            getStr(c, f1, p1 + sizeof(int), c1, l1);
        }

        if (f2 == NULL) {
            c2 = (char*)p2;
            l2 = strlen(c2);
        } else {
            getStr(c, f2, p2, buff2, sizeof(int));
            l2 = readIntFromBinaryString(buff2);

            c2 = malloc(l2);
            getStr(c, f2, p2 + sizeof(int), c2, l2);
        }

        int cmp = 0;
        if (l1 != l2)   cmp = l2 - l1;
        else            cmp = strncmp(c1, c2, l1);

		if (f1) free(c1);
        if (f2) free(c2);
        return cmp;
    }
}


/**
 * @brief Type used to store the catalog corresponding to the dataset of the application
 *
 */
struct catalog {
	Cache cache; 					///< The #Cache to fasten the access times
	FILE* users; 					///< The file where the users are stored
	FILE* commits;					///< The file where the commits are stored
	FILE* repos;					///< The file where the repos are stored
	Format cUserFormat;				///< The format of a compressed user
	Format cCommitFormat;			///< The format of a compressed commit
	Format cRepoFormat;				///< The format of a compressed repo
    Indexer usersById;				///< The index of the users by their id
	Indexer reposById;				///< The index of the repos by their id
	Indexer commitsByRepo;			///< The index of the commits organized by their repo_id
	Indexer reposByLastCommitDate;	///< The index of the repos ordered by their last commit date
	Indexer reposByLanguage;		///< The index of the repos ordered by their language
    Indexer commitsByDate;			///< The index of the commits ordered by their date
    Indexer collaborators;			///< The index of collaborators by repo
    //statistical
    int userCount; 					///< Number of users of type User
    int botCount; 					///< Number of bots
    int organizationCount; 			///< Number of organizations
    double Q2;						///< Values done to resolve query 2
    double Q3;						///< Values done to solve query 3
    double Q4;						///< Values done to solve query 4
};

/**
 * @brief   Get the #Format used to store the static queries of a #Catalog
 * 
 * @return  The #Format
 */
Format getStaticQueriesFormat() {
    struct catalog c;
    void* params[] = { &c.userCount, &c.organizationCount, &c.botCount, &c.Q2, &c.Q3, &c.Q4 };
    FormatType types[] = { BINARY_INT, BINARY_INT, BINARY_INT, BINARY_DOUBLE, BINARY_DOUBLE, BINARY_DOUBLE };
    return makeFormat(&c, params, types, 6, sizeof(struct catalog), NULL, 0, '\0');
}

/**
 * @brief 			A wrapper to call the fuction sortIndexer using a thread
 *
 * @param args 		The arguments to pass to the sortIndexer function
 */
void sortIndexerWrapper(void* args[]){
	sortIndexer((Indexer)args[0], (Cache)args[1]);
}
/**
 * @brief 			A wrapper to call the fuction groupIndexer using a thread
 *
 * @param args 		The arguments to pass to the groupIndexer function
 */
void groupIndexerWrapper(void* args[]){
	groupIndexer((Indexer)args[0],(char*)args[1],*(bool*)args[2], (Cache)args[3]);
}

/**
 * @brief 			Checks whether or not two users are friends using their #Lazy pointers and their id
 *
 * @param c   		The #Catalog
 * @param a     	The id of the first user
 * @param b     	The id of the second user
 * @param one		The lazy of the first user
 * @param two 		The lazy of the second user
 *
 * @return 			Whether or not the users are friends
 */
bool areUsersFriendsByIdAndLazys(Catalog c, int a, int b, Lazy one,Lazy two) {
	int one_len = *(int*)getLazyMember(one,CUFRIENDS,c->cache);
	int two_len = *(int*)getLazyMember(two,CUFRIENDS,c->cache);
	int* one_list = *(int**)getLazyMember(one,CUFRIENDS_LIST,c->cache);
	int* two_list = *(int**)getLazyMember(two,CUFRIENDS_LIST,c->cache);
	bool r=containedInSortedArray(two_list, two_len, a) && containedInSortedArray(one_list, one_len, b);
	return r;
}



/**
 * @brief 			Solves queries number 1,2,3,4 and saves the values in the #Catalog
 * 					It also calculates the friendship status between the collaborators of a commit and the owner of the repo
 *
 * @param catalog 	The #Catalog
 */
void solveStaticQueries(Catalog catalog){
    long long ansQ2=0;
	int numberOfUsers=getElemNumber(catalog->usersById);
    int numberOfCommits=getElemNumber(catalog->commitsByDate);
    int numberOfRepos=getElemNumber(catalog->commitsByRepo);

    catalog->Q3 = 0;
    User user1 = initUser(), user2 = initUser();
    Commit commit = initCommit();
    Repo repo = initRepo();
    Lazy owner = makeLazy(NULL, 0, catalog->cUserFormat, user2),u = makeLazy(NULL, 0, catalog->cUserFormat, user1),
         c = makeLazy(NULL, 0, catalog->cCommitFormat, commit), r = makeLazy(NULL, 0, catalog->cRepoFormat, repo);

    for (int i=0;i<numberOfRepos;i++){
        //number of collaborators to that commit
		int numberOfColabs=getGroupSize(catalog->collaborators,retrieveGroup(catalog->collaborators,i,catalog->cache),catalog->cache);
		ansQ2 += numberOfColabs;

		if (findValueAsLazy(catalog->reposById, retrieveEmbeddedKey(catalog->commitsByRepo,i,catalog->cache),catalog->cache, r)) //the repo to access
        {
            int ownerId=*(int*)getLazyMember(r,CROWNER_ID,catalog->cache);
            pos_t g = retrieveGroup(catalog->commitsByRepo,i,catalog->cache);//the commits to that repo
            getUserById(catalog,ownerId,owner);
            int numberOfCommitsToTheRepo=getGroupSize(catalog->commitsByRepo,g,catalog->cache);
            bool found = false;
            for (int j=0;j<numberOfCommitsToTheRepo;j++){
                getGroupElemAsLazy(catalog->commitsByRepo,g,j,catalog->cache,c);
                int author_id=*(int*)getLazyMember(c,CCAUTHOR_ID,catalog->cache);
                int commiter_id=*(int*)getLazyMember(c,CCCOMMITTER_ID,catalog->cache);
                getUserById(catalog,author_id,u);

                if (!found && *(Type*)getLazyMember(u,CUTYPE,catalog->cache)==BOT){
                    catalog->Q3++;
                    found = true;
                }
                if (areUsersFriendsByIdAndLazys(catalog,author_id,ownerId,u,owner))
                    *(bool*)setLazyMember(c,CCAUTHOR_FRIEND) = true;

                if (author_id!=commiter_id){
                    getUserById(catalog,commiter_id, u);
                    if (!found && *(Type*)getLazyMember(u,CUTYPE,catalog->cache)==BOT){
                        catalog->Q3++;
                        found = true;
                    }
                    if (areUsersFriendsByIdAndLazys(catalog,commiter_id,ownerId,u,owner))
                        *(bool*)setLazyMember(c,CCCOMMITTER_FRIEND) = true;
                }

                printLazyToFile(c, catalog->cache);
            }
        }
	}

    //If the program were to crash, the changes made to the commits file
    //wouldn't be flushed. Flushing them here guarantees this doesn't happen
    //flushCacheFile(catalog->cache, catalog->commits);

    freeLazy(owner);
    freeLazy(u);
    freeLazy(c);
    freeLazy(r);
    free(user1);
    free(user2);
    free(commit);
    free(repo);

    catalog->Q2=((double)ansQ2 / numberOfRepos);
    catalog->Q4=((double)numberOfCommits/numberOfUsers);

    DEBUG_PRINT("solveStaticQueries done\n");
}

/**
 * @brief 		Reads the users in the input file and writes them compressed to the corresponding file and sabes them to the user by id
 *
 * @param args 	The arguments so the function can be executed
 */
void parseUsers(void *args[])
{
	FILE* users=(FILE*)args[0];///< 			The users input File
	FILE* compressed_users=(FILE*)args[1];///<	The ouput File to save the compressed users in
	Indexer usersById=(Indexer)args[2];///<		The #Indexer of usersById
	bool validate=*(bool*)args[3];///<			The boolean flag to check if the users need to be validated
    Cache cache = (Cache)args[4];///<           The #Cache

    int userCount = 0, organizationCount = 0, botCount = 0;

    Format user_f = getUserFormat();
    Format comp_user_f = getCompressedUserFormat();

    char* buffer = NULL;
    int buffer_size;

    getFileLine(users, &buffer, &buffer_size);   //first line
    User u = initUser();

    while (getFileLine(users, &buffer, &buffer_size))
    {
        if (validate ? readFormat(user_f, buffer, u)
                     : (unsafeReadFormat(user_f, buffer, u), true))
        {
            calculateFriends(u);

            switch(getUserType(u))
            {
                case USER:
                    userCount++;
                    break;
                case BOT:
                    botCount++;
                    break;
                case ORGANIZATION:
                    organizationCount++;
                    break;
            }

            pos_t pos = (pos_t)ftell(compressed_users);
            printFormat(comp_user_f, u, compressed_users);
            insertIntoIndex(usersById, (pos_t)getUserId(u), pos);
            freeUserContent(u);
        }
    }

    free(u);
    free(buffer);
    disposeFormat(user_f);
    disposeFormat(comp_user_f);

    fflush(compressed_users);
    sortIndexer(usersById, cache);

    *(int*)args[5] = userCount;
    *(int*)args[6] = organizationCount;
    *(int*)args[7] = botCount;

    DEBUG_PRINT("parseUsers done\n");
}
/**
 * @brief 				Reads the repos in the input file and stores their id in an hashtable
 *
 * @param repos 		The file where the Repos are stored
 * @param repoIds 		The hashtable to store the id's in
 * @param validate 		The boolean flag to see if the repos should be validated or only read
 */
void fillRepoIdHashTable(FILE* repos, GHashTable* repoIds, bool validate)
{
    Format repo_f = getRepoFormat();

    char* buffer = NULL;
    int buffer_size;

    getFileLine(repos, &buffer, &buffer_size);   //first line
    Repo r = initRepo();

    while (getFileLine(repos, &buffer, &buffer_size))
    {
        if (validate ? readFormat(repo_f, buffer, r)
                     : (unsafeReadFormat(repo_f, buffer, r), true))
        {
            g_hash_table_insert(repoIds, GINT_TO_POINTER(getRepoId(r)), GINT_TO_POINTER(1));
            freeFormat(repo_f, r);
        }
    }

    free(r);
    free(buffer);
    disposeFormat(repo_f);

    DEBUG_PRINT("fillRepoIdHashTable done\n");
}
/**
 * @brief 							Reads the commits from the input file and stores them under the compressed form on the coorespondant file
 *
 * @param commits					The File with the commits to be read
 * @param compressed_commits 		The File to output the commits to under the compressed form
 * @param usersById 				The #Indexer of userById
 * @param repoIds 					The hashtable of repos by id
 * @param repoLastCommit 			The hashtable to store the #Date of the last #Commit to each #Repo
 * @param validate 					The boolean flag indicating whether or not to validate the commits
 * @param c 						The #Cache to use to speed up the computation
 */
void filterCommits(FILE* commits, FILE* compressed_commits, Indexer usersById,
                   GHashTable* repoIds, GHashTable* repoLastCommit, bool validate, Cache c)
{
    Format commit_f = getCommitFormat();
    Format comp_commit_f = getCompressedCommitFormat();

    char* buffer = NULL;
    int buffer_size;

    getFileLine(commits, &buffer, &buffer_size);   //first line
    Commit commit = initCommit();
    setCommitAuthorFriend(commit, false);
    setCommitCommitterFriend(commit, false);

    while (getFileLine(commits, &buffer, &buffer_size))
    {
        if (validate ? readFormat(commit_f, buffer, commit)
                     : (unsafeReadFormat(commit_f, buffer, commit), true))
        {
            int author = getCommitAuthorId(commit);
            int committer = getCommitCommitterId(commit);

            if (!validate ||
                (retrieveKey(usersById, (pos_t)author, c) != -1
              && (author == committer || retrieveKey(usersById, (pos_t)committer, c) != -1)
              && g_hash_table_lookup(repoIds, GINT_TO_POINTER(getCommitRepoId(commit))) != NULL))
            {
                printFormat(comp_commit_f, commit, compressed_commits);

                int repo = getCommitRepoId(commit);
                uint date = (uint)getCompressedCommitDate(commit);
                gpointer stored_date = g_hash_table_lookup(repoLastCommit, GINT_TO_POINTER(repo));

                if (stored_date == NULL || GPOINTER_TO_UINT(stored_date) < date)
                    g_hash_table_insert(repoLastCommit, GINT_TO_POINTER(repo), GUINT_TO_POINTER(date));
            }

            freeFormat(commit_f, commit);
        }
    }

    free(commit);
    free(buffer);
    disposeFormat(commit_f);
    disposeFormat(comp_commit_f);

    fflush(compressed_commits);

    DEBUG_PRINT("filterCommits done\n");
}
/**
 * @brief 							Reads the compressed commits and stores in the corresponding #Indexer the values
 *
 * @param compressed_commits		The File where the compressed commits are stored
 * @param usersById					The #Indexer where the usersById are
 * @param commitsByDate				The #Indexer where the commitsByDate are
 * @param commitsByRepo				The #Indexer where the commitsByRepo are
 * @param collaborators				The #Indexer where the collaborators are
 * @param commitsByRepo_ind_vals	The file that stores the arrays of the commitsByRepo
 * @param collaborators_ind_vals	The file that stores the arrays of the collaborators
 * @param c
 */
void parseCommits(FILE* compressed_commits, Indexer usersById,
                  Indexer commitsByDate, Indexer commitsByRepo, Indexer collaborators,
                  char* commitsByRepo_ind_vals, char* collaborators_ind_vals, Cache c)
{
    Format comp_commit_f = getCompressedCommitFormat();
    Commit commit = initCommit();
    Lazy l = makeLazy(NULL, 0, comp_commit_f, commit);

    fseek(compressed_commits, 0, SEEK_END);
    pos_t filesize = (pos_t)ftell(compressed_commits);
    pos_t pos = 0;

    while (pos < filesize)
    {
        setLazyAddress(l, compressed_commits, pos);
        Date d = *(Date*)getLazyMember(l, CCCOMMIT_AT, c);  //TODO: date is uncompressed and compressed
        int repo = *(int*)getLazyMember(l, CCREPO_ID, c);
        int author = *(int*)getLazyMember(l, CCAUTHOR_ID, c);
        int committer = *(int*)getLazyMember(l, CCCOMMITTER_ID, c);

        insertIntoIndex(commitsByDate, (pos_t)getCompactedDate(d), pos);
        insertIntoIndex(commitsByRepo, (pos_t)repo, pos);
        insertIntoIndex(collaborators, (pos_t)repo, getEmbeddedValue(usersById, (pos_t)author, c));
        if (author != committer)
            insertIntoIndex(collaborators, (pos_t)repo, getEmbeddedValue(usersById, (pos_t)committer, c));

        pos = getPosAfterLazy(l, c);
    }

    freeLazy(l);
    free(commit);
    disposeFormat(comp_commit_f);


	bool False=false;
	bool True=true;

	pthread_t threads[2];
	pthread_create(&threads[0], NULL,sequence,SEQ(
		FUNC(sortIndexerWrapper,commitsByRepo, c),
		FUNC(groupIndexerWrapper,commitsByRepo, commitsByRepo_ind_vals, &False, c)
	));
	pthread_create(&threads[1], NULL,sequence,SEQ(
		FUNC(sortIndexerWrapper,collaborators, c),
		FUNC(groupIndexerWrapper,collaborators, collaborators_ind_vals, &True, c)
	));
    sortIndexer(commitsByDate, c);

	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);

    DEBUG_PRINT("parseCommits done\n");
}
/**
 * @brief 		Reads the repos in the input file and writes them compressed to the corresponding file
 *
 * @param args 	The arguments so the function can be executed
 */
void parseRepos(void*args[])
{
	FILE* repos=(FILE*)args[0];							///< The file to read the repos from
	FILE* compressed_repos=(FILE*)args[1];				///< The repos to save the compressed Users
	Indexer usersById=(Indexer)args[2];					///< The #Indexer of usersById
	GHashTable* repoLastCommit=(GHashTable*)args[3];	///< The hashtable of the lastCommitDate to each repo
	Indexer reposById=(Indexer)args[4];					///< The #Indexer of reposById
	Indexer reposByLastCommitDate=(Indexer)args[5];		///< The #Indexer of reposByLastCommitDate
	Indexer reposByLanguage=(Indexer)args[6];			///< The #indexer of reposByLanguage
	char* reposByLanguage_ind_vals=(char*)args[7];		///< The file to store the arrays of repos by language
	bool validate=*(bool*)args[8];						///< The boolean flag weather to validate the repos or nor
	Cache c=(Cache)args[9];								///< The cache to use to accelarate the calculations

    Format repo_f = getRepoFormat();
    Format comp_repo_f = getCompressedRepoFormat();

    char* buffer = NULL;
    int buffer_size;

    getFileLine(repos, &buffer, &buffer_size);   //first line
    Repo r = initRepo();
    Lazy l = makeLazy(NULL, 0, comp_repo_f, NULL);

    while (getFileLine(repos, &buffer, &buffer_size))
    {
        if (validate ? readFormat(repo_f, buffer, r)
                     : (unsafeReadFormat(repo_f, buffer, r), true))
        {
            gpointer lastCommitDate = g_hash_table_lookup(repoLastCommit, GINT_TO_POINTER(getRepoId(r)));

            if (!validate || (retrieveKey(usersById, (pos_t)getRepoOwnerId(r), c) != -1
                              && lastCommitDate != NULL))
            {

                setRepoLastCommitDateFromComp(r, GPOINTER_TO_INT(lastCommitDate));
                repolanguageToLower(r);

                pos_t pos = (pos_t)ftell(compressed_repos);
                printFormat(comp_repo_f, r, compressed_repos);

                insertIntoIndex(reposById, (pos_t)getRepoId(r), pos);
                insertIntoIndex(reposByLastCommitDate, (pos_t)GPOINTER_TO_INT(lastCommitDate), pos);

                //We use the position of the language length as the key to the indexer since the language
                //is right after. This way, to read the language one simply reads the length n and then the
                //following n bytes correspond to the wanted string
                setLazyAddress(l, compressed_repos, pos);
                pos_t lang_pos = getPosOfLazyMember(l, CRLANGUAGE_LEN, c);
                insertIntoIndex(reposByLanguage, lang_pos, pos);
            }
            freeRepoContent(r);
        }
    }

    free(r);
    freeLazy(l);
    free(buffer);
    disposeFormat(repo_f);
    disposeFormat(comp_repo_f);


    fflush(compressed_repos);

	pthread_t threads[2];
	pthread_create(&threads[0], NULL,sequence,SEQ(FUNC(sortIndexerWrapper,reposById,c)));
	pthread_create(&threads[1], NULL,sequence,SEQ(FUNC(sortIndexerWrapper,reposByLastCommitDate,c)));
	sortIndexer(reposByLanguage,c);
    groupIndexer(reposByLanguage, reposByLanguage_ind_vals, false, c);
	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);

    DEBUG_PRINT("parseRepos done\n");
}


/**
 * @brief 		Tries to load the existing #Catalog. If unsucessful, returns NULL
 *
 * @return 		The #Catalog
 */
Catalog loadCatalog()
{
    if (   access(COMPRESSED_USERS,        R_OK) || access(COMPRESSED_COMMITS,        R_OK)
        || access(COMPRESSED_REPOS,        R_OK) || access(USERSBYID_IND,             R_OK)
        || access(REPOSBYID_IND,           R_OK) || access(COMMITSBYREPO_IND,         R_OK)
        || access(COMMITSBYREPO_IND_VALS,  R_OK) || access(REPOSBYLASTCOMMITDATE_IND, R_OK)
        || access(REPOSBYLANGUAGE_IND,     R_OK) || access(REPOSBYLANGUAGE_IND_VALS,  R_OK)
        || access(COMMITSBYDATE_IND,       R_OK) || access(COLLABORATORS_IND,         R_OK)
        || access(COLLABORATORS_IND_VALS,  R_OK) || access(STATIC_QUERIES,            R_OK))
            return NULL;

    Catalog ans = (Catalog)malloc(sizeof(struct catalog));
	ans->cache = getCache(1000000); //TODO: guess size

    ans->users = OPEN_FILE(COMPRESSED_USERS, "rb");
    ans->commits = OPEN_FILE(COMPRESSED_COMMITS, "rb");
    ans->repos = OPEN_FILE(COMPRESSED_REPOS, "rb");

    ans->cUserFormat = getCompressedUserFormat();
	ans->cCommitFormat = getCompressedCommitFormat();
	ans->cRepoFormat = getCompressedRepoFormat();

	ans->usersById = parseIndexer(USERSBYID_IND, NULL, ans->users, directCmp);
	ans->reposById = parseIndexer(REPOSBYID_IND, NULL, ans->repos, directCmp);
	ans->commitsByRepo = parseGroupedIndexer(COMMITSBYREPO_IND, COMMITSBYREPO_IND_VALS, NULL, ans->commits, directCmp);
    ans->reposByLastCommitDate = parseIndexer(REPOSBYLASTCOMMITDATE_IND, NULL, ans->repos, imbeddedDateCmp);
	ans->reposByLanguage = parseGroupedIndexer(REPOSBYLANGUAGE_IND, REPOSBYLANGUAGE_IND_VALS, ans->repos, ans->repos, stringCmp);
	ans->commitsByDate = parseIndexer(COMMITSBYDATE_IND, NULL, ans->commits, imbeddedDateCmp);
	ans->collaborators = parseGroupedIndexer(COLLABORATORS_IND, COLLABORATORS_IND_VALS, NULL, ans->users, directCmp);

    FILE* staticQueries = OPEN_FILE(STATIC_QUERIES, "rb");
    Format static_queries_f = getStaticQueriesFormat();
    char buffer[36];
    
    int read = fread(buffer, sizeof(char), 36, staticQueries);
    if (read != 36) {
        fprintf(stderr, "loadCatalog: unexpected number of characters read (read: %d; expected: 36\n", read);
        freeCatalog(ans);
        ans = NULL;
    } else
        unsafeReadFormat(static_queries_f, buffer, ans);

    disposeFormat(static_queries_f);
    fclose(staticQueries);

    return ans;
}

/**
 * @brief 					Creates the files nedded to load a catalog and then loads it
 *
 * @param users_path 		The path to the file where the users are stored
 * @param commits_path 		The path to the file where the commits are stored
 * @param repos_path 		The path to the file where the repos are stored
 * @param validate 			The boolean flag indicating whether or not the files should be validated
 * @return 					The #Catalog
 */
Catalog newCatalog(char* users_path, char* commits_path, char* repos_path, bool validate)
{
    Catalog ans = (Catalog)malloc(sizeof(struct catalog));
	ans->cache = getCache(1024 * 1024); //TODO: guess size

    GHashTable* repoIds = g_hash_table_new(g_direct_hash, g_direct_equal);
    GHashTable* repoLastCommit = g_hash_table_new(g_direct_hash, g_direct_equal);

    FILE* users = OPEN_FILE(users_path, "r");
    FILE* commits = OPEN_FILE(commits_path, "r");
    FILE* repos = OPEN_FILE(repos_path, "r");

    ans->users = OPEN_FILE(COMPRESSED_USERS, "wb+");
    ans->commits = OPEN_FILE(COMPRESSED_COMMITS, "wb+");
    ans->repos = OPEN_FILE(COMPRESSED_REPOS, "wb+");

    ans->cUserFormat = getCompressedUserFormat();
	ans->cCommitFormat = getCompressedCommitFormat();
	ans->cRepoFormat = getCompressedRepoFormat();

    ans->usersById = makeIndexer(USERSBYID_IND, NULL, ans->users, directCmp);
    ans->reposById = makeIndexer(REPOSBYID_IND, NULL, ans->repos, directCmp);
    ans->commitsByRepo = makeIndexer(COMMITSBYREPO_IND, NULL, ans->commits, directCmp);
    ans->reposByLastCommitDate = makeIndexer(REPOSBYLASTCOMMITDATE_IND, NULL, ans->repos, imbeddedDateCmp);
    ans->reposByLanguage = makeIndexer(REPOSBYLANGUAGE_IND, ans->repos, ans->repos, stringCmp);
    ans->commitsByDate = makeIndexer(COMMITSBYDATE_IND, NULL, ans->commits, imbeddedDateCmp);
    ans->collaborators = makeIndexer(COLLABORATORS_IND, NULL, ans->users, directCmp);

    pthread_t secondaryThread;
    pthread_create(&secondaryThread, NULL, sequence,
    	SEQ(FUNC(parseUsers,users, ans->users, ans->usersById, &validate, ans->cache, &ans->userCount, &ans->organizationCount, &ans->botCount)));
    fillRepoIdHashTable(repos, repoIds, validate);
    fseek(repos, 0, SEEK_SET);
	pthread_join(secondaryThread, NULL);

    filterCommits(commits, ans->commits, ans->usersById, repoIds, repoLastCommit, validate, ans->cache);

	pthread_create(&secondaryThread, NULL, sequence,
    	SEQ(FUNC(parseRepos,repos, ans->repos, ans->usersById, repoLastCommit, ans->reposById,
        ans->reposByLastCommitDate, ans->reposByLanguage, REPOSBYLANGUAGE_IND_VALS, &validate, ans->cache)));
    parseCommits(ans->commits, ans->usersById, ans->commitsByDate, ans->commitsByRepo,
                    ans->collaborators, COMMITSBYREPO_IND_VALS, COLLABORATORS_IND_VALS, ans->cache);
	pthread_join(secondaryThread, NULL);

    g_hash_table_destroy(repoIds);
	g_hash_table_destroy(repoLastCommit);
    fclose(users);
    fclose(commits);
    fclose(repos);

    solveStaticQueries(ans);
    FILE* staticQueries = OPEN_FILE(STATIC_QUERIES, "wb+");
    Format static_queries_f = getStaticQueriesFormat();
    printFormat(static_queries_f, ans, staticQueries);
    disposeFormat(static_queries_f);
    fclose(staticQueries);

    return ans;
}


/**
 * @brief 			Gets the number of users in the dataset
 *
 * @param u 		The given #Catalog
 *
 * @return 			The number of users in the dataset
 */
int getUserCountC(Catalog u) {
    return u->userCount;
}

/**
 * @brief 			Gets the number of bots in the dataset
 *
 * @param u 		The given #Catalog
 *
 * @return 			The number of bots in the dataset
 */
int getBotCountC(Catalog u) {
    return u->botCount;
}

/**
 * @brief 			Gets the number of organizations in the dataset
 *
 * @param u 		The given #Catalog
 *
 * @return 			The number of organizations in the dataset
 */
int getorganizationCountC(Catalog u) {
    return u->organizationCount;
}

/**
 * @brief 			Returns the total number of users stored in the catalog
 *
 * @param c     	The #Catalog
 *
 * @return      	The total number of users
 */
int getUsersCount(Catalog c) {
    return getElemNumber(c->usersById);
}

/**
 * @brief 			Returns the #Laxy pointer to the #User with the specified id. If no such user is present in the catalog, NULL is returned.
 *
 * @param c   		The #Catalog
 * @param id		The specified id
 *
 * @return  		The #Lazy pointer to the #User
 */
void getUserById(Catalog c, int id, Lazy dest) {
    findValueAsLazy(c->usersById, (pos_t)id, c->cache, dest);
}

/**
 * @brief       Prints the login of the specified user to the output file
 *
 * @param c     The #Catalog
 * @param id    The id of the specified #User
 * @param l     Auxiliar #Lazy to load the user. If NULL, the function creates and frees a lazy by its own
 * @param u     Auxiliar value to hold the loaded user. If NULL, the funtion creates and frees a user by its own
 * @param dest  The output file
 */
void printUserLoginById(Catalog c, int id, Lazy l, FILE* dest)
{
    bool freeL = dest == NULL;
    User u;

    if (freeL) {
        u = initUser();
        l = makeLazy(NULL, 0, c->cUserFormat, u);
    }

    getUserById(c, id, l);
    fprintf(dest, "%s", *(char**)getLazyMember(l, CULOGIN, c->cache));

    if (freeL) {
        freeLazy(l);
        free(u);
    }
}

/**
 * @brief 			Returns the number of #Commit stored in the #Catalog
 *
 * @param c     	The #Catalog
 *
 * @return      	The number of #Commit
 */
int getCommitsCount(Catalog c) {
    return getElemNumber(c->commitsByDate);
}

/**
 * @brief Get total number of repos, including phantom repos (these are repos that some commits point to, yet aren't
 *        stored by the catalog)
 * 
 * @param c The #Catalog
 */
int getTotalReposCount(Catalog c)
{
    return getElemNumber(c->commitsByRepo);
}

/**
 * @brief 			Gets the #Repo with the specified id. If no such repo exists, NULL is returned
 *
 * @param c			The #Catalog
 * @param id      	The specified id
 *
 * @return        	The #Repo
 */
bool getRepoById(Catalog c, int id, Lazy dest) {
    return findValueAsLazy(c->reposById, (pos_t)id, c->cache, dest);
}

/**
 * @brief 			Gets the Value From Query 2 Result
 *
 * @param u 		The #Catalog
 *
 * @return 			Querie 2 value
 */
double getValueFromQ2(Catalog u) {
    return u->Q2;
}

/**
 * @brief 			Gets the Value From Query 3 Result
 *
 * @param u 		The #Catalog
 *
 * @return 			Querie 3 value
 */
int getValueFromQ3(Catalog u) {
    return u->Q3;
}

/**
 * @brief 			Gets the Value From Query 4 Result
 *
 * @param u 		The #Catalog
 *
 * @return 			Querie 4 value
 */
double getValueFromQ4(Catalog u) {
    return u->Q4;
}

/**
 * @brief 			Answers the first query based on the given data
 *
 * @note            Complexity: O(1)
 *
 * @param catalog  	The #Catalog
 * @param stream    The stream to write to
 */
void queryOne(Catalog catalog, FILE* stream) {
    fprintf(stream, "Bot: %d\nOrganization: %d\nUser: %d\n",catalog->botCount,catalog->organizationCount,catalog->userCount);
}

/**
 * @brief 			Executes the seventh query (repos not updated since given date)
 *
 * @param catalog   The given #Catalog
 * @param date      The given #Date
 * @param stream    The stream to write the ouput to
 */
void querySeven(Catalog catalog, Date date, FILE* stream) {

    int last = retrieveKeyLowerBound(catalog->reposByLastCommitDate, (pos_t)getCompactedDate(date), catalog->cache);
    Repo repo = initRepo();
    Lazy r = makeLazy(NULL, 0, catalog->cRepoFormat, repo);
    for (int i = 0; i < last; i++){
        retrieveValueAsLazy(catalog->reposByLastCommitDate, i, catalog->cache, r);
        int repoId = *(int*)getLazyMember(r, CRID, catalog->cache);
        char* desc = *(char**)getLazyMember(r, CRDESCRIPTION, catalog->cache);
        fprintf(stream, "%d;%s\n", repoId, desc);
    }
    freeLazy(r);
    free(repo);
}

/**
 * @brief 			Frees a #Catalog
 *
 * @param catalog  	The #Catalog to be freed
 */
void freeCatalog(Catalog catalog)
{
    disposeFormat(catalog->cUserFormat);
	disposeFormat(catalog->cCommitFormat);
	disposeFormat(catalog->cRepoFormat);

    freeIndexer(catalog->usersById, catalog->cache);
    freeIndexer(catalog->reposById, catalog->cache);
    freeIndexer(catalog->commitsByRepo, catalog->cache);
    freeIndexer(catalog->reposByLastCommitDate, catalog->cache);
    freeIndexer(catalog->reposByLanguage, catalog->cache);
    freeIndexer(catalog->commitsByDate, catalog->cache);
    freeIndexer(catalog->collaborators, catalog->cache);

    //The cache must be freed before closing any altered file to allow it to flush the changes
    freeCache(catalog->cache);

    fclose(catalog->commits);
    fclose(catalog->users);
    fclose(catalog->repos);


    free(catalog);
}


/**
 * @brief 				Gets a HashTable Of #User and their number of #Commit in an interval of #Date
 *
 * 						The hashtable has a type UserID:Number of commits they collaborated in
 *
 * @param catalog 		the #Catalog to search the commits in
 * @param startDate		the starting #Date to search
 * @param endDate 		the ending #Date to search
 * @param du 			the number of diferent #User which collaborated in between the #Date
 *
 * @return 				GHashTable of #User and the number of commits they collaborated in
 */
GHashTable* getHashTableOfUserWithCommitsAfter(Catalog catalog,Date startDate,Date endDate,int* du) {
	GHashTable* users = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, NULL);
	pos_t date1=(pos_t)getCompactedDate(startDate);
	pos_t date2=(pos_t)getCompactedDate(endDate);
    int commits = getElemNumber(catalog->commitsByDate);
    Commit c = initCommit();
    Lazy commit = makeLazy(NULL, 0, catalog->cCommitFormat, c);
    *du = 0;
    for (int i = retrieveKeyLowerBound(catalog->commitsByDate, date1, catalog->cache); i < commits; i++){
        if (retrieveEmbeddedKey(catalog->commitsByDate, i, catalog->cache) > date2) break;
        retrieveValueAsLazy(catalog->commitsByDate, i, catalog->cache, commit);
        int commiter_id = *(int*)getLazyMember(commit,CCCOMMITTER_ID,catalog->cache);
		int author_id = *(int*)getLazyMember(commit,CCAUTHOR_ID,catalog->cache);
		increaseNumberInHashTableIfFound(users,GINT_TO_POINTER(author_id),du);
        if (commiter_id != author_id) increaseNumberInHashTableIfFound(users,GINT_TO_POINTER(commiter_id),du);
    }
    freeLazy(commit);
    free(c);
    return users;
}

/**
 * @brief 			Gets the HashTable Of number of #Commit of a #User in of a given language.
 *
 * 					The hashtable has a type UserID:Number of commits they collaborated in of the given language.
 *
 * @param catalog 	the #Catalog to find the commits in
 * @param lang 		the language to search by (case insensitive)
 * @param du 		the number of different users found
 *
 * @return 			GHashTable of #User and the number of #Commit they collaborated in of the given language.
 */
GHashTable* getHashTableOfCommitsPerLanguage(Catalog catalog, char* lang, int*du) {
    GHashTable* count = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, NULL);
    char* dup=toLower(strdup(lang));
    pos_t repos = getGroup(catalog->reposByLanguage, (pos_t)dup, catalog->cache);
    int repos_size = getGroupSize(catalog->reposByLanguage, repos, catalog->cache);
    Repo r = initRepo();
    Commit c = initCommit();
    Lazy repo = makeLazy(NULL, 0, catalog->cRepoFormat, r), commit = makeLazy(NULL, 0, catalog->cCommitFormat, c);
    *du = 0;
    for (int i = 0; i < repos_size; i++){
        getGroupElemAsLazy(catalog->reposByLanguage, repos, i, catalog->cache, repo);
		pos_t commits = getGroup(catalog->commitsByRepo, *(int*)getLazyMember(repo,CRID,catalog->cache), catalog->cache);
		int N_commits=getGroupSize(catalog->commitsByRepo, commits, catalog->cache);
		for (int j = 0; j < N_commits; j++){
			getGroupElemAsLazy(catalog->commitsByRepo, commits, j, catalog->cache, commit);
			int committer_id = *(int*)getLazyMember(commit,CCCOMMITTER_ID,catalog->cache);
			int author_id = *(int*)getLazyMember(commit,CCAUTHOR_ID,catalog->cache);
			increaseNumberInHashTableIfFound(count,GINT_TO_POINTER(committer_id),du);
        	if (committer_id != author_id) increaseNumberInHashTableIfFound(count,GINT_TO_POINTER(author_id),du);
		}
	}
    freeLazy(commit);
    freeLazy(repo);
    free(dup);
    free(c);
    free(r);
	return count;
}

/**
 * @brief 			Gets the HashTable of Numbers Of repos Of A Language updated After a given date
 *
 * 					the hashtable has type language:Number of appearences
 *
 * @param catalog 	the catalog to get the data from
 * @param startDate the date to lower bound of dates
 * @return 			GHashTable of languages and the number of repos using that language updated after a given date
 */
GHashTable* getHashTableOfNumbersOfAperencesOfALanguageAfter(Catalog catalog,Date startDate){
	GHashTable* languageCount = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, NULL);
	pos_t date1=(pos_t)getCompactedDate(startDate); //imbed datas em pos_t
    int N_commits = getElemNumber(catalog->commitsByDate);
    Commit c = initCommit();
	Repo r = initRepo();
    Lazy commit = makeLazy(NULL, 0, catalog->cCommitFormat, c);
	Lazy repo = makeLazy(NULL, 0, catalog->cRepoFormat, r);
    for (int i = retrieveKeyLowerBound(catalog->commitsByDate, date1, catalog->cache); i < N_commits; i++){
        retrieveValueAsLazy(catalog->commitsByDate, i, catalog->cache, commit);
		if (getRepoById(catalog,*(int*)getLazyMember(commit,CCREPO_ID,catalog->cache),repo)) {
            char* language = toLower(*(char**)getLazyMember(repo,CRLANGUAGE,catalog->cache));
            gpointer searchResult = g_hash_table_lookup(languageCount,language);
            if(searchResult != NULL) {
                int newValue = GPOINTER_TO_INT(searchResult) + 1;
                g_hash_table_insert(languageCount,language,GINT_TO_POINTER(newValue));
            } else
                g_hash_table_insert(languageCount,strdup(language),GINT_TO_POINTER(1));
        }
    }
    freeLazy(commit);
    freeLazy(repo);
    free(c);
	free(r);
	return languageCount;
}

/**
 * @brief 			Gets the HashTable of the number of #Commit in friends #Repo of each #User
 *
 * 					The HashTable has type UserID:Number of commits in friends repos
 *
 * @param catalog 	the #Catalog to get the data from
 *
 * @return 			GHashTable of the number of #Commit in friends #Repo of each user
 */
GHashTable* getHashTableOfCommitCountInReposOfFriends(Catalog catalog){
	int differentUsers=0;
    GHashTable* count = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, NULL);
	int numberOfCommits=getCommitsCount(catalog);
    Commit c = initCommit();
    Lazy commit = makeLazy(NULL, 0, catalog->cCommitFormat, c);
    for (int i=0;i<numberOfCommits;i++){
		retrieveValueAsLazy(catalog->commitsByDate,i,catalog->cache,commit);
		int author   = *(int*)getLazyMember(commit,CCAUTHOR_ID,catalog->cache);
        int commiter = *(int*)getLazyMember(commit,CCCOMMITTER_ID,catalog->cache);
		if (*(bool*)getLazyMember(commit,CCAUTHOR_FRIEND,catalog->cache))
			increaseNumberInHashTableIfFound(count,GINT_TO_POINTER(author),&differentUsers);
		if(commiter!=author && *(bool*)getLazyMember(commit,CCCOMMITTER_FRIEND,catalog->cache))
			increaseNumberInHashTableIfFound(count,GINT_TO_POINTER(commiter),&differentUsers);
	}
    freeLazy(commit);
    free(c);
    return count;
}

/**
 * @brief 			Get the HashTable of the users and their maximum length commit message to a given repo (the one in position pos)
 *
 * 					The HashTable has type UserID : the length of the biggest message of a commit the user collaborated in
 *
 * @param catalog 	the catalog to seatch the data in
 * @param pos 		the position of the repo in the catalog->repos
 * @param id 		the id of the repos in position pos
 * @return 			GHashTable of users and the length of their biggest message in a commit they collaborated in of a given repo
 */
GHashTable* getHashTableOfLengthOfCommitsInArepoByRepoPositionInList(Catalog catalog,int pos,int*id){
    //get list of commits foreach repo
	GHashTable* UserbyCount=g_hash_table_new(g_direct_hash, g_direct_equal);
    *id=(int)retrieveEmbeddedKey(catalog->commitsByRepo,pos,catalog->cache);
	pos_t g = retrieveGroup(catalog->commitsByRepo,pos,catalog->cache);
	int numberOfCommits=getGroupSize(catalog->commitsByRepo,g,catalog->cache);
    Commit c = initCommit();
    Lazy commit = makeLazy(NULL, 0, catalog->cCommitFormat, c);
	for (int j=0;j<numberOfCommits;j++){
    	getGroupElemAsLazy(catalog->commitsByRepo,g,j, catalog->cache,commit);
		int messageLen   = *(int*)getLazyMember(commit,CCMESSAGE_LEN,catalog->cache);
        int author_id    = *(int*)getLazyMember(commit,CCAUTHOR_ID,catalog->cache);
        int committer_id = *(int*)getLazyMember(commit,CCCOMMITTER_ID,catalog->cache);
		storeNumberInHashTableIfGreater(UserbyCount, GINT_TO_POINTER(author_id),messageLen);
		if (committer_id!=author_id) storeNumberInHashTableIfGreater(UserbyCount, GINT_TO_POINTER(committer_id),messageLen);
	}
    freeLazy(commit);
    free(c);
	return UserbyCount;
}
