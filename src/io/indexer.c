/**
 * @file indexer.c
 * 
 * File containing the implementation of the #Indexer type
 * 
 */


/**
 * @brief Allow the usage of the function qsort_r
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include "io/cache.h"
#include "io/indexer.h"


/**
 * @brief The maximum number of lines that will be copied to RAM at once (128MB)
 */
#define MAX_FILE_LINES 8388608 //128MB (size of each file is 8388608 * size of each line (sizeof(LINE)))

/**
 * @brief Defines a line of the indexer file: the key and the position that key maps to
 */
typedef struct line {
    pos_t key;     ///< The key: position in the key file (if it is NULL, it is embedded)
    pos_t value;   ///< The value: position in the value file (if it is NULL, it is embedded)
} LINE;

/**
 * @brief Structure representing an index of the application
 */
struct indexer {
    FILE* index;                                        ///< The index file
    int elem_no;                                        ///< The number of lines stored in the index file
    char* index_name;                                   ///< The name of the index file (if NULL it is a temporary file deleted on program exit)
    bool changed_since_cache_refresh;                   ///< Whether or not the index has changed since the #Cache has refreshed

    FILE* keys;                                         ///< The file containing the keys
    int (*cmpKeys)(FILE*, pos_t, FILE*, pos_t, Cache);  ///< The function used to compare keys

    FILE* values;                                       ///< The file containing the values
    FILE* grouped_values;                               ///< The file containing the values grouped by key
};

/**
 * @brief Represents a min heap of the lines in the file. The lines are indexed by their key
 * 
 */
typedef struct heap {
    LINE* values;           ///< The array of values of the heap
    int* heap;              ///< The indices of the values stored
    int size;               ///< The number of elements in the heap
} HEAP, * Heap;

/**
 * @brief       Heapifies the #Heap (sorts the #Heap)
 * 
 * @param h     The given #Heap
 * @param i     The given #Indexer (used to compare keys)
 * @param c     The #Cache to use when comparing keys
 * @param elem  The current position in the heap (initially zero)
 */
void heapify(Heap h, Indexer i, Cache c, int elem) {
    int l = 2 * elem + 1;
    int r = 2 * elem + 2;
    int smallest = elem;
    if (l < h->size && i->cmpKeys(i->keys, h->values[h->heap[l]].key, i->keys, h->values[h->heap[smallest]].key, c) < 0)
        smallest = l;
    if (r < h->size && i->cmpKeys(i->keys, h->values[h->heap[r]].key, i->keys, h->values[h->heap[smallest]].key, c) < 0)
        smallest = r;
    if (smallest != elem)
    {
        int aux = h->heap[elem];
        h->heap[elem] = h->heap[smallest];
        h->heap[smallest] = aux;
        heapify(h, i, c, smallest);
    }
}

/**
 * @brief       Returns the element with the smallest key stored in the #Heap
 * 
 * @param h     The given #Heap
 * @param i     The given #Indexer
 * @param c     The #Cache to use when comparing keys
 * 
 * @return      The element with the smallest key stored in the #Heap
 */
int popHeap(Heap h, Indexer i, Cache c) {
    int ans = h->heap[0];
    if (--h->size > 0)
    {
        h->heap[0] = h->heap[h->size];
        heapify(h, i, c, 0);
    }

    return ans;
}

/**
 * @brief       Inserts the given value into the #Heap
 * 
 * @param h     The given #Heap
 * @param i     The given #Indexer
 * @param c     The #Cache to use when comparing keys
 * @param val   The given value
 */
void pushHeap(Heap h, Indexer i, Cache c, int val) {
    int pos = h->size++;
    h->heap[pos] = val;

    // Fix the min heap property if it is violated
    int parent = (pos - 1) / 2;
    while (pos != 0 && i->cmpKeys(i->keys, h->values[h->heap[parent]].key, i->keys, h->values[h->heap[pos]].key, c) > 0)
    {
        int aux = h->heap[pos];
        h->heap[pos] = h->heap[parent];
        h->heap[parent] = aux;

        pos = parent;
        parent = (pos - 1) / 2;
    }
}

/**
 * @brief An #Indexer and a #Cache
 */
