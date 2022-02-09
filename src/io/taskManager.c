/**
 * @file taskManager.c
 * 
 * File containing the functions used to control multi threading
 */

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "io/taskManager.h"
#include "utils/utils.h"


/**
 * @brief           Used with pthread_create to sequencially call multiple functions
 *
 * @param arg_list  A pointer to a void pointer array, containing all the information required to call the functions.
 *                  See documentation of macro SEQ for more info
 * 
 * @return          Always returns NULL (required by pthread_create thread_start prototype).
 */
void* sequence(void* arg_list) {

    void** args = (void**)arg_list;
    long n = (long)args[0],inc;

    for (int i = 1; i <= n; ) {

        routine r = (routine)args[i++];
        inc = (long)args[i++];
        r(args + i);
        i += inc;
    }
    return NULL;
}

/**
 * @brief fetches the next task to executed
 * 
 * @param p List of the list of tasks their current state,what task is being executed,the state and the routine solver (void* taskList[], int *tasks,  int *currentTask, void* state, routine solver)
 * 
 * @return NULL
 */
void* fetchTasks(void* p) {
    void** params = (void**)p;

    void** taskList = (void**)params[0];
    int tasks = *(int*)params[1];
    int *task = (int*)params[2];
    void*catalog = params[3];
    void (*solver)(int, void*, void*) = params[4];
    pthread_mutex_t* mutex = (pthread_mutex_t*)params[5];
    bool running = true;

    while (running) {
        pthread_mutex_lock(mutex);
        int current_task = (*task)++;
        pthread_mutex_unlock(mutex);

        if (current_task >= tasks)
            running = false;
        else
            solver(current_task, taskList[current_task],catalog);
    }

    return NULL;
}

/**
 * @brief           Executes a task
 * 
 * @param taskList  List of all tasks to be executed
 * @param tasks     Number of the task to execute
 * @param state     Current state of the task
 * @param solver    Solver to the query
 * @param threads   Number of threads avaiable
 */
void executeTasks(void* taskList[], int tasks, void* catalog, void (*solver)(int, void*, void*), int threads) {

    if (tasks < threads)
        threads = tasks;

    pthread_t threadArr[threads];
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    int task = 0;
    void* params[6] = { taskList, &tasks, &task, catalog, solver, &mutex };

    for (int i = 0; i < threads; i++) {
        pthread_create(&threadArr[i], NULL, fetchTasks, params);
        sleep(1);
    }
        

    for (int i = 0; i < threads; i++)
        pthread_join(threadArr[i], NULL);

    pthread_mutex_destroy(&mutex);
}