//printf("debug: \n");
#include "ana.h"

//从socket接收并进行改变
int channel_num = 8;
int min_channel = 0;
int end_location;
int board_num = 1024;
int client_num = 1;

//功能控制
int work;
FILE *error_fp; //错误报告
FILE *log_save;
int file_count = 0;

//计数
int recv_count;

unsigned int recved_pack[PACK_SIZE];

LOCA_TIME list_adc[8][2560 / 8 * 50];

void debug_data_read()
{
    FILE *co_fp = fopen("log_save/correct_data_withddddcccc.log", "r");
    if (co_fp == NULL)
    {
        printf("open debug data failed!\n");
        exit(1);
    }
    int write_ll = 0;
    for (int i = 0; i < 2560; i++)
    {
        for (int k = 0; k < 1024; k++)
        {
            fscanf(co_fp, "%x\n", recved_pack + write_ll);
            write_ll++;
        }
    }
    fclose(co_fp);
    // write_data_error_log(&error_fp, recved_pack, 50, 0);
    // exit(1);
}

void sort_data(){
    //LOCA_TIME (*list_adc)[8] = (LOCA_TIME (*)[8])malloc(sizeof(LOCA_TIME) * 50 * 2200);
    if(list_adc == NULL){
        printf("create list_adc failed!\n");
        exit(1);
    }
    memset(list_adc, 0, sizeof(list_adc));

    int loca = 0, start_loca, ret;
    int _channel, adc_count[8] = {0};
    char buf[50] = {'\0'};
    LOCA_TIME ll_adc;
    FILE *fp[8];

    for(int board_count = 0; board_count < 2560; board_count++){
        ret = find_board_head(recved_pack + board_count * 1024, 0, 0);
        if(ret == 100){
            printf("can not find board head\n");
            continue;
        }
        
        loca = board_count * 1024 + ret;
        start_loca = loca;
        printf("find board head ret = %d, loca = %d     %x\n", ret, loca, recved_pack[loca]);
        for(; loca - start_loca < 1023;){
            if (loca >= PACK_SIZE - 10)
            {
                printf("loca error: loca = %d\n", loca);
                break;
            }
            ret = find_adc_head(recved_pack + loca, 0, 0);
            if(ret == 100){
                printf("can not find adc head\n");
                break;
            }
            loca += ret;
            if(loca - start_loca >= 1024){
                break;
            }
            printf("find adc head ret = %d, loca = %d       %x\n", ret, loca, recved_pack[loca]);
            _channel = bit_head_read(recved_pack + loca, 'c');
            printf("channel = %d, adc_count[%d] = %d\n", _channel, _channel, adc_count[_channel]);
            list_adc[_channel][adc_count[_channel]].m_channel = _channel;
            printf("0001\n");
            list_adc[_channel][adc_count[_channel]].m_location = loca;
            list_adc[_channel][adc_count[_channel]].m_timestamp = bit_time_read(recved_pack + loca);
            list_adc[_channel][adc_count[_channel]].m_length = bit_head_read(recved_pack + loca, 'l');
            loca += list_adc[_channel][adc_count[_channel]].m_length;
            
            for (int i = adc_count[_channel]; i > 0; i--)
            {
                if (list_adc[_channel][i].m_timestamp < list_adc[_channel][i - 1].m_timestamp)
                {
                    memcpy(&ll_adc, list_adc[_channel] + i, sizeof(LOCA_TIME));
                    memcpy(list_adc[_channel] + i, list_adc[_channel] + i - 1, sizeof(LOCA_TIME));
                    memcpy(list_adc[_channel] + i - 1, &ll_adc, sizeof(LOCA_TIME));
                }else{
                    break;
                }
            }
            
            adc_count[_channel]++;
        }
    }

    // for(int c = 0; c < 8; c++){
    //     sprintf(buf, "channel_%d.log", c + 1);
    //     fp[c] = fopen(buf, "w+");
    //     if(fp == NULL){
    //         printf("%s open failed!\n", buf);
    //         exit(1);
    //     }
    //     for(int i = 0; i < adc_count[c]; i++){
    //         fprintf(fp[c], "%d :   channel = %d    timestamp = %llx, loca = %d      %x, %x ,%x, %x, %x\n", i, c, list_adc[c][i].m_timestamp, list_adc[c][i].m_location, recved_pack[list_adc[c][i].m_location - 2], recved_pack[list_adc[c][i].m_location - 1], recved_pack[list_adc[c][i].m_location], recved_pack[list_adc[c][i].m_location + 1], recved_pack[list_adc[c][i].m_location + 2]);
    //     }
    //     fclose(fp[c]);
    // }
}

void save_data(){
    
}

int main(){

    debug_data_read();

    sort_data();
    return 0;
}