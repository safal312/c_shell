#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    // Check if the correct number of command-line arguments is provided
    if (argc != 2) {
        fprintf(stderr, "Usage: %s N\n", argv[0]);
        return 1; // Return an error code
    }

    // Convert the command-line argument to an integer
    int seconds = atoi(argv[1]);

    // Check if the conversion was successful
    if (seconds <= 0) {
        fprintf(stderr, "Please provide a positive integer value for N.\n");
        return 1; // Return an error code
    }

    // Run the process for N seconds and print an output every second
    for (int i = 0; i <= seconds; ++i) {
        printf("Demo %d/%d\n", i, seconds);
        sleep(1); // Sleep for 1 second
    }

}