#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct board_head
{
    char board_type[8];
    char board_addr[8];
    char Ftype[2];
    char Error[14];
} BOARD_HEAD;

typedef struct frame_head
{
    char channel_id[8];
    char error[6];
    char Ftype[2];
    char length[16];
    //char timestamp_H[32];
    //char timestamp_L[32];
    char timestamp[64];
} FRAME_HEAD;

//根据is不同的返回值
int64_t struct_head_read(void *body, char is)
{
    if (!body || is == 0)
    {
        printf("Void pointer or error arg!");
        return -1;
    }
    char outbuf[4][32] = {'\0'};

    switch (is)
    {
    case 'b'://输出board info
        BOARD_HEAD *bb = (BOARD_HEAD *)body;
        memcpy(outbuf[0], bb->board_addr, 8);
        memcpy(outbuf[1], bb->board_type, 8);
        memcpy(outbuf[2], bb->Error, 14);
        memcpy(outbuf[3], bb->Ftype, 2);
        printf("**********BOARD INFO**********\n");
        printf("Board type:%s \nBoard addr:%s\nBoard Ftype:%s\nBoard Error:%s\n\n", outbuf[1],
               outbuf[0], outbuf[3], outbuf[2]);
        return 0;
        break;
    
    case 'f'://输出frame info
        FRAME_HEAD *cc = (FRAME_HEAD *)body;
        memcpy(outbuf[0], cc->channel_id, 8);
        memcpy(outbuf[1], cc->error, 6);
        memcpy(outbuf[2], cc->Ftype, 2);
        memcpy(outbuf[3], cc->length, 16);
        printf("**********FRAME INFO**********\n");
        printf("channel_id: %s\nError: %s\nFtype: %s\nLength: %s\n\n",
               outbuf[0], outbuf[1], outbuf[2], outbuf[3]);
        return atoi(outbuf[3]);
        break;

    case 'l'://返回adc data 的length（64位整型）
        FRAME_HEAD *cc = (FRAME_HEAD *)body;
        memcpy(outbuf[3], cc->length, 16);
        return atoi(outbuf[3]);
        break;

    case 'c'://返回adc data's channel_id(int64_)
        FRAME_HEAD *cc = (FRAME_HEAD *)body;
        memcpy(outbuf[0], cc->channel_id, 8);
        return atoi(outbuf[0]);
        break;

    case 't'://back adc data's timestamp(int64_)
        FRAME_HEAD *cc = (FRAME_HEAD *)body;
        char timest[65] = {'\0'};
        memcpy(timest, cc->timestamp, 64);
        return atoi(timest);
        break;
    }
}