typedef struct indexerCachePair
{
    Indexer indexer;    ///< The #Indexer
    Cache cache;        ///< The #Cache
} INDEXERCACHEPAIR, * IndexerCachePair;

/**
 * @brief       Compares two #Line based on their keys
 * 
 * @param a     The first #Line
 * @param b     The second #Line
 * @param p     A pair of #Indexer and #Cache
 * 
 * @return < 0  If a < b
 * @return 0    If a == b
 * @return > 0  If a > b
 */
int compareLines(const void* a, const void* b, void* p) {
    IndexerCachePair pair = (IndexerCachePair)p;
    return pair->indexer->cmpKeys(pair->indexer->keys, ((LINE*)a)->key,
           pair->indexer->keys, ((LINE*)b)->key, pair->cache);
}

/**
 * @brief               Creates an #Indexer
 * 
 * @param index_file    The file to write the index to
 * @param keys          The file containing the keys
 * @param values        The file containing the values
 * @param cmpKeys       The function which compares two keys
 * 
 * @return              The requested #Indexer
 */
Indexer makeIndexer(char* index_file, FILE* keys, FILE* values, int (*cmpKeys)(FILE*, pos_t, FILE*, pos_t, Cache)) {
    Indexer i = malloc(sizeof(struct indexer));
    i->index = index_file == NULL ? tmpfile() : OPEN_FILE(index_file, "wb+");
    i->elem_no = 0;
    i->index_name = index_file == NULL ? NULL : strdup(index_file);
    i->changed_since_cache_refresh = false;
    i->keys = keys;
    i->cmpKeys = cmpKeys;
    i->values = values;
    i->grouped_values = NULL;
    return i;
}

/**
 * @brief               Reads an existing #Indexer from file
 * 
 * @note                If the #Indexer does not exist, it creates a new one
 * 
 * @param index_file    The file to read the index from
 * @param keys          The file containing the keys
 * @param values        The file containing the values
 * @param cmpKeys       The function which compares two keys
 * 
 * @return              The requested #Indexer
 */
Indexer parseIndexer(char* index_file, FILE* keys, FILE* values, int (*cmpKeys)(FILE*, pos_t, FILE*, pos_t, Cache)) {
    Indexer i = malloc(sizeof(struct indexer));
    i->keys = keys;
    i->cmpKeys = cmpKeys;
    i->values = values;
    i->grouped_values = NULL;
    i->index_name = index_file == NULL ? NULL : strdup(index_file);
    i->changed_since_cache_refresh = false;

    if (index_file == NULL)
        i->index = tmpfile();
    else
        i->index = OPEN_MAYBE_FILE(index_file, "b");

    fseek(i->index, 0, SEEK_END);
    i->elem_no = ftell(i->index) / sizeof(LINE);

    return i;
}

/**
 * @brief                   Reads an existing #Indexer containing grouped values from file
 * 
 * @param index_file        The path to the file to read the index from
 * @param values            The file containing the values
 * @param keys              The file containing the keys
 * @param grouped_values    The file containing the grouped values
 * @param cmpKeys           The function which compares two keys
 * 
 * @return                  The requested #Indexer
 */
Indexer parseGroupedIndexer(char* index_file, char* values, FILE* keys, FILE* grouped_values,
                            int (*cmpKeys)(FILE*, pos_t, FILE*, pos_t, Cache)) {

    Indexer i = malloc(sizeof(struct indexer));
    i->keys = keys;
    i->cmpKeys = cmpKeys;
    i->values = OPEN_FILE(values, "rb");
    i->grouped_values = grouped_values;
    i->index = OPEN_FILE(index_file, "rb+");
    i->index_name = index_file == NULL ? NULL : strdup(index_file);
    i->changed_since_cache_refresh = false;

    fseek(i->index, 0, SEEK_END);
    i->elem_no = ftell(i->index) / sizeof(LINE);

    return i;
}

/**
 * @brief   Flushes the #Indexer, i.e., if you have changed the index file, the #Cache is refreshed
 * 
 * @param i The given #Indexer
 * @param c The #Cache to refresh
 */
void flushIndex(Indexer i, Cache c) {
    if (i->changed_since_cache_refresh) {
        fflush(i->index);
        refreshCacheFile(c, i->index);
        i->changed_since_cache_refresh = false;
    }
}

