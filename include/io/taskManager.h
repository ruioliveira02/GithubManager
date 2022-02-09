/**
 * @file taskManager.h
 * 
 * File containing declaration of function used to add multi threading to IO operations
 */

#ifndef _TASK_MANAGER_H_

/**
 * @brief Include guard
 */
#define _TASK_MANAGER_H_


#include <stdlib.h>
#include <stdio.h>


/**
 * @brief Calculates the number of parameters passed
 */
#define LENGTH(...) (sizeof((void *[]){__VA_ARGS__}) / sizeof(void *))

/**
 * @brief Helper of SEQ. packages a single function
 */
#define FUNC(f, ...) f, (void*)LENGTH(__VA_ARGS__), __VA_ARGS__

/**
 * @brief Used to package arguments into a void pointer, to help the call to function sequence.
 *        For example: SEQ(FUNC(readName, &aux), FUNC(write, "hello ", &aux)), when passed to sequence will call
 *        the function readName with parameter &aux, followed by function write with parameters "hello " and &aux
 *        
 *        IMPORTANT: all parameters are resolved before the call of the first call. This means that if a variable passed
 *        by reference is changed by a function, subsequent functions will stil receive the old value. To get around this,
 *        allways pass that variable by reference to following functions.
 */
#define SEQ(...) (void*)(void*[]){ (void*)LENGTH(__VA_ARGS__), __VA_ARGS__ }


/**
 * @brief Defines the function prototype accepted by the function sequence for sequencing functions as threads
 *        Functions that follow this prototype take as parameters an array of void pointers and return nothing (void)
*/
typedef void (*routine)(void**);


void* sequence(void*);

void executeTasks(void* taskList[], int tasks, void* catalog, void (*solver)(int, void*, void*), int threads) ;

#endif