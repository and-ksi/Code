#include <iostream>
#define ElemType char
using namespace std;

typedef struct BiTNode{//二叉树节点
    ElemType data;
    struct BiTNode *lchild, *rchild;
}BiTNode, *BiTree;

typedef struct PosStack{//位置栈，用来存储将来要搜索的位置
    int sta_pos, end_pos;
    BiTree tree;
    struct PosStack *next;
}PosStack, *LinkPosStack;

int Push(LinkPosStack &S, int pos1, int pos2, BiTree tr){
    LinkPosStack p = new PosStack;
    p->sta_pos = pos1;
    p->end_pos = pos2;
    p->next = S;
    p->tree = tr;
    S = p;
    return 1;
}

int Pop(LinkPosStack &S, int &pos1, int &pos2, BiTree &tr){
    if(!S){
        cout << "PosStack Pop error!" << endl;
        exit(1);
    }
    pos1 = S->sta_pos;
    pos2 = S->end_pos;
    tr = S->tree;
    LinkPosStack p = S;
    S = S->next;
    delete p;
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

int InsertBin(BiTree &T, ElemType c){
    T = new BiTNode;
    T->data = c;
    T->lchild = T->rchild = NULL;
    return 1;
}

//由输入的前序序列和中序序列生成对应的二叉树
int CreateBinTree(BiTree &T, string front, string mid){
    int len = front.length();
    if(len != mid.length() || len <= 0){
        cout << "String length error!" << endl;
        return -1;
    }
    int pos, pos_root, pos_child;
    int start = 0, end = len - 1;
    LinkPosStack S = NULL;
    //Push(S, 0, len - 1);
    pos_root = Search(mid, front[0], 0, len - 1);
    InsertBin(T, front[0]);
    BiTree p = T;
    for(pos = 1; pos < len; pos++){
        if(start >= end){
            if(!S)
                break;
            Pop(S, start, end, p);
            pos_root = start - 1;
        }
        pos_child = Search(mid, front[pos], start, end);
        if(pos_root > pos_child){
            InsertBin(p->lchild, front[pos]);
            if(pos_root != end || start > end)
                Push(S, pos_root + 1, end, p);
            p = p->lchild;
            end = pos_root - 1;
        }else{
            InsertBin(p->rchild, front[pos]);
            // if (start != pos_root && start > end)
            //     Push(S, start, pos_root - 1, p);
            p = p->rchild;
            start = pos_root + 1;
        }
        pos_root = pos_child;
    }
    return 1;
}

//将二叉树按照前序序列输出
int BinFrontPrint(BiTree T, string &front){
    LinkPosStack S = NULL;
    int a, b;
    front = "";
    while(1){
        while(T){
            front += T->data;
            if(T->rchild){
                Push(S, 0, 0, T->rchild);
            }
            T = T->lchild;
        }
        if(!S)
            break;
        Pop(S, a, b, T);
    }
    return 1;
}

//二叉树中序输出
int BinMidPrint(BiTree T, string &mid){
    LinkPosStack S = NULL;
    int a, b;
    mid = "";
    while(1){
        if(T){
            Push(S, a, b, T);
            T = T->lchild;
        }else{
            if(!S)
                break;
            Pop(S, a, b, T);
            mid += T->data;
            T = T->rchild;
        }
    }
    return 1;
}

//二叉树后序输出
int BinRightPrint(BiTree T, string &right){
    LinkPosStack S = NULL;
    int a, b;
    right = "";
    while(1){
        
    }
}

int main(){
    string fr = "ABDCE", en = "BDAEC";
    // string fr = "ABDGECF", en = "DGBEACF";
    BiTree T;
    CreateBinTree(T, fr, en);
    string output;
    BinFrontPrint(T, output);
    cout << output << endl;
    BinMidPrint(T, output);
    cout << output << endl;
    return 0;
}