/**
 * @brief       Inserts the given key-value pair into an #Indexer
 * 
 * @param i     The given #Indexer
 * @param key   The given key
 * @param value The given value
 */
void insertIntoIndex(Indexer i, pos_t key, pos_t value) {
    fseek(i->index, i->elem_no * sizeof(LINE), SEEK_SET);
    LINE l = { .key = key, .value = value };
    fwrite(&l, sizeof(LINE), 1, i->index);
    i->changed_since_cache_refresh = true;
    i->elem_no++;
}

/**
 * @brief   Sorts the #Indexer
 * 
 * @param i The given #Indexer
 * @param c The #Cache to use when comparing keys
 */
void sortIndexer(Indexer i, Cache c) {
    fseek(i->index, 0, SEEK_SET);
    fflush(i->index);
    pos_t size, k = (i->elem_no + MAX_FILE_LINES - 1) / MAX_FILE_LINES;
    LINE* buffer = malloc(i->elem_no / k * sizeof(LINE));
    FILE** tmp_files = malloc(k * sizeof(FILE*));
    int j = 0;

    for (pos_t l = 0; l < i->elem_no; l += size) {
        tmp_files[j] = tmpfile();
        size = i->elem_no / k + (i->elem_no % k > j ? 1 : 0);

        int read = fread(buffer, sizeof(LINE), size, i->index);
        if (read != size)
            fprintf(stderr, "sortIndexer: unexpected number of characters read (read: %d; expected: %lld)\n", read, size);
        
        INDEXERCACHEPAIR p = { .indexer = i, .cache = c };
        qsort_r(buffer, size, sizeof(LINE), compareLines, &p);

        fwrite(buffer, sizeof(LINE), size, tmp_files[j++]);
    }
    
    HEAP h;
    h.heap = malloc(k * sizeof(int));
    h.values = malloc(k * sizeof(LINE));
    h.size = 0;

    for (int j = 0; j < k; j++) {
        fseek(tmp_files[j], 0, SEEK_SET);
        if (fread(&h.values[j], sizeof(LINE), 1, tmp_files[j]))
            pushHeap(&h, i, c, j);
    }

    fseek(i->index, 0, SEEK_SET);

    while (h.size) {
        //TODO: function replaceHeap to speed up pop + push
        int branch = popHeap(&h, i, c);
        fwrite(&h.values[branch], sizeof(LINE), 1, i->index);
        
        if (fread(&h.values[branch], sizeof(LINE), 1, tmp_files[branch]))
            pushHeap(&h, i, c, branch);
    }

    for (int j = 0; j < k; j++)
        fclose(tmp_files[j]);

    i->changed_since_cache_refresh = true;
    free(tmp_files);
    free(h.values);
    free(h.heap);
    free(buffer);
}

/**
 * @brief       Auxiliary function to @ref removeDuplicatesAux
 * 
 *              Compares two ::pos_t
 * 
 * @param a     The first position
 * @param b     The second position
 * 
 * @return -1   If a < b
 * @return 0    If a == b
 * @return > 0  If a > b
 */
int compareAux(const void* a, const void* b) {
    return *(pos_t*)b > *(pos_t*)a ? -1 : *(pos_t*)b < *(pos_t*)a;
}

/**
 * @brief Auxiliary function to @ref groupIndexer. Removes the duplicates from the given array of positions
 * 
 * @param f         The given file
 * @param pos       The starting position in the file
 * @param elems     The number of elements in a block
 * @param aux       The auxiliary array
 * @param aux_size  The number of elements in the array
 * 
 * @return          The new size of the array
 */
int removeDuplicatesAux(FILE* f, pos_t pos, int elems, pos_t** aux, int *aux_size) {
    if (*aux == NULL) {
        *aux = malloc(sizeof(pos_t) * elems);
        *aux_size = elems;
    } else if (*aux_size < elems) {
        *aux_size = MAX(elems, *aux_size * 1.5);
        *aux = realloc(*aux, sizeof(pos_t) * *aux_size);
    }

    fseek(f, pos, SEEK_SET);
    int read = fread(*aux, sizeof(pos_t), elems, f);
    if (read != elems)
        fprintf(stderr, "removeDuplicatesAux: unexpected number of objects read (read: %d; expected: %d)\n", read, elems);
    qsort(*aux, elems, sizeof(pos_t), compareAux);

    int last = 0;

    for (int i = 1; i < elems; i++)
        if ((*aux)[i] != (*aux)[last])
            (*aux)[++last] = (*aux)[i];

    fseek(f, pos, SEEK_SET);
    fwrite(*aux, sizeof(pos_t), last + 1, f);
    return last + 1;
}

