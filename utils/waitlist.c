#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#include "waitlist.h"

// // Global variables
// ThreadNode* waiting_list = NULL; // Head of the global waiting list

// Function to add a new thread to the waiting list
ThreadNode* addNode(pthread_t thread, int client, int remaining_time, int algo, int quantum) {
    ThreadNode* new_node = (ThreadNode*)malloc(sizeof(ThreadNode));
    new_node->done = 0;
    new_node->thread = thread;
    new_node->client = client;
    new_node->algo = algo;
    new_node->quantum = quantum;
    new_node->remaining_time = remaining_time;
    sem_init(&(new_node->semaphore), 0, 0);
    new_node->next = NULL;
    new_node->prev = NULL;

    // Add the new node to the end of the linked list
    if (waiting_list == NULL) {
        waiting_list = new_node;
    } else {
        ThreadNode* current = waiting_list;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
        new_node->prev = current;
    }

    return new_node;
}

// Function to delete a node from the waiting list
void deleteNode(ThreadNode* node) {
    if (node == NULL) {
        return; // Do nothing if the node is NULL
    }

    if (node->prev != NULL) {
        node->prev->next = node->next;
    } else {
        waiting_list = node->next;
    }

    if (node->next != NULL) {
        node->next->prev = node->prev;
    }

    free(node);
}

// Function to execute the threads in the waiting list
void* scheduler(void* arg) {
    while (1) {
        ThreadNode* current = waiting_list;

        while (current != NULL) {
            // Wait for the semaphore to be signaled (indicating turn on the scheduler)
            sem_wait(&(current->semaphore));

            // Execute the thread for 1 second
            // (Note: For simplicity, we use sleep here, but in a real-world scenario,
            // you may want to use a more precise timer mechanism.)
            sleep(1);
            current->remaining_time--;

            // If the thread has finished execution, remove it from the waiting list
            if (current->remaining_time <= 0) {
                printf("Thread %lu finished execution\n", current->thread);
                ThreadNode* temp = current;
                current = current->next;

                if (temp->prev != NULL) {
                    temp->prev->next = temp->next;
                } else {
                    waiting_list = temp->next;
                }

                if (temp->next != NULL) {
                    temp->next->prev = temp->prev;
                }

                free(temp);
            } else {
                // Move to the next node in the linked list
                current = current->next;
            }
        }
    }
    return NULL;
}