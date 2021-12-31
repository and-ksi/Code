#include "../tcp_pthread/recv_ana.h"

int main(){
    unsigned int mid[3];
    scanf("%x", mid);
    scanf("%x", mid + 1);
    scanf("%x", mid + 2);
    bit_head_read(mid, 'f');
    return 0;
}