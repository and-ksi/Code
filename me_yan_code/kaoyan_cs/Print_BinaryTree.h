#ifndef Print_BinTree
#define Print_BinTree
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <string.h>
#include <iomanip>
using namespace std;
#ifndef ElemType
#define ElemType string
#endif

typedef struct BiTNode{//二叉树节点需要定义在头文件内部
    ElemType data;
    struct BiTNode *lchild, *rchild;
}BiTNode, *BiTree;

typedef struct __Print_BiTNode{//二叉树节点
    string data;
    struct __Print_BiTNode *lchild, *rchild;
}__Print_BiTNode, *__Print_BiTree;

int __Print_TranToString(BiTNode *T, __Print_BiTNode *&N){
    if(!T){
        N = NULL;
        return 0;
    }
    N = new __Print_BiTNode;
    stringstream str;
    str << T->data;
    str >> N->data;
    __Print_TranToString(T->lchild, N->lchild);
    __Print_TranToString(T->rchild, N->rchild);

    return 0;
}

typedef struct __Print_BinStackNode{
    __Print_BiTree tree;
    int row;
    struct __Print_BinStackNode *next;
}__Print_BinStackNode, *__Print_LinkBinStack;

int __Print_Push(__Print_LinkBinStack &S, __Print_BiTree tr){
    __Print_LinkBinStack p = new __Print_BinStackNode;
    p->next = S;
    p->tree = tr;
    S = p;
    return 1;
}

int __Print_Push(__Print_LinkBinStack &S, __Print_BiTree tr, int a){
    __Print_LinkBinStack p = new __Print_BinStackNode;
    p->next = S;
    p->tree = tr;
    p->row = a;
    S = p;
    return 1;
}

int __Print_Pop(__Print_LinkBinStack &S, __Print_BiTree &tr){
    if(!S){
        cout << "PosStack __Print_Pop error!" << endl;
        exit(1);
    }
    tr = S->tree;
    __Print_LinkBinStack p = S;
    S = S->next;
    delete p;
    return 1;
}

int __Print_Pop(__Print_LinkBinStack &S, __Print_BiTree &tr, int &r){
    if(!S){
        cout << "PosStack __Print_Pop error!" << endl;
        exit(1);
    }
    tr = S->tree;
    r = S->row;
    __Print_LinkBinStack p = S;
    S = S->next;
    delete p;
    return 1;
}

int __Print_PrintBinTree(__Print_BiTree T){
    int __Print_print_debug(string *strprint, string *strsym);
    if(!T)
        return 1;
    __Print_BiTree p = T;
    string strprint[30], strsym[30];
    int printlen = 0, row = 1, len;
    int sig = 1, pos[30] = {0}, mid[30] = {0};//pos存储每行最后一个节点最左端位置
                                              //mid存储符号层'|'位置

    __Print_LinkBinStack s = NULL;
    if(T->lchild){
        if(T->rchild){
            __Print_Push(s, T->rchild, 1);
        }
        T = T->lchild;
    }else{
        sig = 0;
        T = T->rchild;
    }

    strprint[0] += " (ROOT)";
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
            //__Print_print_debug(strprint, strsym);
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
                //__Print_print_debug(strprint, strsym);
            }
            //mid[row - 1] += len;
            if(T->lchild){
                sig = 1;
                if(T->rchild)
                    __Print_Push(s, T->rchild, row + 1);
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
                        __Print_Pop(s, T, row);
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
                //__Print_print_debug(strprint, strsym);
            }
            if(T->lchild){
                sig = 1;
                if(T->rchild)
                    __Print_Push(s, T->rchild, row + 1);
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
                        __Print_Pop(s, T, row);
                    else
                        T = NULL;
                }
            }
        }
        //__Print_print_debug(strprint, strsym);
    }
    for(int i = 0; strprint[i].length() > 0; i++){
        cout << strprint[i] << endl;
        cout << strsym[i + 1] << endl;
    }
    return 1;
}

int Print_Binary_Tree(BiTNode *T){
    __Print_BiTNode *N;
    __Print_TranToString(T, N);
    __Print_PrintBinTree(N);
    return 0;
}

#endif