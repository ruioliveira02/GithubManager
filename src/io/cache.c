/**
 * @file cache.c
 * 
 * File containing the implementation of the #Cache type
 * 
 * The #Cache is used to minimize the number of file accesses needed when executing a query
 * 
 */

#include <glib.h>
#include <stdio.h>
#include <unistd.h>

#include "io/cache.h"


/**
 * @brief The prime number used to calculate the hash of a #Key
 * 
 */
#define HASH_KEY 4294967029

/**
 * @brief   The basic data unit of the #Cache
 * 
 *          Contains information about the position in a file of the data block read
 */
typedef struct key {
    int file_desc;  ///< The file descriptor the of the data
    pos_t pos;      ///< The position (in bytes) in the file of the data
} KEY, * Key;

/**
 * @brief   A line of the #Cache
 * 
 *          A line is a doubly linked list of #Key and the data as a binary string 
 */
typedef struct line {
    struct line * prev;     ///< The previous element in the list (NULL if first element)
    struct line * next;     ///< The next element in the list (NULL if last element)

    KEY key;                ///< The #Key of the element

    bool loaded;            ///< Whether or not the data has been loaded from file
    bool altered;

    pthread_mutex_t mutex;  ///< The mutex of the line
    char* data;             ///< The data of the line (size LINE_SIZE)
} * Line;

/**
 * @brief       Checks if two #Key are equal
 * 
 *              Two #Key are said to be equal if they have the same position and file descriptor
 * 
 * @param k1    The first #Key
 * @param k2    The second #Key
 * 
 * @return      Whether or not they are the same      
 */
gboolean key_equal(gconstpointer k1, gconstpointer k2) {
    Key _k1 = (Key)k1, _k2 = (Key)k2;
    return _k1->pos == _k2->pos && _k1->file_desc == _k2->file_desc;
}

/**
 * @brief   Hashes a #Key
 *
 * 
 * @param k The given #Key
 * 
 * @return  The hash of the #Key (an integer)
 */
guint key_hash(gconstpointer k) {
    Key _k = (Key)k;
    return _k->file_desc ^ (_k->pos % (guint)HASH_KEY);
}

/**
 * @brief The type used to represent the program's cache
 * 
 * The cache aims to reduce the number of file accesses needed throughout the execution of the program
 * 
 */
struct cache {
    Line* lines;                ///< The lines of the cache, each LINE_SIZE bytes long. Not to confuse with file lines, that end in '\n'
    Line first;                 ///< The most recently accessed #Line
    Line last;                  ///< The least recently accessed #Line

    pthread_mutex_t mutex;      ///< The mutex of the cache
    GHashTable* posLinePairs;   ///< Hashtable of cache #Line indexed by #Key

    int line_num;               ///< The number of lines of the cache
    long hits;                  ///< The number of hits in the cache (for statistical purposes only)
    long misses;                ///< The number of misses in the cache (for statistical purposes only)
};

/**
 * @brief           Creates a #Cache with the given number of #Line
 * 
 * @param line_num  The number of #Line
 * @return          The requested #Cache
 */
Cache getCache(int line_num) {
    Cache c = malloc(sizeof(struct cache));
    c->posLinePairs = g_hash_table_new(key_hash, key_equal);
    pthread_mutex_init(&c->mutex, NULL);

    c->lines = malloc(line_num * sizeof(Line));
    c->lines[0] = malloc(line_num * sizeof(struct line));
    c->lines[0]->data = malloc(line_num * CACHE_LINE_SIZE * sizeof(char));
    c->lines[0]->prev = NULL;
    c->lines[0]->key.file_desc = -1;
    c->lines[0]->loaded = false;
    pthread_mutex_init(&c->lines[0]->mutex, NULL);

    for (int i = 1; i < line_num; i++) {
        c->lines[i] = c->lines[0] + i;
        c->lines[i]->data = c->lines[0]->data + i * CACHE_LINE_SIZE * sizeof(char);

        c->lines[i-1]->next = c->lines[i];
        c->lines[i]->prev = c->lines[i-1];

        c->lines[i]->key.file_desc = -1;
        c->lines[i]->loaded = false;
        c->lines[i]->altered = false;
        pthread_mutex_init(&c->lines[i]->mutex, NULL);
    }

    c->lines[line_num-1]->next = NULL;

    c->first = c->lines[0];
    c->last = c->lines[line_num - 1];

    c->line_num = line_num;
    c->hits = 0;
    c->misses = 0;

    return c;
}

