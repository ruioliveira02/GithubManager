/**
 * @file lazy.c
 *
 * File containing the implementation of the #Lazy type
 *
 * The #Lazy type is used to minimize reading from files by only reading what is strictly necessary
 */

#include <stdio.h>

#include "io/cache.h"
#include "types/format.h"
#include "types/lazy.h"

/**
 * @brief A type which serves as a wrapper for the access of a formatted object in a file
 *
 */
struct lazy {
    FILE* file;         ///< The file to read
    pos_t pos;          ///< The position in the file where the target data is stored
    Format format;      ///< The #Format of the wrapped type
    void* obj;          ///< Pointer to the object the lazy is the wrapper of
    bool* loaded;       ///< Array. Each bool refers to whether or not that member has been loaded to memory
    bool* altered;      /**< Array. Each bool stored whether that member has been altered by setLazyMember
                        and should be written back to the file when FprintLazyToFile() is called*/
    pos_t* string_pos;  /**< The position of each member in the file. string_pos[format->members] is
                        the position after the end of the object*/
    int string_pos_it;  ///< The last calculated string_pos
    int list_index;     ///< The index of the last read list
};

/**
 * @brief           Creates a #Lazy
 *
 * @param file      The file to read data from
 * @param pos       The starting position in the file
 * @param format    The #Format of the type to read
 * @param obj       The object the lazy is meant to wrap. Assumed to be empty
 *
 * @return          The requested #Lazy
 */
Lazy makeLazy(FILE* file, pos_t pos, Format format, void* obj) {
    if (!isBinary(format)) {
        fprintf(stderr, "makeLazy: format must be binary\n");
        return NULL;
    }

    Lazy l = (Lazy)malloc(sizeof(struct lazy));
    l->file = file;
    l->pos = pos;
    l->format = copyFormat(format);
    l->obj = obj;
    l->loaded = (bool*)calloc(getFormatMembers(l->format), sizeof(bool));
    l->altered = (bool*)calloc(getFormatMembers(l->format), sizeof(bool));
    l->string_pos = (pos_t*)malloc(sizeof(pos_t) * (getFormatMembers(l->format) + 1));
    l->string_pos[0] = pos;
    l->string_pos_it = 0;
    l->list_index = 0;
    return l;
}

/**
 * @brief       Calculates the position of each member in the file up to the given position
 *
 * @param l     The given #Lazy
 * @param pos   The given position
 * @param c     The #Cache
 */
void loadStringPos(Lazy l, int pos, Cache c) {
    while (l->string_pos_it < pos) {
        l->string_pos_it++;
        l->string_pos[l->string_pos_it] = stringSize(getMemberType(l->format, l->string_pos_it - 1));

        if (l->string_pos[l->string_pos_it] == 0) //if the length is unknown it must be a binary list type
            l->string_pos[l->string_pos_it] = *(int*)getLazyMember(l, getListPairLengthMember(l->format, l->list_index++), c)
                * elemStringSize(getMemberType(l->format, l->string_pos_it - 1));

        l->string_pos[l->string_pos_it] += l->string_pos[l->string_pos_it - 1];
    }
}

/**
 * @brief           Gets the requested member of the object
 *
 * @param l         The given #Lazy
 * @param member    The index of the member to get
 * @param c         The #Cache
 *
 * @return          The requested member of the object
 */
void* getLazyMember(Lazy l, int member, Cache c) {
    if (!l->loaded[member]) {
        loadStringPos(l, member + 1, c);
        int length = l->string_pos[member + 1] - l->string_pos[member];
        int stack_length = length > 1000 ? 0 : length;
        char stack_buffer[stack_length];
        char* buffer;

        if (length > 1000)
            buffer = malloc(length);
        else
            buffer = stack_buffer;

        getStr(c, l->file, l->string_pos[member], buffer, length);

        readBinaryMember(getMemberType(l->format, member), buffer, length, getMember(l->format, l->obj, member));
        l->loaded[member] = true;

        if (length > 1000)
            free(buffer);
    }

    return getMember(l->format, l->obj, member);
}

