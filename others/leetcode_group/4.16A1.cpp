// https: // vjudge.net/contest/489211#problem

#include <iostream>

typedef struct value
{
    int num;
    struct value *next;
} * va;

int Sort(int a, va &b)
{
    va b1 = b, b2;
    va g = new value;
    g->num = a;
    g->next = NULL;
    int signal = 1;
    while (a < b1->num)
    {
        signal = 0;
        b2 = b1;
        b1 = b1->next;
    }
    if (signal)
    {
        g->next = b;
        b = g;
    }
    else
    {
        g->next = b2->next;
        b2->next = g;
    }
    if (g->num == g->next->num)
    {
        return 0;
    }
    return 1;
}

int GetValue(int n)
{
    if (n & 1)
    {
        return 3 * n + 1;
    }
    return n / 2;
}

int main()
{
    using namespace std;
    int s, count = 1;
    va a;
    a = new value;
    a->next = NULL;
    a->num = -1;
    cin >> s;
    while (Sort(s, a))
    {
        s = GetValue(s);
        //        cout << "s = " << s << endl;
        count++;
    }
    cout << count << endl;
    return 0;
}