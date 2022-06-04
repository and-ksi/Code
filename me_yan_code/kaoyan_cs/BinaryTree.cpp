#include <iostream>
#define ElemType char
using namespace std;

typedef struct BiTNode{//二叉树节点
    ElemType data;
    struct BiTNode *lchild, *rchild;
}BiTNode, *BiTree;

typedef struct StackNode{//栈节点
    BiTNode *data;
    struct StackNode *next;
}StackNode, *LinkStack;

typedef struct PosStack{//位置栈，用来存储将来要搜索的位置
    int sta_pos, end_pos;
    struct PosStack *next;
}PosStack, *LinkPosStack;

typedef struct QueueNode{//队列节点
    BiTree data;
    struct QueueNode *next;
}QueueNode, *QueuePoint;

typedef struct LinkQueue{//队列
    QueuePoint front, rear;
    int length;
}LinkQueue;

int Push(LinkStack &S, BiTNode *q){
    LinkStack p = new StackNode;
    p->data = q;
    p->next = S;
    S = p;
    return 1;
}

int StackEmpty(LinkStack S){
    if(S)
        return 0;
    return 1;
}

int Pop(LinkStack &S, BiTree &p){
    if(!S){
        cout << "Stack is empty! Pop error!" << endl;
        exit(1);
    }
    p = S->data;
    S = S->next;
    return 1;
}

int QueueInitia(LinkQueue &q){
    q.front = new QueueNode;
    q.rear = q.front;
    q.rear->next = NULL;
    q.length = 0;
    return 1;
}

int QueueEmpty(LinkQueue p){
    if(p.front == p.rear)
        return 1;
    return 0;
}

int EnQueue(LinkQueue &q, BiTree e){
    q.rear->next = new QueueNode;
    q.rear = q.rear->next;
    q.rear->data = e;
    q.rear->next = NULL;
    q.length++;
    return 1;
}

int DeQueue(LinkQueue &q, BiTree &e){
    if(QueueEmpty(q)){
        cout << "The queue is empty! Dequeue error!" << endl;
        exit(1);
    }
    QueuePoint k;
    e = q.front->next->data;
    k = q.front->next;
    if(q.rear == q.front->next)
        q.rear = q.front;
    q.front->next = q.front->next->next;
    q.length--;
    
    delete k;
    return 1;
}

int DestroyQueue(LinkQueue &q){
    QueuePoint e;
    while(q.front != NULL){
        e = q.front;
        q.front = q.front->next;
        delete e;
    }
    return 1;
}

int Search(string &s, char c, int start, int end){
    if(end >= s.length()){
        cout << "Search range error!" << endl;
        return -1;
    }
    int pos, len = s.length();
    for(pos = start; pos <= end; pos++){
        if(s[pos] == c){
            return pos;
        }
    }
    return -1;
}

int CreateBinTree(BiTree &T, string front, string mid){
    int len = front.length();
    if(len != mid.length()){
        cout << "String length error!" << endl;
        return -1;
    }
    int pos, pos_mid;
    int start = 0, end = len - 1;
    for(pos = 0; pos < len; pos++){
        pos_mid = Search(mid, front[pos], start, end);
        
    }
}

int CreateBitTree(BiTree &T){
    ElemType lc = 0, rc = 0;
    char sig;
    cout << "Please input root node:" << endl;
    T = new BiTNode;
    cin >> T->data;
    //LinkStack a = NULL, b = NULL;
    //Push(a, T);
    LinkQueue a, b;
    QueuePoint e;
    QueueInitia(a);
    QueueInitia(b);
    BiTree p;
    EnQueue(a, T);
    while(!QueueEmpty(a)){
        while(!QueueEmpty(a)){
            DeQueue(a, p);
            cout << "Then input children node of " << p->data << "." << endl;
            cout << "Empty node input 'e', children input 'l<value>' or 'r<value>', no space." << endl;
            cin >> sig;
            if(sig == 'e'){
                p->lchild = NULL;
            }else{
                cin >> lc;
                p->lchild = new BiTNode;
                p->lchild->data = lc;
                EnQueue(b, p->lchild);
            }
            sig = cin.get();
            if(sig == 'e'){
                p->rchild = NULL;
            }else{
                cin >> rc;
                p->rchild = new BiTNode;
                p->rchild->data = rc;
                EnQueue(b, p->rchild);
            }
        }
        DestroyQueue(a);
        a.front = b.front;
        a.rear = b.rear;
        a.length = b.length;
        QueueInitia(b);
    }
    return 1;
}



int main(){
    BiTNode *T = NULL;
    CreateBitTree(T);
    return 1;
}