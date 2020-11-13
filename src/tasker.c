/*
 * file:     tasker.c
 * author:   Ferenc Nemeth, Refactored by koonix (https://github.com/soystemd)
 * date:     21 Jul 2018
 * brief:
 *
 * This is a really simple, non-preemptive task scheduler.
 * You can register tasks with their runnable function and the periodic
 * time you want to call them. With a help of a timer the tasks get into READY
 * state after every time period (except if they are PAUSED) and they get
 * called and executed in the main()'s inifinte loop. After they are finished
 * everything starts over. This Scheduler helps you to keep your tasks and
 * timing organized.
 *
 * Copyright (c) 2018 Ferenc Nemeth - https://github.com/ferenc-nemeth/
 *
 */

#include <stddef.h>
#include "config.h"
#include "tasker.h"

// Static function prototypes:
static void check_task_counter_and_handle_state(uint8_t task_index);
static uint8_t get_task_index(TaskFunctionPtr task);

static Task task_array[NUMBER_OF_TASKS] = {NULL};
static uint8_t numberof_tasks = 0;

void tsk_task_create(TaskFunctionPtr function, TaskState state, TaskTime period)
{
    task_array[numberof_tasks].run     = function;
    task_array[numberof_tasks].period  = period;
    task_array[numberof_tasks].state   = state;
    task_array[numberof_tasks].counter = 1;
    numberof_tasks++;
}

/*
 * Finds a task's function and find it's
 * index in the tasks_array.
 */
static uint8_t get_task_index(TaskFunctionPtr task)
{
    uint8_t task_index = 0;
    for (; task_index < numberof_tasks; task_index++)
        if (task_array[task_index].run == task)
            break;
    return task_index;
}

/* This function keeps track of the tasks' time and puts them into
READY state. This function shall be called in a timer interrupt. */
void tsk_task_time_manager(void)
{
    for (uint8_t task_index = 0; task_index < numberof_tasks; task_index++)
        if (task_array[task_index].state != PAUSED)
            check_task_counter_and_handle_state(task_index);
}
/* This is a backend method used in the function "tsk_task_time_manager". */
static void check_task_counter_and_handle_state(uint8_t task_index)
{
    /* Put it into READY state. */
    if (task_array[task_index].counter >= task_array[task_index].period) {
        task_array[task_index].counter = 1;
        task_array[task_index].state   = READY;
    }

    /* Or increment task's counter. */
    else {
        task_array[task_index].counter++;
    }
}

/* This function calls the READY tasks and then puts them back into
RUNNABLE state. This function SHALL be called in the infinite loop. */
void tsk_task_runner(void)
{
    for (uint8_t task_index = 0; task_index < numberof_tasks; task_index++) {
        /* If it is ready, call it.*/
        if (task_array[task_index].state == READY) {
            task_array[task_index].state = RUNNABLE;
            task_array[task_index].run();
        }
    }
}

/* Returns the state of the task. */
TaskState tsk_get_task_state(TaskFunctionPtr task)
{
    return task_array[get_task_index(task)].state;
}

/* Returns the burst time of the task. */
TaskTime tsk_get_task_period(TaskFunctionPtr task)
{
    return task_array[get_task_index(task)].period;
}

/* Returns the current counter value of the task. */
TaskTime tsk_get_task_counter(TaskFunctionPtr task)
{
    return task_array[get_task_index(task)].counter;
}

/* Manually changes the task's state. */
void tsk_set_task_state(TaskFunctionPtr task, TaskState new_state)
{
    uint8_t task_index           = get_task_index(task);
    task_array[task_index].state = new_state;
    if (new_state == PAUSED)
        task_array[task_index].counter = 1;
}

/* Manually changes the task's burst time. */
void tsk_set_task_period(TaskFunctionPtr task, TaskTime new_period)
{
    task_array[get_task_index(task)].period = new_period;
}

/* Manually changes the task's counter. */
void tsk_set_task_counter(TaskFunctionPtr task, TaskTime new_counter)
{
    task_array[get_task_index(task)].counter = new_counter;
}
