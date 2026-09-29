#ifndef __QUEUE_H__
#define __QUEUE_H__
#include "request.h"
#include <stdlib.h>
typedef struct Node
{
    struct request *request;
    struct Node *next;
    struct Node *prev;
} Node;

typedef struct queue
{
    int size;
    Node *head;
    Node *tail;
} queue;

void init(queue *q);


void destroy(queue *q);

void push_back(queue *q, request *request);

void push_front(queue *q, request *request);

void pop_back(queue *q);

void pop_front(queue *q);

request *get_back(queue *q);

request *get_front(queue *q);

request *remove_index(queue *q, int index);

request *find_by_reqest(queue *q, request *req);

void remove_by_request(queue *q, request *req);
#endif
