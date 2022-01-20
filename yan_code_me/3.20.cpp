//数制的转换————10->8
#include <iostream>
#include <stdlib.h>
using namespace std;

#define OK 1
#define ERROR 0
#define OVERFLOW -2

#define ElemType int
#define SElemType int

typedef struct StackNode
{
    int data;
    struct StackNode *next;
}StackNode, *LinkStack;

int InitStack(LinkStack &S)
{
    S = NULL;
    return OK;
}

int Push(LinkStack &S, SElemType e)
{
    StackNode *p = new StackNode;
    p->data = e;
    p->next = S;
    S = p;
    return OK;
}

int Pop(LinkStack &S, SElemType &e)
{
    if(S == NULL) return ERROR;
    e = S->data;
    LinkStack p = S;
    S = S->next;
    delete p;
    return OK;
}

SElemType GetTop(LinkStack &S)
{
    if (S == NULL) return ERROR;
    return S->data;
}

bool StackEmpty(LinkStack &S)
{
    if(S == NULL) return true;
    return false;
}

int conversion(int N)
{
    int e;
    LinkStack S;
    InitStack(S);
    while(N)
    {
        Push(S, N % 8);
        N = N / 8;
    }
    while (!StackEmpty(S))
    {
        Pop(S, e);
        cout << e;
    }
    cout << endl;
    return OK;
}

int main()
{
    int N;
    cout << "Please enter a decimal number." << endl;
    cin >> N;
    cout << "The octal number is: " << endl;
    conversion(N);
    system("pause");
    return OK;
}