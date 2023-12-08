#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>

#include "waitlist.h"
#include "../globals.h"


// #define FIRST_ROUND_QUANTUM 3
// #define REMAINING_ROUND_QUANTUM 7

// // Global variables
// ThreadNode* waiting_list = NULL; // Head of the global waiting list

// Function to add a new thread to the waiting list
ThreadNode* addNode(NodeList* list, pthread_t thread, int client, int remaining_time, int algo, int quantum) {
    ThreadNode* new_node = (ThreadNode*)malloc(sizeof(ThreadNode));
    new_node->done = 0;
    new_node->round = 1;
    new_node->thread = thread;
    new_node->client = client;
    new_node->algo = algo;
    new_node->quantum = quantum;
    new_node->remaining_time = remaining_time;
    // every node has a semaphore which starts at 0, by default all the nodes are paused
    // the scheduler will post this semaphore indicating its turn to process
    sem_init(&(new_node->semaphore), 0, 0);
    sem_init(&(new_node->preempt_sm), 0, 0);
    new_node->next = NULL;
    new_node->prev = NULL;

    // Add the new node to the end of the linked list
    if (list->head == NULL) {
        list->head = new_node;
        list->tail = new_node;
    } else {
        new_node->prev = list->tail;
        list->tail->next = new_node;
        list->tail = new_node;
    }
    return new_node;
}

// Function to delete a node from the waiting list
void deleteNode(NodeList* list, ThreadNode* node) {
    if ((list == NULL) | (list->head == NULL) | (node == NULL)) {
        return; // Do nothing if the node is NULL
    }

    // If the node to be deleted is the head node
    if (node->prev == NULL){
        list->head = node->next;
    }

    // If the node to be deleted is the tail node
    if (node->next == NULL){
        list->tail = node->prev;
    }

    // Adjusting pointers of the previous and next nodes
    if (node->prev != NULL) {
        node->prev->next = node->next;
    }
    if (node->next != NULL) {
        node->next->prev = node->prev;
    }
    // Free the memory occupied by the node
    free(node);
}

// function to print the waiting list
void printList() {
    ThreadNode* current = waiting_list.head;
    if (current == NULL) {
        printf("Waiting list is empty\n");
        return;
    }
    while (current != NULL) {
        printf("Thread %lu, remaining time: %d\n", current->thread, current->remaining_time);
        current = current->next;
    }
}

sem_t* curr_preempt_sm;

volatile sig_atomic_t timer_running = 0;

void start_timer(int seconds) {
    alarm(seconds);
    timer_running = 1;
}

void stop_timer() {
    alarm(0);
    timer_running = 0;
}

void signal_handler(int signum) {
	// posts the semaphore to stop the process preemptively
	sem_post(curr_preempt_sm);
	stop_timer();
}


void* scheduler_thread (void* args) {
    // Set up signal handlers
    signal(SIGALRM, signal_handler);
	ThreadNode* current = NULL;

	while (1) {
		// wait for this semaphore to be available, to continue with the control flow
		// this becomes available when a program finishes execution
		sem_wait(&(continue_semaphore));
		if (current && current->done == 1) {
			stop_timer();
			deleteNode(&waiting_list, current);
		}

		// if timer running, stop it and post preempt semaphore
		if (timer_running) {
            if (current->remaining_time > waiting_list.tail->remaining_time) {
                curr_preempt_sm = &(current->preempt_sm);
                raise (SIGALRM);
                sem_wait(&(continue_semaphore));
            } else {
                continue;
            }
		}
		
		// choose the head to start from
		current = scheduler(current);

		// printList();

		if (current != NULL) {
			curr_preempt_sm = &(current->preempt_sm);
			if (current->remaining_time < current->quantum) {
				// additional time for operations handling
				if (current->remaining_time > 0) start_timer(current->remaining_time);
				else start_timer(2);
			} else {
				// start the timer right before letting a node start processing
				start_timer(current->quantum);
			}
			// if the semaphore is 0, it means that the thread is waiting for its turn
			// so we need to signal it
			sem_post(&(current->semaphore));
		}
	}
}

// Function to execute the threads in the waiting list
ThreadNode* scheduler(ThreadNode* node) {
    if (node && node->done == 1) {
        stop_timer();
        deleteNode(&waiting_list, node);
    }

    // if list is empty, return NULL
    if (waiting_list.head == NULL) return NULL;
    // if only node in list, return node
    if (waiting_list.head->next == NULL) return waiting_list.head;

    // we return the node with STRF
    int min_time = INT_MAX;
    ThreadNode* min_node = NULL;
    
    ThreadNode* current = waiting_list.head;
    while (current != NULL) {
        if (current->remaining_time < min_time) {
            // skip the same node
            if (node->client == current->client) {
                current = current->next;
                continue;
            }

            min_time = current->remaining_time;
            min_node = current;
        }
        current = current->next;
    }

    return min_node;
       
}