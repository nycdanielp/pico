#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stdbool.h>

// Maximum number of tasks our OS can handle
#define MAX_TASKS 10

// A task is just a function pointer
typedef void (*task_func_t)(void);

// Task states
typedef enum {
    TASK_READY,
    TASK_RUNNING
} task_state_t;

// Task Control Block (TCB)
// This structure holds all the information the OS needs to know about a task
typedef struct {
    task_func_t function;
    task_state_t state;
} tcb_t;

// OS API functions
void os_init(void);
bool os_add_task(task_func_t function);
void os_run(void);

#endif // KERNEL_H
