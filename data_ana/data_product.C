#include <sys/time.h>
#include <stdlib.h>
#include <iostream>


void data_product(){
    struct timeval time1;
    printf("%d", sizeof(time1));
}

int main(){
    data_product();
    return 0;
}