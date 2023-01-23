#include "algorithm.h"
#include <stdio.h>
#include <string.h>

static node_t *new_slot_link(slot_t data)
{
    node_t *head = (node_t *)malloc(sizeof(node_t));
    head->data = data;
    head->next = NULL;
    return head;
}
// 信号初始化
signal_t *Qsignal()
{
    signal_t *s = (signal_t *)malloc(sizeof(signal_t));
    s->slot = (node_t *)malloc(sizeof(node_t));
    s->slot->next = NULL;
    return s;
}

// 添加槽函数
void Qconnect(signal_t *_signal, slot_t _slot)
{
    node_t *p;
    p = _signal->slot;
    while (p->next != NULL)
    {
        p = p->next;
    }

    p->next = new_slot_link(_slot);
}

// 发射信号
void emit(signal_t *_signal)
{
    node_t *p;
    p = _signal->slot;
    while (p->next != NULL)
    {
        slot_t func = p->next->data;
        if (func != NULL)
            func();

        p = p->next;
    }
}

// 销毁
void del(signal_t *_signal)
{
    node_t *p, *q;
    p = _signal->slot;
    q = p;
    while (p->next != NULL)
    {
        q = p;
        p = p->next;
        free(q);
    }
    free(p);
    free(_signal);
}
