#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>

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
    } else {
        ThreadNode* current = list->head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
        new_node->prev = current;
    }
    return new_node;
}

// Function to delete a node from the waiting list
void deleteNode(NodeList* list, ThreadNode* node) {
    if ((list == NULL) | (list->head == NULL) | (node == NULL)) {
        return; // Do nothing if the node is NULL
    }

    // If the node to be deleted is the head node
    if (list->head == node) {
        list->head = node->next;
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
    ThreadNode* current = waiting_list->head;
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
	ThreadNode* current = waiting_list->head;

	while (1) {
		// wait for this semaphore to be available, to continue with the control flow
		// this becomes available when a program finishes execution
		sem_wait(&(continue_semaphore));
		if (current && current->done == 1) {
			stop_timer();
			deleteNode(waiting_list, current);
		}	
		// if timer running, stop it and post preempt semaphore
		if (timer_running) {
			curr_preempt_sm = &(current->preempt_sm);
			raise (SIGALRM);
			sem_wait(&(continue_semaphore));
		}
		
		// choose the head to start from
		current = scheduler(current);

		printList();

		if (current != NULL) {
			curr_preempt_sm = &(current->preempt_sm);
			if (current->remaining_time < current->quantum) {
				// additional time for operations handling
				if (current->remaining_time > 0) start_timer(current->remaining_time + 1);
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
    return waiting_list->head;
    // while (1) {
    //     ThreadNode* current = waiting_list;

    //     while (current != NULL) {
    //         // Wait for the semaphore to be signaled (indicating turn on the scheduler)
    //         sem_wait(&(current->semaphore));

    //         // Execute the thread for 1 second
    //         // (Note: For simplicity, we use sleep here, but in a real-world scenario,
    //         // you may want to use a more precise timer mechanism.)
    //         sleep(1);
    //         current->remaining_time--;

    //         // If the thread has finished execution, remove it from the waiting list
    //         if (current->remaining_time <= 0) {
    //             printf("Thread %lu finished execution\n", current->thread);
    //             ThreadNode* temp = current;
    //             current = current->next;

    //             if (temp->prev != NULL) {
    //                 temp->prev->next = temp->next;
    //             } else {
    //                 waiting_list = temp->next;
    //             }

    //             if (temp->next != NULL) {
    //                 temp->next->prev = temp->prev;
    //             }

    //             free(temp);
    //         } else {
    //             // Move to the next node in the linked list
    //             current = current->next;
    //         }
    //     }
    // }
    // return NULL;
}






// Function for Round Robin Scheduler
// ThreadNode* scheduler(ThreadNode* arg) {
//     int current_round_quantum = FIRST_ROUND_QUANTUM; // Quantum for the current round

//     while (1) {
//         ThreadNode* current = waiting_list;

//         // Loop through the waiting list
//         while (current != NULL) {
//             if (current && current->done == 1) {
//                 // If a thread is marked as done, remove it from the list
//                 ThreadNode* to_remove = current;
//                 current = current->next;
//                 deleteNode(to_remove);
//                 continue;
//             }

//             // Check if the current thread is ready to run
//             if (current->remaining_time > 0) {
//                 // If the thread's remaining time is greater than the current round's quantum, run for quantum time
//                 int run_time = (current->remaining_time > current_round_quantum) ? current_round_quantum : current->remaining_time;

//                 // Update the remaining time for the thread
//                 current->remaining_time -= run_time;

//                 // Indicate the turn to process for this thread
//                 sem_post(&(current->semaphore));

//                 // Wait for the thread to finish its quantum or its completion
//                 sem_wait(&(current->preempt_sm));

//                 // If the thread is completed, remove it from the list
//                 if (current->done) {
//                     ThreadNode* to_remove = current;
//                     current = current->next;
//                     deleteNode(to_remove);
//                     continue;
//                 }
//             }

//             // Move to the next thread
//             current = current->next;
//         }

//         // Update the quantum for the next round
//         current_round_quantum = REMAINING_ROUND_QUANTUM;
//     }

//     return NULL;
// }