//舞伴问题  未完成
#include <iostream>
using namespace std;

#define OK 1;
#define ERROR 0;

#define Elemtype int
#define SElemtype int

typedef struct StackNode
{
    Elemtype data;
    struct StackNode *next;
} StackNode, *LinkStack;

int InitStack(LinkStack &S)
{
    S = NULL;
    return OK;
}

bool StackEmpty(LinkStack S)
{
    if (S == NULL)
        return true;
    return false;
}

int GetTop(LinkStack S)
{
    if (S == NULL)
        return ERROR;
    return S->data;
}

int Push(LinkStack &S, SElemtype e)
{
    LinkStack p;
    p = new StackNode;
    p->data = e;
    p->next = S;
    S = p;
    return OK;
}

int Pop(LinkStack &S, SElemtype &e)
{
    if (S == NULL)
        return ERROR;
    e = S->data;
    S = S->next;
    return OK;
}