/**
 * @brief       Updates a #Cache line: flushes it if altered and loads if if not loaded
 * 
 * @warning     Does not set the previous/next members
 * 
 * @param line      The #Line to update
 * @param old_key   The old key to flush
 */
void updateCacheLine(Line line, Key old_key) {
    if (!line->loaded || line->altered) {
        pthread_mutex_lock(&line->mutex);

        if (line->altered) {
            ssize_t write = pwrite(line->key.file_desc, line->data, CACHE_LINE_SIZE, line->key.pos);
        
            if (write == -1)
                fprintf(stderr, "updateCacheLine: error writing to file (file descriptor: %d)\n", line->key.file_desc);
        
            line->altered = false;
        }

        if (!line->loaded) {
            ssize_t read = pread(line->key.file_desc, line->data, CACHE_LINE_SIZE, line->key.pos);

            if (read == -1)
                fprintf(stderr, "updateCacheLine: error reading file (file descriptor: %d)\n", line->key.file_desc);
            else if (read < CACHE_LINE_SIZE)    //End of file
                line->data[read] = '\0';

            line->loaded = true;
        }

        pthread_mutex_unlock(&line->mutex);
    }
}

/**
 * @brief           Searches the #Cache for a #Key matching the given file descriptor and position.
 *                  The given position may be bigger than the file's size
 * 
 * @param c         The given #Cache
 * @param file_desc The given file descriptor
 * @param pos       The given position in the file
 * 
 * @return NULL     If there is no such #Line
 * @return Line     The requested #Line
 */
Line getCacheLine(Cache c, int file_desc, pos_t pos) {
    //TODO: make small caches not fail miserably with multi-threading

    Line l;
    KEY key = (KEY){ .file_desc = file_desc, .pos = pos - pos % (pos_t)CACHE_LINE_SIZE };
    KEY old_key;

    pthread_mutex_lock(&c->mutex);

    gpointer search = g_hash_table_lookup(c->posLinePairs, (gpointer)&key);

    if (search != NULL) { //Hit 
        c->hits++;
        l = (Line)search;
    } else { //Miss
        c->misses++;
        l = c->last;
        if (l->loaded)
            g_hash_table_remove(c->posLinePairs, (gpointer)&l->key);
        old_key = l->key;
        l->key = key;
        l->loaded = false;
        l->altered = false;
        g_hash_table_insert(c->posLinePairs, (gpointer)&l->key, (gpointer)l);
    }

    if (l != c->first) {
        //Place l at the beginning of the queue (most recently accessed)
        if (l == c->last)
            c->last = l->prev;     
        else
            l->next->prev = l->prev;

        l->prev->next = l->next;
        l->next = c->first;
        c->first->prev = l;
        c->first = l;
        l->prev = NULL;
    }

    pthread_mutex_unlock(&c->mutex);
    updateCacheLine(l, &old_key);
    return l;
}

/**
 * @brief Fills the buffer with one line of the file, excluding line breaks ('\n' and '\r')
 * 
 *                  Writes no more than max_write bytes.
 * 
 * @warning         If the above threshold is reached, no null-terminator is printed
 * 
 * @param c         The given #Cache
 * @param file      The given file
 * @param pos       The starting position inside the file
 * @param buffer    The given buffer
 * @param max_write The maximum number of bytes to write
 * 
 * @return          The number of written characters
 */
int getLine(Cache c, FILE* file, pos_t pos, char buffer[], int max_write) {
    Line l = getCacheLine(c, fileno(file), pos);

    if (l == NULL)
        return 0;
    
    int line_pos = pos % (pos_t)CACHE_LINE_SIZE, str_len = CACHE_LINE_SIZE - line_pos, i;
    char* str = l->data + line_pos;

    for (i = 0; *str != '\n' && *str != '\0' && i < str_len && i < max_write; i++)
        buffer[i] = *(str++);

    if (i < str_len && *str == '\n' && buffer[i-1] == '\r')
            buffer[i-1] = '\0';
    else if (i < max_write && i < str_len && (*str == '\n' || *str == '\0'))
            buffer[i++] = '\0';
    else if (i < max_write && i == str_len && buffer[i-1] != '\n' && buffer[i-1] != '\0')
        return i + getLine(c, file, pos + (pos_t)str_len, buffer + str_len, max_write - str_len);

    return i;
}