/**
 * @brief Flags the specified member as altered. Returns a pointer to the specified member of the wrapped object.
 *        If not requested previously, the returned member won't loaded from the source file, therefore the returned
 *        pointer shouldn't be read from.
 * 
 * @param l         The #Lazy
 * @param member    The index of the member to set
 * @return          Pointer to the member to set
 */
void* setLazyMember(Lazy l, int member)
{
    l->loaded[member] = l->altered[member] = true;
    return getMember(l->format, l->obj, member);
}

/**
 * @brief           Gets the position of a member of the type in the file
 *
 * @param l         The #Lazy
 * @param member    The given member
 * @param c         The #Cache
 *
 * @return          The position of the requested member in the file
 */
pos_t getPosOfLazyMember(Lazy l, int member, Cache c) {
    loadStringPos(l, member, c);
    return l->string_pos[member];
}

/**
 * @brief Gets the position after the read object
 *
 * @param l         The given #Lazy
 * @param c         The #Cache
 *
 * @return          The position after the last read object
 */
pos_t getPosAfterLazy(Lazy l, Cache c) {
    return getPosOfLazyMember(l, getFormatMembers(l->format), c);
}

/**
 * @brief Writes the altered members of the #Lazy to the source file
 * 
 * @param l     The #Lazy
 * @param c     The #Cache 
 * 
 */
void printLazyToFile(Lazy l, Cache c)
{
    int list_index = -1;

    for (int i = 0; i < getFormatMembers(l->format); i++)
    {
        if (l->altered[i])
        {
            FormatType t = getMemberType(l->format, i);
            int length = stringSize(t);
            
            if (length == 0) {
                while (getListPairListMember(l->format, list_index) < i)
                    list_index++;
                length = *(int*)getLazyMember(l, getListPairLengthMember(l->format, list_index), c) * elemStringSize(t);
            }

            int stack_length = length > 1000 ? 0 : length;
            char stack_buffer[stack_length];
            char *buffer;

            if (length > 1000)
                buffer = malloc(length);
            else
                buffer = stack_buffer;

            writeBinaryMember(t, getMember(l->format, l->obj, i), buffer, length);
            setStr(c, l->file, l->string_pos[i], buffer, length);

            if (length > 1000)
                free(buffer);
        }
    }
}

/**
 * @brief Sets the object wrapped by the lazy. If the lazy already wrapped an object, the old object's contents are freed
 * 
 * @param l     The #Lazy
 * @param obj   The new object to wrap
 */
void setLazyObj(Lazy l, void* obj)
{
    if (l->obj != NULL)
        for (int i = 0; i < getFormatMembers(l->format); i++)
            if (l->loaded[i])
                freeMember(l->format, l->obj, i);
    
    memset(l->loaded, 0, getFormatMembers(l->format) * sizeof(bool));
    memset(l->altered, 0, getFormatMembers(l->format) * sizeof(bool));
    l->obj = obj;
}

/**
 * @brief Sets the underlying address of the #Lazy to the given file and position. Any members already loaded
 *        by the lazy are freed
 * 
 * @param l     The #Lazy
 * @param file  The given file
 * @param pos   The given position
 */
void setLazyAddress(Lazy l, FILE* file, pos_t pos)
{
    if (l->obj != NULL)
        for (int i = 0; i < getFormatMembers(l->format); i++)
            if (l->loaded[i])
                freeMember(l->format, l->obj, i);

    memset(l->loaded, 0, getFormatMembers(l->format) * sizeof(bool));
    memset(l->altered, 0, getFormatMembers(l->format) * sizeof(bool));
    l->file = file;
    l->pos = pos;
    l->string_pos[0] = pos;
    l->string_pos_it = 0;
    l->list_index = 0;
}

/**
 * @brief       Frees the memory allocated to a #Lazy
 *
 * @param l     The #Lazy to free
 */
void freeLazy(Lazy l)
{
    for (int i = 0; i < getFormatMembers(l->format); i++)
        if (l->loaded[i])
            freeMember(l->format, l->obj, i);

    disposeFormat(l->format);
    free(l->loaded);
    free(l->altered);
    free(l->string_pos);
    free(l);
}