/**
 * @brief                       Groups an #Indexer, i.e., joins all elements with the same key in the same group
 * 
 * @warning                     #Indexer must be sorted
 * 
 * @note                        Decreases the size of #Indexer
 * 
 * @param i                     The given #Indexer 
 * @param value_file            The path to the file that is to contain the values
 * @param removeDuplicateVals   Whether or not to remove duplicated values
 * @param c                     The #Cache used when comparing keys
 */
void groupIndexer(Indexer i, char* value_file, bool removeDuplicateVals, Cache c) {
    FILE *dest, *out = value_file == NULL ? tmpfile() : OPEN_FILE(value_file, "wb+");
    char* dest_name = NULL;

    if (i->index_name == NULL)
        dest = tmpfile();
    else {
        int filename_size = strlen(i->index_name);
        dest_name = malloc(filename_size + 5);
        sprintf(dest_name, "%s.tmp", i->index_name);
        dest = OPEN_FILE(dest_name, "wb+");
    }

    if (i->elem_no != 0) {
        LINE l;
        int block_no = 1, block_length = 1;

        pos_t out_block = 0;
        pos_t out_pos = sizeof(int);

        pos_t* aux = NULL;
        int aux_size;

        fflush(i->index);
        fseek(i->index, 0, SEEK_SET);
        fseek(out, sizeof(int), SEEK_SET);

        int read = fread(&l, sizeof(LINE), 1, i->index);
        if (read != 1)
            fprintf(stderr, "groupIndexer: unexpected number of objects read (read: %d; expected: 1)\n", read);
        LINE l_out = { .key = l.key, .value = out_block };
        fwrite(&l_out, sizeof(LINE), 1, dest);
        pos_t last_key = l.key;
        fwrite(&l.value, sizeof(pos_t), 1, out);
        out_pos += sizeof(pos_t);

        while (fread(&l, sizeof(LINE), 1, i->index)) {
            int cmp = i->cmpKeys(i->keys, l.key, i->keys, last_key, c);
            if (cmp < 0)
                fprintf(stderr, "groupIndexer: indexer must be sorted\n");

            if (cmp == 0)
                block_length++;
            else {
                if (removeDuplicateVals) {
                    block_length = removeDuplicatesAux(out, out_block + sizeof(int), block_length, &aux, &aux_size);
                    out_pos = out_block + sizeof(int) + sizeof(pos_t) * (pos_t)block_length;
                }

                fseek(out, out_block, SEEK_SET);
                fwrite(&block_length, sizeof(int), 1, out);
                out_block = out_pos;
                out_pos += sizeof(int);
                fseek(out, out_pos, SEEK_SET);

                l_out = (LINE){ .key = l.key, .value = out_block };
                fwrite(&l_out, sizeof(LINE), 1, dest);
                last_key = l.key;
                block_length = 1;
                block_no++;
            }

            fwrite(&l.value, sizeof(pos_t), 1, out);
            out_pos += sizeof(pos_t);
        }

        if (removeDuplicateVals) {
            block_length = removeDuplicatesAux(out, out_block + sizeof(int), block_length, &aux, &aux_size);
            out_pos = out_block + sizeof(int) + sizeof(pos_t) * (pos_t)block_length;
        }

        fseek(out, out_block, SEEK_SET);
        fwrite(&block_length, sizeof(int), 1, out);

        i->elem_no = block_no;
        free(aux);
    }

    fclose(i->index);

    if (i->index_name == NULL)
        i->index = dest;
    else {
        fclose(dest);
        remove(i->index_name);
        rename(dest_name, i->index_name);
        i->index = OPEN_FILE(i->index_name, "rb+");
    }

    i->changed_since_cache_refresh = true;
    i->grouped_values = i->values;
    i->values = out;
    fflush(out);
    free(dest_name);
}

