#include "os.h"
#include <stdint.h>
#include "common.h"

void queue_init(task_queue_t * list) {
    list->head = list->tail = NULL;
    list->len = 0;
}

void enqueue(task_queue_t * list, PD * task) {
    if (list->len == 0) {
        list->head = list->tail = task;
    } else {
        list->tail->next = task;
    }
    list->tail = task;
    task->next = NULL;
    list->len++;
}

PD * deque(task_queue_t * list) {
    if (list->len == 0) {
        return NULL;
    } else if (list->len == 1) {
        PD * p = list->head;
        list->head = list->tail = NULL;
        list->len = 0;
        return p;
    } else {
        PD * p = list->head;
        list->head = p->next;
        list->len--;
        return p;
    }
}