//If the requested string goes beyond the end of the file, only a single '\0' is garanteed to exist!!!
//buffer is not '\0'-terminated by getStr
/**
 * @brief Writes the requested number of bytes from the starting position of the given file to the fgiven buffer
 * 
 * @warning             If the requested string goes beyond the end of the file, only a single '\0' is garanteed to exist
 * @warning             The buffer is not garanteed to be null-terminated
 * 
 * @param c             The given #Cache
 * @param file          The given file
 * @param pos           The given starting position
 * @param buffer        The given buffer
 * @param max_write     The number of bytes to write
 * 
 * @return              The number of written bytes 
 */
int getStr(Cache c, FILE* file, pos_t pos, char buffer[], int max_write) {
    Line l = getCacheLine(c, fileno(file), pos);

    if (l == NULL)
        return 0;

    int line_pos = pos % (pos_t)CACHE_LINE_SIZE, str_len = CACHE_LINE_SIZE - line_pos, write = MIN(str_len, max_write);
    char* str = l->data + line_pos;

    memcpy(buffer, str, write);

    if (write < max_write)
        return write + getStr(c, file, pos + (pos_t)str_len, buffer + str_len, max_write - str_len);

    return write;
}

/**
 * @brief       Gets the integer at the given position of the file
 * 
 * @param c     The #Cache
 * @param file  The given file
 * @param pos   The starting position
 * 
 * @return      The requested integer
 */
int getInt(Cache c, FILE* file, pos_t pos) {
    int ans;
    getStr(c, file, pos, (char*)&ans, sizeof(int));
    return ans;
}


/**
 * @brief       Gets the position at the given position of the file
 * 
 * @param c     The #Cache
 * @param file  The given file
 * @param pos   The starting position
 * 
 * @return      The requested oosition
 */
pos_t getPosT(Cache c, FILE* file, pos_t pos) {
    pos_t ans;
    getStr(c, file, pos, (char*)&ans, sizeof(pos_t));
    return ans;
}

/**
 * @brief           Writes to the cache in the specified file and position
 * 
 * @param c         The #Cache to write to
 * @param file      The specified file
 * @param pos       The specified position
 * @param buffer    The data to write
 * @param write     The number of bytes to write
 */
void setStr(Cache c, FILE* file, pos_t pos, char buffer[], int write)
{
    Line l = getCacheLine(c, fileno(file), pos);

    if (l == NULL) {
        fprintf(stderr, "setStr: error writing %d characters to cache (pos=%llu)\n", write, pos);
        return;
    }

    l->altered = true;
    int line_pos = pos % (pos_t)CACHE_LINE_SIZE, str_len = CACHE_LINE_SIZE - line_pos, write_cur = MIN(str_len, write);
    char* str = l->data + line_pos;

    memcpy(str, buffer, write_cur);

    if (write_cur < write)
        setStr(c, file, pos + (pos_t)str_len, buffer + str_len, write - str_len);
}

/**
 * @brief       Auxiliary function to @ref flushCacheFile
 * 
 *              Checks if a #Line has the altered flag
 * 
 * @param key   The #Key
 * @param line  The #Line
 * @param file  The file descriptor
 * @return      Whether or not the #Line referes the given file
 */
void flushFileAux(gpointer key, gpointer line, gpointer file) {
    if (((Key)key)->file_desc == GPOINTER_TO_INT(file))
        updateCacheLine((Line)line, (Key)key);
}

/**
 * @brief       Writes all altered #Line the #Cache refering to the specified file
 * 
 * @param c     The #Cache
 * @param file  The specified file
 */
void flushCacheFile(Cache c, FILE* file) {
    pthread_mutex_lock(&c->mutex);
    g_hash_table_foreach(c->posLinePairs, flushFileAux, file);
    pthread_mutex_unlock(&c->mutex);
}

