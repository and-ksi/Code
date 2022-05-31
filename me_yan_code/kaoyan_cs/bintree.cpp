#include <iostream>
#define ElemType int
using namespace std;

typedef struct BiTNode{
    ElemType data;
    struct BiTNode *lchild, *rchild;
}BiTNode, *BiTree;

typedef struct StackNode{
    BiTNode data;
    struct StackNode *next;
}StackNode, *LinkStack;

int CreateBitTree(BiTree &T){
    BiTree p = T;
    ElemType lc, rc;
    char sig;
    cout << "Please input root node:" << endl;
    T = new BiTNode;
    cin >> T->data;
    while(1){
        cout << "Then input children node of " << p->data << " ." << endl;
        cout << "Empty node input '#'." << endl;
        sig = cin.peek();
        if(sig != '#'){
            cin >> lc;
            p->lchild = new BiTNode;
            p->lchild->data = lc;
        }else{
            p->lchild = NULL;
        }
        sig = cin.peek();
        if (sig != '#'){
            cin >> rc;
            p->rchild = new BiTNode;
            p->rchild->data = rc;
        }else{
            p->lchild = NULL;
        }
    }
}

int main(){
    
}