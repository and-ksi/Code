#include <iostream>

#define NUM_TYPE long long

using namespace std;

int get_complement(int num){
    return num;
}

int get_true(int num){
    if(num >= 0){
        return num;
    }else
    {
        return (-num) | 0x80000000;
    }
}

int get_ocomplement(int num){
    if(num >= 0){
        return num;
    }else
    {
        return ~(-num);
    }
}

int get_shift(int num){
    return num ^ 0x80000000;
}

int print_bit(int num){
    
}