/**
 * @brief       Auxiliary function to @ref flushCache
 *              Flushes the given #Cache line
 * 
 * @param key   The #Key
 * @param line  The #Line
 * @param null  Is always NULL. Required by g_hash_table_foreach
 */
void flushAux(gpointer key, gpointer line, gpointer null) {
    updateCacheLine((Line)line, (Key)key);
}

/**
 * @brief   Writes all altered #Line to their files
 * 
 * @param c The #Cache
 */
void flushCache(Cache c) {
    pthread_mutex_lock(&c->mutex);
    g_hash_table_foreach(c->posLinePairs, flushAux, NULL);
    pthread_mutex_unlock(&c->mutex);
}

/**
 * @brief       Auxiliary function to @ref refreshCache
 * 
 *              Checks if a #Line referes the given file
 * 
 * @param key   The #Key
 * @param line  The #Line
 * @param file  The file descriptor
 * @return      Whether or not the #Line referes the given file
 */
gboolean refreshFileAux(gpointer key, gpointer line, gpointer file) {
    return ((Key)key)->file_desc == GPOINTER_TO_INT(file);
}

/**
 * @brief       Clears all the #Line of the #Cache refering to the specified file. Any pending
 *              writes to the lines are deleted
 * 
 * @param c     The given #Cache
 * @param file  The given file
 */
void refreshCacheFile(Cache c, FILE* file) {
    pthread_mutex_lock(&c->mutex);
    g_hash_table_foreach_remove(c->posLinePairs, refreshFileAux, GINT_TO_POINTER(fileno(file)));
    pthread_mutex_unlock(&c->mutex);
}

/**
 * @brief   Clears all #Line of the #Cache. Any pending writes to the lines are deleted
 * 
 * @param c The #Cache
 */
void refreshCache(Cache c) {
    pthread_mutex_lock(&c->mutex);
    g_hash_table_remove_all(c->posLinePairs);
    pthread_mutex_unlock(&c->mutex);
}

/**
 * @brief       Auxiliary function to @ref clearCacheFile
 * 
 *              If #Line referes the given file, flushes and returns true. Otherwise returns false
 * 
 * @param key   The #Key
 * @param line  The #Line
 * @param file  The file descriptor
 * @return      Whether or not the #Line referes the given file
 */
gboolean clearFileAux(gpointer key, gpointer line, gpointer file) {
    if (((Key)key)->file_desc == GPOINTER_TO_INT(file)) {
        updateCacheLine((Line)line, (Key)key);
        return true;
    }

    return false;
}

/**
 * @brief   Clears the content of the #Cache refering to the specified file. Any pending writes are flushed.
 * 
 * @param c The given #Cache
 */
void clearCacheFile(Cache c, FILE* file) {
    pthread_mutex_lock(&c->mutex);
    g_hash_table_foreach_remove(c->posLinePairs, clearFileAux, GINT_TO_POINTER(fileno(file)));
    pthread_mutex_unlock(&c->mutex);
}

/**
 * @brief   Clears the content of the #Cache. Any pending writes are flushed.
 * 
 * @param c The given #Cache
 */
void clearCache(Cache c) {
    pthread_mutex_lock(&c->mutex);
    g_hash_table_foreach(c->posLinePairs, flushAux, NULL);
    g_hash_table_remove_all(c->posLinePairs);
    pthread_mutex_unlock(&c->mutex);
}

/**
 * @brief   Frees the memory allocated to the #Cache. Any pending writes are flushed.
 * 
 * @param c The given #Cache
 */
void freeCache(Cache c) {

    DEBUG_PRINT("Cache line size: %d bytes\n", CACHE_LINE_SIZE);
    DEBUG_PRINT("Cache usage: %d/%d cache lines\n", g_hash_table_size(c->posLinePairs), c->line_num);
    DEBUG_PRINT("Cache hits: %ld\n", c->hits);
    DEBUG_PRINT("Cache misses: %ld\n", c->misses);

    flushCache(c);

    for (int i = 0; i < c->line_num; i++)
        pthread_mutex_destroy(&c->lines[i]->mutex);

    free(c->lines[0]->data);    //all lines are malloc'd together, so one free frees them all
    free(c->lines[0]);          //same here
    free(c->lines);

    pthread_mutex_destroy(&c->mutex);
    g_hash_table_destroy(c->posLinePairs);
    free(c);
}