/**
 * @brief       Gets the number of elements of an #Indexer
 * 
 * @note        If applied to a grouped #Indexer, returns the number of groups instead
 * 
 * @param i     The given #Indexer
 * 
 * @return      The number of elements/groups of the #Indexer
 */
int getElemNumber(Indexer i) {
    return i->elem_no;
}

/**
 * @brief       Returns the position of the key in the list of keys
 * 
 * @param i     The given #Indexer
 * @param key   The given key
 * @param c     The #Cache
 * 
 * @return -1   If the key was not found
 * @return      The position of the key (0-indexed)
 */
int retrieveKey(Indexer i, pos_t key, Cache c) {
    flushIndex(i, c);

    if (i->elem_no == 0)
        return -1;  //NOT FOUND

    int l = 0, r = i->elem_no - 1, m;
    pos_t aux;

    while (m = (l + r) / 2, l < r) {
        aux = getPosT(c, i->index, m * sizeof(LINE));
        int cmp = i->cmpKeys(NULL, key, i->keys, aux, c);

        if (cmp < 0)
            r = m - 1;
        else if (cmp > 0)
            l = m + 1;
        else
            r = m;
    }

    aux = getPosT(c, i->index, l * sizeof(LINE));
    if (i->cmpKeys(NULL, key, i->keys, aux, c) == 0)
        return l;
    else
        return -1;  //NOT FOUND
}

/**
 * @brief       Retrieves a lower bound of the position of the given key
 * 
 *              This means if the key exists, returns the key itself and if it doesn't returns the
 *              position of the key immediately after
 * 
 * @param i     The given #Indexer
 * @param key   The given key
 * @param c     The #Cache
 * 
 * @return      The smallest position lower-bounded by the given key
 */
int retrieveKeyLowerBound(Indexer i, pos_t key, Cache c) {

    flushIndex(i, c);

    if (i->elem_no == 0)
        return 0;

    int l = 0, r = i->elem_no - 1, m;
    pos_t aux;

    while (m = (l + r) / 2, l < r) {

        aux = getPosT(c, i->index, m * sizeof(LINE));
        int cmp = i->cmpKeys(NULL, key, i->keys, aux, c);

        if (cmp < 0)
            r = m - 1;
        else if (cmp > 0)
            l = m + 1;
        else
            r = m;
    }

    aux = getPosT(c, i->index, l * sizeof(LINE));
    if (i->cmpKeys(NULL, key, i->keys, aux, c) <= 0)
        return l;
    else
        return l + 1;
}

/**
 * @brief               Returns the key in the given position 
 * 
 * @param i             The given #Indexer
 * @param key_order     The given position
 * @param c             The #Cache
 * 
 * @return              The key in the given position
 */
pos_t retrieveEmbeddedKey(Indexer i, int key_order, Cache c)
{
    if (key_order < 0 || key_order >= i->elem_no) {
        fprintf(stderr, "retrieveEmbeddedKey: key_order %d is out of bounds\n", key_order);
        return 0;
    }

    flushIndex(i, c);
    return getPosT(c, i->index, key_order * sizeof(LINE));
}

/**
 * @brief               Returns the embedded value in the given position 
 * 
 * @param i             The given #Indexer
 * @param key_order     The given position
 * @param c             The #Cache
 * 
 * @return              The embedded value in the given position
 */
pos_t retrieveEmbeddedValue(Indexer i, int key_order, Cache c)
{
    if (key_order < 0 || key_order >= i->elem_no) {
        fprintf(stderr, "retrieveEmbeddedValue: key_order %d is out of bounds\n", key_order);
        return 0;
    }

    flushIndex(i, c);
    return getPosT(c, i->index, key_order * sizeof(LINE) + sizeof(pos_t));
}

/**
 * @brief               Returns the value in the given position and stores its position
 *                      in the given #Lazy
 * 
 * @param i             The given #Indexer
 * @param key_order     The given position
 * @param c             The #Cache
 * @param dest          The destination #Lazy. Mustn't be NULL
 * 
 * @return              The value in the given position
 */
