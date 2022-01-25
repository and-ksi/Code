//括号匹配的校验
#include <iostream>
#include <stdlib.h>
using namespace std;

#define OK 1
#define ERROR 0
#define OVERFLOW -2

#define ElemType int
#define SElemType char

typedef struct StackNode
{
    ElemType data;
    struct StackNode *next;
}StackNode, *LinkStack;

int InitStack(LinkStack &S)
{
    S = NULL;
    return OK;
}

int Push(LinkStack &S, SElemType e)
{
    //if(S == NULL) return ERROR;
    LinkStack p = new StackNode;
    p->data = e;
    p->next = S;
    S = p;
    return OK;
}

int Pop(LinkStack &S, SElemType &e)
{
    if (S == NULL)
        return ERROR;
    e = S->data;
    LinkStack p = S;
    S = S->next;
    delete p;
    return OK;
}

bool StackEmpty(LinkStack &S)
{
    if(S == NULL)
        return OK;
    else
        return ERROR;
}

SElemType GetTop(LinkStack &S)
{
    if (S == NULL)
        return ERROR;
    return S->data;
}

int Matching()
{
    int flag = 1;
    char x;
    char ch[20];
    int i = 0;
    LinkStack S;
    InitStack(S);
    cout << "Please enter expression, limit 20 chars!\n'#' to end your input." << endl;
    cin >> ch;
    while(ch[i] != '#' && flag != 0)
    {
        switch (ch[i])
        {
        case '[':
            Push(S, ch[i]);
            ++i;
            break;

        case '(':
            Push(S, ch[i]);
            ++i;
            break;

        case ']':
            if (!StackEmpty(S) && GetTop(S) == '[')
                {
                    Pop(S, x);
                    ++i;
                }
            else
                flag = 0;
            break;

        case ')':
            if (!StackEmpty(S) && GetTop(S) == '(')
            {
                Pop(S, x);
                ++i;
            }
            else
                flag = 0;
            break;
        }
    }
    if(StackEmpty(S) && flag == 1)
    {
        cout << "It's true!" << endl;
        return true;
    }
    else
    {
        cout << "It's false!" << endl;
        return false;
    }
}

int main()
{
    Matching();
    return 0;
}