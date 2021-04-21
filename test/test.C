/* #include <stdio.h>
#include <string.h>

typedef struct board_head
{
    char board_type[8];
    char board_addr[8];
    char Ftype[2];
    char Error[14];
} BOARD_HEAD;

int main(){
    BOARD_HEAD board_head;
    memset(&board_head, '0', sizeof(board_head));
    memset(&board_head.board_type, '1', 8);
    memset(&board_head.board_addr, '2', 8);
    memset(&board_head.Ftype, '3', 2);
    memset(&board_head.Error, '5', 14);

    char buf[10];
    memset(&buf, '1', 10);
    char ll[32];
    memset(&ll, '0', 32);
    memcpy(&ll, (const void *)&board_head, 32);
    int a = 39;
    sprintf(buf, "%d", a);
    printf("Struct body: %s\n", ll);
    printf("Buf : %s\n", buf);
    putchar(buf[3]);
    return 0;
} */
//  1.struct是按照字符顺序存储和读取的
//  2.向数组中注入数字会自动在后边加入结束字符

#include <stdio.h>
#include <string.h>

int main(){
    int a = 4;
    float b;
    b = 1.0/3.0;
    char buf[20];
    memset(&buf, '\0', 20);
    printf("a = %08d, b = %f \n", a, b);
    sprintf(buf, "%*.11f", 12, b);
    printf("%s\n", buf);
}
