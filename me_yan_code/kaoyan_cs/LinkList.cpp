#include <iostream>
#include <cstdlib>
#include <ctime>
#define ElemType int

typedef struct LNode{
    ElemType data;
    struct LNode *next;
    struct LNode *tail;
}LNode, *LinkList;

int CreateList(LinkList &L){//传入头指针
    L = new LNode;
    L->next = NULL;
    L->tail = L;
    return 1;
}

int Insert(LinkList &L, ElemType a){
    LinkList p = L->next;
    L->next = new LNode;
    L->next->data = a;
    L->next->next = p;
    return 1;
}

int InsertTail(LinkList &L, ElemType a){
    L->tail->next = new LNode;
    L->tail->next->data = a;
    L->tail->next->next = NULL;
    L->tail = L->tail->next;
    return 1;
}

int InputList(LinkList &L, int count){
    using namespace std;
    ElemType a;
    if(!count){
        cout << "Enter the count of the number: " << endl;
        cin >> count;
    }
    for(int i = 0; i < count; i++){
        cin >> a;
        InsertTail(L, a);
    }
    return 1;
}

int RandList(LinkList &L, int n){//传入链表头指针
    //srand(time(0));
    srand(time(0));
    //LinkList p = L->next;
    for(int i = 0; i < n; i++){
        //Insert(p, rand());
        Insert(L, rand() % 10);
    }
    return 1;
}

//王道数据结构P38 T6 错误答案
/* int SortList(LinkList &L){
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
} */
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
            cout << "Print error!!! Over 50 Elements." << endl;
            return 0;
        }
    }
    std::cout << std::endl;
    return 1;
}
//T7 正确答案
int DelRange(LinkList &L, int a, int b){
    LinkList p = L, r;
    while(p->next){
        if(p->next->data > a && p->next->data < b){
            r = p->next;
            p->next = p->next->next;
            free(r);
        }else{
            p = p->next;
        }
    }
    return 1;
}

//T9
int PrintFree(LinkList &head){
    using namespace std;
    LinkList nlist = head, olist = head->next->next, r;
    nlist->next->next = NULL;//这句必不可少，否则nlist不是一个独立的仅含有1个首元节点的链表
    while(olist){
        nlist = head;
        while(nlist->next && olist->data > nlist->next->data){
            nlist = nlist->next;
        }
        r = nlist->next;
        nlist->next = olist;
        olist = olist->next;
        nlist->next->next = r;
    }
    r = head->next;
    while(r){
        cout << r->data << " ";
        r = r->next;
        free(head->next);
        head->next = r;
    }
    head->next = NULL;
    cout << endl;
    return 1;
}

//T10
LinkList SplitList(LinkList &A){
    LinkList B = new LNode;
    B->next = NULL;
    LinkList r = A->next, tail = B;
    while(r->next){
        tail->next = r->next;
        r->next = r->next->next;//
        r = r->next;//
        tail = tail->next;
        if(!r) break;//
    }
    tail->next = NULL;
    return B;
}

//T12
int DelReplication(LinkList &L){
    LinkList r, p = L->next;
    while(p && p->next){
        if(p->data == p->next->data){
            r = p->next;
            p->next = p->next->next;
            free(r);
        }else{
            p = p->next;
        }
    }
    return 1;
}

//T13
LinkList Merge(LinkList &A, LinkList &B){
    LinkList C = new LNode;
    C->next = NULL;
    LinkList alist = A->next, blist = B->next, r;
    while(alist  && blist){
        if(alist->data < blist->data){
            r = C->next;
            C->next = alist;
            alist = alist->next;
            C->next->next = r;
        }else{
            r = C->next;
            C->next = blist;
            blist = blist->next;
            C->next->next = r;
        }
    }
    if(blist && !alist){
        alist = blist;
    }
    while(alist){
        r = C->next;
        C->next = alist;
        alist = alist->next;
        C->next->next = r;
    }
    return C;
}

//T14
LinkList GeneratePublic(LinkList &A, LinkList &B){
    LinkList alist = A->next, blist = B->next, tail;
    LinkList C = new LNode;
    ElemType a = 0x7fffffff;
    C->next = NULL;
    tail = C;
    while(alist && blist){
        if(alist->data == blist->data && alist->data != a){
            tail->next = new LNode;//
            tail->next->data = alist->data;
            tail->next->next = NULL;
            tail = tail->next;
            a = alist->data;
            alist = alist->next;
            blist = blist->next;
        }else if(alist->data < blist->data){
            alist = alist->next;
        }else{
            blist = blist->next;
        }
    }
    return C;
}

int ReSort(LinkList L){
    LinkList mid = L->next, p = L->next, r;
    while(p != NULL && p->next != NULL){
        mid = mid->next;
        p = p->next->next;
    }
    p = mid->next->next;
    mid->next->next = NULL;
    while(p != NULL){
        r = p->next;
        p->next = mid->next;
        mid->next = p;
        p = r;
    }
    p = L->next;
    /* while(mid->next != NULL){
        r = p->next;
        p->next = mid->next;
        p = p->next->next;
        mid->next = mid->next->next;
        p->next = r;
    } */
    while(mid->next != NULL){
        r = p->next;
        p->next = mid->next;
        mid->next = mid->next->next;
        p->next->next = r;
        p = r;
    }
    return 1;
}

int Create1_n(LinkList &L, int n){
    for( ; n > 0; n--){
        Insert(L, n);
    }
    return 1;
}

int main(){
    //T9
    /*LinkList L;
    CreateList(L);
    RandList(L, 10);
    //InputList(L, 10);
    PrintList(L);
    //SortList_1(L);
    PrintFree(L);
    //DelRange(L, 2, 5);
    PrintList(L);*/

    //T10
    /* LinkList A;
    CreateList(A);
    RandList(A, 30);
    PrintList(A);
    LinkList B = SplitList(A);
    PrintList(A);
    PrintList(B); */

    //T12
    /* LinkList L;
    CreateList(L);
    RandList(L, 23);
    PrintList(L);
    SortList_1(L);
    PrintList(L);
    DelReplication(L);
    PrintList(L);*/

    //T13
    /* LinkList A, B, C;
    CreateList(A);
    CreateList(B);
    RandList(A, 13);
    RandList(B, 16);
    SortList_1(A);
    SortList_1(B);
    PrintList(A);
    PrintList(B);
    C = Merge(A, B);
    PrintList(C); */

    //T14
    /* LinkList A, B, C;
    CreateList(A);
    CreateList(B);
    RandList(A, 13);
    RandList(B, 16);
    SortList_1(A);
    SortList_1(B);
    PrintList(A);
    PrintList(B);
    C = GeneratePublic(A, B);
    PrintList(C); */

    //T25
    LinkList L;
    CreateList(L);
    Create1_n(L, 6);
    PrintList(L);
    ReSort(L);
    PrintList(L);

    return 0;
}