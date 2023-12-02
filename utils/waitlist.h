// thread_manager.h

#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H

#include <pthread.h>
#include <semaphore.h>

// Define a structure for a thread node
typedef struct ThreadNode {
    int done;               // Flag to indicate if the execution is complete
    pthread_t thread;       // Pointer to the thread
    int algo;               // Scheduling algorithm type: 1 for FCFS, 2 for RR
    int quantum;            // Quantum for RR scheduling algorithm
    int client;             // Client socket number
    int remaining_time;     // Remaining time for execution in seconds
    sem_t semaphore;        // Semaphore for indicating turn on the scheduler
    struct ThreadNode* next; // Pointer to the next node in the linked list
    struct ThreadNode* prev; // Pointer to the previous node in the linked list
} ThreadNode;

// Declare the global waiting list as extern
extern ThreadNode* waiting_list;

// Function declarations
ThreadNode* addNode(pthread_t thread, int client, int remaining_time, int algo, int quantum);
void deleteNode(ThreadNode* node);
void printList();
void* scheduler(void* arg);

#endif // THREAD_MANAGER_H
