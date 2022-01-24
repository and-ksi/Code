//表达式求值
#include <iostream>
#include <math.h>
using namespace std;

#define OK 1;
#define ERROR 0;

#define Elemtype int
#define SElemtype int

int compare[7][7] = {{2, 2, 1, 1, 1, 2, 2}, {2, 2, 1, 1, 1, 2, 2}, {2, 2, 2, 2, 1, 2, 2}, {2, 2, 2, 2, 1, 2, 2}, {1, 1, 1, 1, 1, 0, -1}, {2, 2, 2, 2, -1, 2, 2}, {1, 1, 1, 1, 1, -1, 0}};

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

char Precede(char a, char b)
{//比较两操作符的优先级，已预先设好比较矩阵
    char num[2] = {a, b};
    int num_ch[2];
    for(int i = 0; i < 2; ++i)
    {
        switch (num[i])
        {
        case '+':
            num_ch[i] = 0;
            break;
        case '-':
            num_ch[i] = 1;
            break;
        case '*':
            num_ch[i] = 2;
            break;
        case '/':
            num_ch[i] = 3;
            break;
        case '(':
            num_ch[i] = 4;
            break;
        case ')':
            num_ch[i] = 5;
            break;
        case '#':
            num_ch[i] = 6;
            break;
        case '\0':
            num_ch[i] = 6;
            break;
        }
    }
    switch (compare[num_ch[0]][num_ch[1]])
    {
    case 2:
        return '>';
        break;
    case 1:
        return '<';
        break;
    case 0:
        return '=';
        break;
    default:
        break;
    }
    return ERROR;
}

int Operate(int a, int theta, int b)
{
    switch (theta)
    {
    case '+':
        return a + b;
        break;
    case '-':
        return a - b;
        break;
    case '*':
        return a * b;
        break;
    case '/':
        return a / b;
        break;
    }
    return ERROR;
}

int EvaluateExpression(char *ch)
{
    int i = 0, operand = 0;
    int k = 0;//k为数位
    int theta, a, b;
    LinkStack OPND, OPTR;//operand操作数， operator运算符， delimiter界限符
    InitStack(OPND);
    InitStack(OPTR);
    Push(OPTR, '#');
    while(*(ch + i) != '\0')
    {
        if (*(ch + i) >= '0' && *(ch + i) <= '9')
        {
            while (*(ch + i) >= '0' && *(ch + i) <= '9')
            {
                operand += (ch[i] - '0') * pow(10,k);
                ++k;
                ++i;
            }
            Push(OPND, operand);
            operand = 0;
            k = 0; //数字、数位归零
        }
        else
        {
            switch (Precede(GetTop(OPTR), ch[i]))
            {
            case '<':
                Push(OPTR, ch[i]);
                break;
            case '>':
                Pop(OPTR, theta);
                Pop(OPND, b);
                Pop(OPND, a);
                Push(OPND, Operate(a, theta, b));
            case '=':
                Pop(OPTR, a);
                break;
            }
            ++i;
        }
    }
    return GetTop(OPND);
}

int main()
{
    char ch[100];
    cout << "Please input expression, '#' shuold be the end of expression:" << endl;
    cin >> ch;
    cout << EvaluateExpression(ch) << endl;
    return 0;
}