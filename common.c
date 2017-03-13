#include "os.h"
#include "common.h"

void proc_list_init(task_queue_t * list) {
    list->head = list->tail = NULL;
    list->len = 0;
}

void proc_list_append(task_queue_t * list, PD * proc) {
    if (list->len == 0) {
        list->head = list->tail = proc;
    } else {
        list->tail->next = proc;
    }
    list->tail = proc;
    proc->next = NULL;
    list->len++;
}

PD * proc_list_pop(task_queue_t * list) {
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
