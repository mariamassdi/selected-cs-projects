#include "queue.h"
void init(queue *q)
{
    q->size = 0;
     q->head = NULL;
     q->tail = NULL;
    return;
}
void destroy(queue *q)
{
    if (q == NULL)
    {
        return;
    }
    while (q->head != NULL)
    {
        free(q->head->request);
        pop_front(q);
    }
    free(q);
}

void push_back(queue *q, request *request)
{
    if (q == NULL)
    {
        return;
    }
    Node *node = malloc(sizeof(Node));
    node->request = request;
    node->next = NULL;
    node->prev = q->size != 0 ? q->tail : NULL;

    if (q->size == 0)
    {
        q->head = q->tail = node;
    }
    else
    {
        q->tail->next = node;
        q->tail = q->tail->next;
    }
    q->size++;
}

void push_front(queue *q, request *request)
{
    if (q == NULL)
    {
        return;
    }
    Node *node = malloc(sizeof(Node));

    node->request = request;
    node->next = q->size != 0 ? q->head : NULL;
    node->prev = NULL;

    if (q->size == 0)
    {
        q->head = q->tail = node;
    }
    else
    {
        q->head->prev = node;
        q->head = q->head->prev;
    }
    q->size++;
}

void pop_back(queue *q)
{
    if (q == NULL || q->size == 0)
    {
        return;
    }
    Node *temp = q->tail;
    q->tail = temp->prev;
    q->size--;

    if (q->size == 0)
    {
        q->head = NULL;
    }
    else
    {
        q->tail->next = NULL; 
    }
    free(temp);
}

void pop_front(queue *q)
{
    if (q == NULL || q->size == 0)
    {
        return;
    }
    Node *temp = q->head;
    q->head = temp->next;
    q->size--;

    if (q->size == 0)
    {
        q->tail = NULL;
    }
    else
    {
        q->head->prev = NULL; 
    }
    free(temp);
}

request *get_back(queue *q)
{
    if (q == NULL || q->size == 0)
    { 
        return NULL;
    }
    return q->tail->request;
}

request *get_front(queue *q)
{
    if (q == NULL || q->size == 0)
    { 
        return NULL;
    }
    return q->head->request;
}

request *remove_index(queue *q, int index)
{
    if (q == NULL || index < 0 || index >= q->size)
    {
        return NULL;
    }

    if (index == 0)
    {
        request *front_request = get_front(q);
        if (front_request == NULL)
        { 
            return NULL;
        }
        request *request_copy = malloc(sizeof(request));
        if (request_copy == NULL)
        {
            return NULL; 
        }
        *request_copy = *front_request;
        pop_front(q);
        return request_copy;
    }

    if (index == q->size - 1)
    {
        request *back_request = get_back(q);
        if (back_request == NULL)
        { 
            return NULL;
        }
        request *request_copy = malloc(sizeof(request));
        if (request_copy == NULL)
        {
            return NULL;
        }
        *request_copy = *back_request;
        pop_back(q);
        return request_copy;
    }

    
    request *request_copy = malloc(sizeof(request));
    if (request_copy == NULL)
    {
        return NULL; 
    }
    Node *temp = q->head;
    for (int i = 0; i < index; i++)
    {
        temp = temp->next;
    }
    if (temp == NULL || temp->request == NULL)
    {
        free(request_copy);
        return NULL;
    }
    *request_copy = *(temp->request);

    temp->prev->next = temp->next;
    if (temp->next != NULL)
    {
        temp->next->prev = temp->prev;
    }
    q->size--;
    free(temp);
    return request_copy;
}

request *find_by_reqest(queue *q, request *req)
{
    if (q == NULL || req == NULL)
    {
        return NULL;
    }
    if (q->head == NULL)
    {
        return NULL;
    }
    Node *temp = q->head;
    while (temp != NULL)
    {
        if (temp->request == req)
        {
            return temp->request;
        }
        temp = temp->next;
    }
    return NULL;
}

void remove_by_request(queue *q, request *req)
{
    if (q == NULL || req == NULL)
    {
        return;
    }
    if (q->head == NULL)
    {
        return;
    }
    Node *temp = q->head;
    int index = 0;
    while (temp != NULL)
    {
        if (temp->request == req)
        {
            break;
        }
        index++;
        temp = temp->next;
    }
    remove_index(q, index);
}
