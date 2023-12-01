#include <semaphore.h>

// ANSI escape codes for text color
#define RED_TEXT "\x1b[31m"
#define GREEN_TEXT "\x1b[32m"
#define YELLOW_TEXT "\x1b[33m"
#define BLUE_TEXT "\x1b[34m"
#define RESET_TEXT "\x1b[0m"

extern sem_t continue_semaphore;