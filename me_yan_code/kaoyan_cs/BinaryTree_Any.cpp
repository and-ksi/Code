#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <string.h>
#include <iomanip>

#define ElemType string
using namespace std;

typedef struct BiTNode{//二叉树节点
    ElemType data;
    struct BiTNode *lchild, *rchild;
}BiTNode, *BiTree;

typedef struct BinStackNode{
    BiTree tree;
    int row;
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
    BinQueuePtr p;
    t = Q.front->next->data;
    p = Q.front->next;
    int a;
    a = 10;
    if(Q.front->next == Q.rear)
        Q.rear = Q.front;
    Q.front->next = p->next;
    delete p;
    Q.length--;
    return 1;
}

int EnBinQueue(LinkBinQueue &Q, BiTree t){
    BinQueuePtr n = new BinQueueNode;
    n->data = t;
    n->next = NULL;
    Q.rear->next = n;
    Q.rear = n;
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

int Push(LinkBinStack &S, BiTree tr, int a){
    LinkBinStack p = new BinStackNode;
    p->next = S;
    p->tree = tr;
    p->row = a;
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

int Pop(LinkBinStack &S, BiTree &tr, int &r){
    if(!S){
        cout << "PosStack Pop error!" << endl;
        exit(1);
    }
    tr = S->tree;
    r = S->row;
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

int CreateRandomBinTree_string(BiTree &T, int sig){
    if(rand() % 30 >= 31){//debug
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
    int levnum[26] = {0}, level = 0;
    char letter = 'A';
    while(Q.length && letter <= (sig + 'A' - 1)){
        DeBinQueue(Q, p);
        if(rand() % 30 < 30){//*****
            InsertBinNode(p->lchild, letter + to_string(levnum[level]));
            EnBinQueue(Q, p->lchild);
            rearnew = p->lchild;
            levnum[level]++;
        }
        if(rand() % 30 < 30){//*****
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

int PrintBinTree(BiTree T){
    int print_debug(string *strprint, string *strsym);
    if(!T)
        return 1;
    BiTree p = T;
    string strprint[30], strsym[30];
    int printlen = 0, row = 1, len;
    int sig = 1, pos[30] = {0}, mid[30] = {0};//pos存储每行最后一个节点最左端位置
                                              //mid存储符号层'|'位置

    LinkBinStack s = NULL;
    if(T->lchild){
        if(T->rchild){
            Push(s, T->rchild, 1);
        }
        T = T->lchild;
    }else{
        sig = 0;
        T = T->rchild;
    }

    strprint[0] += "*(ROOT)";
    strsym[0] += "**********************";
    pos[0] = 1;
    mid[0] = 3;
    while(s || T){
        if(sig){//sig为0表示其为右孩子，1为左孩子
            len = strprint[row].length();
            strprint[row].append(pos[row - 1] - len, ' ');//将新行与父母行对齐
            strprint[row] += '(' + T->data + ')';
            pos[row] = strprint[row].length() - T->data.length() - 2;//pos存储每行最右边的节点最左端位置
            //len = strprint[row].length() - T->data.length() / 2 - 2;//len存储在符号层中'|'之前添加了len个空格
            len = pos[row] - strsym[row].length() + (T->data.length() + 1) / 2;
            strsym[row].append(len, ' ');//在符号层'|'前添加空格
            strsym[row] += '|';
            printlen = mid[row];
            mid[row] = strsym[row].length() - 1;
            strprint[row - 1].insert(pos[row - 1], 2, '_');
            strprint[row - 1].insert(pos[row - 1], (T->data.length() + 1) / 2, ' '); //修改其父母节点的空格以及'_'
            len = (T->data.length() + 1) / 2 + 2;//当前节点双亲节点增加的位数
            strsym[row - 1].insert(printlen, len, ' ');//双亲节点对应符号层改变
            printlen = mid[row - 1];
            mid[row - 1] += len;
            //print_debug(strprint, strsym);
            for(int i = row - 2; i >= 0; i--){//对其父母节点以上的行增加空格以及'_'
                if(printlen < mid[i]){//i行为上一行的左孩子
                    if(printlen == mid[i + 1]){//i+1行的中点未变动
                        strprint[i].insert(printlen, len, strprint[i][printlen]);
                        strsym[i].insert(printlen, len, ' ');
                        printlen = mid[i];
                        mid[i] += len;
                    }else{
                        strprint[i].insert(printlen, len, strprint[i][printlen - 1]);
                        strsym[i].insert(printlen, len, ' ');
                        printlen = mid[i];
                        mid[i] += len;
                    }
                }else{
                    if(printlen == mid[i + 1]){
                        printlen = mid[i];
                    }else{
                        strprint[i].append(len, '_');
                        printlen = mid[i];
                    }
                }
                //print_debug(strprint, strsym);
            }
            //mid[row - 1] += len;
            if(T->lchild){
                sig = 1;
                if(T->rchild)
                    Push(s, T->rchild, row + 1);
                T = T->lchild;
                row++;
            }else{
                sig = 0;
                if(T->rchild){
                    sig = 0;
                    T = T->rchild;
                    row++;
                }else{
                    if(s)
                        Pop(s, T, row);
                    else
                        T = NULL;
                }
            }
        }else{//T指示的为右孩子时
            strprint[row - 1].append(2, '_');//给双亲节点右边添加两个'_'
            len = strprint[row - 1].length() - 1 - strsym[row].length();//符号层需要添加的空格数量
            strsym[row].append(len, ' ');
            strsym[row] += '|';//strsym的length - 1位置即为中点位置
            mid[row] = strsym[row].length() - 1;
            len = strsym[row].length() - (T->data.length() + 1) / 2;//当前行输入启始位置
            strprint[row].append(len - strprint[row].length() - 1, ' ');
            strprint[row] += '(' + T->data + ')';
            pos[row] = strprint[row].length() - 2 - T->data.length();
            len = strprint[row].length() - strsym[row].length() + 2;
            printlen = mid[row - 1];
            for(int i = row - 2; i >= 0; i--){
                if(printlen > mid[i]){//printlen存储下一行节点原中点位置 判断该行是否为右节点
                    if(printlen != mid[i + 1]){//判断中点位置是否变动
                        strprint[i].append(len, '_');
                        printlen = mid[i];
                    }else{
                        printlen = mid[i];
                    }
                }else{
                    if(printlen != mid[i + 1]){
                        strprint[i].insert(printlen, len, strprint[i][printlen - 1]);
                        strsym[i].insert(mid[i], len, ' ');
                        printlen = mid[i];
                        mid[i] += len;
                    }else{
                        strprint[i].insert(printlen, len, strprint[i][printlen]);
                        strsym[i].insert(mid[i], len, ' ');
                        printlen = mid[i];
                        mid[i] += len;
                    }
                }
                //print_debug(strprint, strsym);
            }
            if(T->lchild){
                sig = 1;
                if(T->rchild)
                    Push(s, T->rchild, row + 1);
                T = T->lchild;
                row++;
            }else{
                sig = 0;
                if(T->rchild){
                    sig = 0;
                    T = T->rchild;
                    row++;
                }else{
                    if(s)
                        Pop(s, T, row);
                    else
                        T = NULL;
                }
            }
        }
        //print_debug(strprint, strsym);
    }
    for(int i = 0; strprint[i].length() > 0; i++){
        cout << strprint[i] << endl;
        cout << strsym[i + 1] << endl;
    }
    return 1;
}

int print_debug(string *strprint, string *strsym){
    for(int i = 0; strprint[i].length() > 0; i++){
        cout << strprint[i] << endl;
        cout << strsym[i + 1] << endl;
    }
    return 1;
}

int main(){
    srand(time(NULL));
    BiTree T;
    CreateRandomBinTree_string(T, 4);
    PrintBinTree(T);
    return 0;
}