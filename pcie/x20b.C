//read hex to bin
#include <stdio.h>
#include "../recv_ana.h"
#include "../data_ana.h"
#include <stdlib.h>

int main(){
    FILE *fp;
    fp = fopen("data.log", "w+");
    if(fp == NULL){
        printf("Open fail!\n");
        return -1;
    }

    unsigned int data[1024];
    fread(data, sizeof(data), 4, fp);
    char buf[100];
    for(int i = 0; i < 1024; i++)
    {
        printf("%0b\n", data[i]);
        



    }
    



    return 0;
}