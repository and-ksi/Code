#include <iostream>
#include <cstdlib>
#include <ctime>
#define ElemType int

typedef struct LNode{
    ElemType data;
    struct LNode *next;
}LNode, *LinkList;

int CreateList(LinkList &L){//传入头指针
    L = new LNode;
    L->next = NULL;
    return 1;
}

int Insert(LinkList &L, ElemType a){
    LinkList p = L->next;
    L->next = new LNode;
    L->next->data = a;
    L->next->next = p;
    return 1;
}

int RandList(LinkList &L, int n){//传入链表头指针
    srand(time(0));
    //LinkList p = L->next;
    for(int i = 0; i < n; i++){
        //Insert(p, rand());
        Insert(L, rand() % 10);
    }
    return 1;
}

//王道数据结构P38 T6 错误答案
int SortList(LinkList &L){
    LinkList p = L->next, q = L->next, pre;
    p->next = NULL;
    while(q){
        p = L->next;
        pre = L;
        while(q->data > p->data && p){
            pre = p;
            p = p->next;
        }
        pre->next = q;
        q = q->next;
        pre->next->next = p;
    }
    return 1;
}
//正确答案
int SortList_1(LinkList &L){
    LinkList oldl, newl, pre;
    oldl = L->next->next;
    L->next->next = NULL;
    while(oldl){
        newl = L->next;
        pre = L;
        while(newl && oldl->data > newl->data){
            pre = newl;
            newl = newl->next;
        }
        pre->next = oldl;
        oldl = oldl->next;
        pre->next->next = newl;
    }
    return 1;
}

int PrintList(LinkList &L){
    using namespace std;
    LinkList p = L->next;
    int i = 0;
    while(p){
        std::cout << p->data << " ";
        p = p->next;
        if(i++ > 50){
            cout << endl;
            cout << "Print error!!!" << endl;
            return 0;
        }
    }
    std::cout << std::endl;
    return 1;
}

int main(){
    LinkList L;
    CreateList(L);
    RandList(L, 10);
    PrintList(L);
    SortList_1(L);
    PrintList(L);
    return 0;
}