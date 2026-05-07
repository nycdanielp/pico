#include "kernel.h"
#include "pico/stdlib.h"

static tcb_t tasks[MAX_TASKS];
static uint8_t num_tasks = 0;
static uint8_t current_task = 0;

void os_init(void) {
    num_tasks = 0;
    current_task = 0;
    for (int i = 0; i < MAX_TASKS; i++) {
        tasks[i].state = TASK_READY;
        tasks[i].function = NULL;
    }
}

bool os_add_task(task_func_t function) {
    if (num_tasks >= MAX_TASKS) {
        return false; // Task limit reached
    }
    tasks[num_tasks].function = function;
    tasks[num_tasks].state = TASK_READY;
    num_tasks++;
    return true;
}

void os_run(void) {
    while (1) {
        if (num_tasks == 0) continue;

        tcb_t *task = &tasks[current_task];

        // Run the task if it's ready
        if (task->state == TASK_READY) {
            task->state = TASK_RUNNING;
            
            // Execute the task
            // IMPORTANT: In this simple cooperative scheduler, 
            // tasks must NOT block (e.g., no sleep_ms).
            // They should do their work and return quickly.
            task->function(); 
            
            task->state = TASK_READY;
        }

        // Move to the next task in a round-robin fashion
        current_task = (current_task + 1) % num_tasks;
    }
}
