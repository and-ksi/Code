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
}StackNode, *LinkStack;

int InitStack(LinkStack &S)
{
    S = NULL;
    return OK;
}

bool StackEmpty(LinkStack S)
{
    if(S == NULL)
        return true;
    return false;
}

int GetTop(LinkStack S)
{
    if(S == NULL)
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
    if(S == NULL)
        return ERROR;
    e = S->data;
    S = S->next;
    return OK;
}

int StackLength(LinkStack S)
{ //返回栈长度，该算法可改进？
    int result = 0;
    while (S != NULL)
    {
        S = S->next;
        ++result;
    }
    return result;
}

int StackTraverse(LinkStack S)
{
    if(S == NULL)
        return ERROR;
    while(S != NULL)
    {
        cout << S->data << endl;
        S = S->next;
    }
    return OK;
}

int DestroyStack(LinkStack &S)
{
    LinkStack p;
    while (S != NULL)
    {
        p = S;
        S = S->next;
        delete p;
    }
    return OK;
}

int ClearStack(LinkStack &S)
{ //其与栈销毁的区别？
    while (S != NULL)
    {
    }
}