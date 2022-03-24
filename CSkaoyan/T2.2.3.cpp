// P17
#include <iostream>

#define ElemType int
#define MaxSize 1024

typedef struct SqList{
    ElemType data[MaxSize];
    int length;
}SqList;

typedef struct SqList{
    ElemType *data;
    int length;
}SeqList;

//T1
bool delete_min_elem(SeqList &seqlist, ElemType &res){//忘记元素类型不一定为int，需要引用参数返回值
    if(seqlist.length <= 0){
        std::cout << "error" << std::endl;
        return false;
    }
    //int min = __INT_MAX__, pos = 0;
    int pos = 0;
    res = seqlist.data[0];
    for(int i = 0; i < seqlist.length; i++){
        if(seqlist.data[i] < res){
            res = seqlist.data[i];
            pos = i;
        }
    }
    seqlist.data[pos] = seqlist.data[seqlist.length - 1];//忘记-1
    seqlist.length--;//忘记--   需要--吗？
    return true;
}

//T2
bool reverse_list(SeqList &seqlist){//修改太多了！！！
    ElemType temp;
    int i = seqlist.length - 1;
    for(int k = 0; k < seqlist.length/2; k++){
        temp = seqlist.data[k];
        seqlist.data[k] = seqlist.data[i];
        seqlist.data[i] = temp;
        i--;
    }
    return true;
}

int main(){
    return 0;
}