void retrieveValueAsLazy(Indexer i, int key_order, Cache c, Lazy dest)
{
    if (key_order < 0 || key_order >= i->elem_no) {
        fprintf(stderr, "retrieveValueAsLazy: key_order %d is out of bounds\n", key_order);
        return;
    }

    pos_t pos = retrieveEmbeddedValue(i, key_order, c);
    setLazyAddress(dest, i->values, pos);
}

/**
 * @brief               Gets the embedded value mapped by the given key
 * 
 * @param i             The #Indexer to search
 * @param key           The given #Key
 * @param c             The #Cache
 * 
 * @return              The embedded value mapped by the given key
 */
pos_t getEmbeddedValue(Indexer i, pos_t key, Cache c) {
    int key_order = retrieveKey(i, key, c);
    return retrieveEmbeddedValue(i, key_order, c);
}

/**
 * @brief           Searches for the embedded value mapped by the given key
 * 
 * @param i         The #Indexer to search
 * @param key       The given #Key 
 * @param c         The #Cache
 * @param ans       The pointer to where, if found, the value is placed
 * @return          Whether the value was found
 */
bool findEmbeddedValue(Indexer i, pos_t key, Cache c, pos_t* ans) {
    int key_order = retrieveKey(i, key, c);
    if (key_order == -1)
        return false;
    *ans = retrieveEmbeddedValue(i, key_order, c);
    return true;
}

/**
 * @brief               Searches for the embedded value mapped by the given key and stores its position
 *                      in the given #Lazy
 * 
 * @param i             The given #Indexer
 * @param key           The given key
 * @param c             The #Cache
 * @param dest          The destination #Lazy. Mustn't be NULL
 * 
 * @return              The embedded value in the given position
 */
bool findValueAsLazy(Indexer i, pos_t key, Cache c, Lazy dest)
{
    if (i->values == NULL) {
        fprintf(stderr, "getValueAsLazy: the value mustn't be embedded\n");
        return false;
    }

    pos_t pos;
    if (findEmbeddedValue(i, key, c, &pos)) {
        setLazyAddress(dest, i->values, pos);
        return true;
    }
    return false;
}

/**
 * @brief           Gets the size of the given group
 * 
 * @param i         The given #Indexer
 * @param group     The given group
 * @param c         The #Cache
 * 
 * @return          The size of the group 
 */
int getGroupSize(Indexer i, pos_t group, Cache c)
{
    if (i->values == NULL) {
        fprintf(stderr, "getGroupSize: the value mustn't be embedded\n");
        return 0;
    }

    return getInt(c, i->values, group);
}

/**
 * @brief           Gets the element in the given position of the given group
 * 
 * @param i         The given #Indexer
 * @param group     The given group
 * @param elem      The index of the element
 * @param c         The #Cache
 * 
 * @return          The requested element
 */
pos_t getGroupElem(Indexer i, pos_t group, int elem, Cache c)
{
    if (i->values == NULL) {
        fprintf(stderr, "getGroupElem: the value mustn't be embedded\n");
        return 0;
    }

    return getPosT(c, i->values, group + sizeof(int) + sizeof(pos_t) * (pos_t)elem);
}

/**
 * @brief           Gets the element in the given position of the given group as a #Lazy
 * 
 * @param i         The given #Indexer
 * @param group     The given group
 * @param elem      The index of the element
 * @param c         The #Cache
 * @param dest      The destination #Lazy
 * 
 * @return          The requested element
 */
void getGroupElemAsLazy(Indexer i, pos_t group, int elem, Cache c, Lazy dest)
{
    if (i->grouped_values == NULL) {
        fprintf(stderr, "getGroupElemAsLazy: the grouped value mustn't be embedded\n");
        return;
    }

    pos_t pos = getGroupElem(i, group, elem, c);
    setLazyAddress(dest, i->grouped_values, pos);
}

/**
 * @brief   Flushes the index to the given cache and frees the memory allocated to an #Indexer
 * 
 * @warning Does not close the keys and the values files
 * 
 * @param i The given #Indexer
 * @param c The #Cache to flush to
 */
void freeIndexer(Indexer i, Cache c) {
    if (i->grouped_values != NULL)
        fclose(i->values);

    flushIndex(i, c);
    fclose(i->index);
    free(i->index_name);
    free(i);
}