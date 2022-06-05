#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string>

#define ElemType string
using namespace std;

typedef struct BiTNode{//二叉树节点
    ElemType data;
    struct BiTNode *lchild, *rchild;
}BiTNode, *BiTree;

typedef struct BinStackNode{
    BiTree tree;
    struct BinStackNode *next;
}BinStackNode, *LinkBinStack;

typedef struct BinQueueNode{
    BiTree data;
    struct BinQueueNode *next;
}BinQueueNode, *BinQueuePtr;

typedef struct LinkBinQueue{
    BinQueuePtr front, rear;
    int length;
}LinkBinQueue;

int InitBinQueue(LinkBinQueue &Q){
    Q.rear = Q.front = new BinQueueNode;
    Q.length = 0;
    Q.front->next = NULL;
    return 1;
}

int DeBinQueue(LinkBinQueue &Q, BiTree &t){
    if(Q.front == Q.rear)
        return -1;
    t = Q.front->next->data;
    BinQueuePtr p;
    p = Q.front->next;
    Q.front->next = p->next;
    delete p;
    Q.length++;
    return 1;
}

int EnBinQueue(LinkBinQueue &Q, BiTree t){
    BinQueueNode n;
    n.data = t;
    n.next = NULL;
    Q.rear->next = &n;
    Q.rear = &n;
    Q.length++;
    return 1;
}

int Push(LinkBinStack &S, BiTree tr){
    LinkBinStack p = new BinStackNode;
    p->next = S;
    p->tree = tr;
    S = p;
    return 1;
}

int Pop(LinkBinStack &S, BiTree &tr){
    if(!S){
        cout << "PosStack Pop error!" << endl;
        exit(1);
    }
    tr = S->tree;
    LinkBinStack p = S;
    S = S->next;
    delete p;
    return 1;
}

int InsertBinNode(BiTree &T, ElemType c){
    T = new BiTNode;
    T->data = c;
    T->lchild = T->rchild = NULL;
    return 1;
}

int CreateRandomBinTree_string(BiTree &T){
    if(rand() % 30 >= 26){
        T = NULL;
        return 1;
    }
    BiTree p;
    string str = "ROOT";
    InsertBinNode(T, str);
    p = T;
    LinkBinQueue Q;
    BiTree rearold = p, rearnew = NULL;
    InitBinQueue(Q);
    EnBinQueue(Q, p);
    int levnum[26] = {1}, level = 0;
    char letter = 'A';
    while(Q.length && letter <= 'C'){//debug
        DeBinQueue(Q, p);
        if(rand() % 30 < 26){
            InsertBinNode(p->lchild, letter + to_string(levnum[level]));
            EnBinQueue(Q, p->lchild);
            rearnew = p->lchild;
            levnum[level]++;
        }
        if(rand() % 30 < 26){
            InsertBinNode(p->rchild, letter + to_string(levnum[level]));
            EnBinQueue(Q, p->rchild);
            rearnew = p->rchild;
            levnum[level]++;
        }
        if(rearold == p){
            rearold = rearnew;
            letter++;
            level++;
        }
    }
    return 1;
}

int main(){
    srand(time(NULL));
    BiTree T;
    CreateRandomBinTree_string(T);
    return 0;
}