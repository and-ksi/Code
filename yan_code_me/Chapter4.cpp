//病毒感染检测
#include <iostream>
#include <fstream>
using namespace std;

#define MAXLEN 225
typedef struct
{//串的定长存储结构
    char ch[MAXLEN];
    int length;
}SString;

typedef struct
{ //串的堆式顺序结构
    char *ch;
    int length;
} HString;

#define CHUNKSIZE 80

typedef struct Chunk
{ //串的链式存储结构
    char ch[CHUNKSIZE];
    struct Chunk *next;
} Chunk;

typedef struct
{//串的链式存储结构
    Chunk *head, *tail;
    int length;
};

int Index_BF(SString S, SString T, int pos)
{//模式匹配BF算法
    int i = pos, j = 0;
    while(i < S.length && j < T.length)
    {
        if(S.ch[i] == T.ch[j])
        {
            ++i;
            ++j;
        }else
        {
            j = 0;
            i -= j - 1;
        }
    }
    if(j = T.length)
    {
        return i - j;
    }
    return 0;
}

int Virus_detection()
{//病毒检测算法
    int num, flag;
    char vir[MAXLEN];
    SString Virus;
    SString Person;
    ifstream inFile("Virus_input_data.txt");
    ofstream outFile("Virus_output_result.txt");
    if(!inFile || !outFile)
    {
        cout << "Open file error!" << endl;
        return -1;
    }
    inFile >> num;
    while(num--)
    {
        inFile >> Virus.ch + 1;
        inFile >> Person.ch + 1;
        vir = Virus.ch;
        flag = 0;
        int m = Virus.length;
        for(int i = m + 1, j = 1; j <= m; j++)
        {
            Virus.ch[i++] = Virus.ch[j];
        }
    }
}