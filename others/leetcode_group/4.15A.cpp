//https: // vjudge.net/contest/489211#problem

#include <iostream>

typedef struct value{
    int num;
    struct value *next;
    struct value *front;
}*va;

int Sort(va &a, va &b){
    while(a->num < b->num){
        
    }
}

int GetValue(int n){
    if(n & 1){
        return 3 * n + 1;
    }
    return n/2;
}

int main(){
    using namespace std;
    int s;
    
    cin >> s;
    
}