#include "ana.h"

unsigned int data[1024];

void read_data(){
    FILE *fp = fopen("log_save/analys_data.log", "r");
    if(fp == NULL){
        printf("file open failed!\n");
        exit(1);
    }

    for(int c = 0; c < 264; c++){
        fscanf(fp, "%x\n", data + c);
        //printf("c = %d, %x\n", c, data[c]);
    }
    fclose(fp);
}

void write_data(){
    FILE *fp = fopen("log_save/analys_resualt.log", "w+");
    if (fp == NULL)
    {
        printf("file open failed!\n");
        exit(1);
    }

    int ret = 0, addr = 0;
    int channel, timestamp, length;
    double data1, data2;

    for(int c = 0; c < 8; c++){
        ret = find_adc_head(data + addr, 0, 0);
        if(ret == 100){
            break;
        }
        addr += ret;
        channel = bit_head_read(data + addr, 'c');
        length = bit_head_read(data + addr, 'l');
        timestamp = bit_time_read(data + addr);
        addr += 4;
        fprintf(fp, "channel = %d   timestamp = %x %d   length = %d\n", channel, timestamp, timestamp, length);
        for(int i = 0; i < length; i++){
            data1 = bit_float_read(data + addr + i, 0);
            data2 = bit_float_read(data + addr + i, 1);
            if(data1 == 5.){
                break;
            }
            fprintf(fp, "%lf\n", data1);
            if(data2 == 5.){
                break;
            }
            fprintf(fp, "%lf\n", data2);
        }
        addr += length;
    }
    fclose(fp);
}

int main(){
    read_data();
    write_data();
    return 0;
}