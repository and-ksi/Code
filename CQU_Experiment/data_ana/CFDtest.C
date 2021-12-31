#include <stdio.h>
#include <math.h>

float TrackData[8][1024];

float average_func(int count, float *pointer)
{ //对输入浮点数组前count个求平均
    float sum;
    for(int i = 0; i < count; i++){
        sum += *(pointer + i);
    }
    return (sum/count);
}

//返回给定数组T的前n个元素最小值的位置
int LocMin(int n, float *T){
    if(n <= 0 || !T)
        return -1;
    float Min = *T;
    int loc = 0;
    for(int i = 0; i < n; i++){
        if(*(T + i) < Min)
        {
            Min = *(T + i);
            loc = i;
        }
    }
    return loc;
}

//恒比定时方法--AnaCheck.C 有问题
void cfd(){
    float Tch[8] = {0};
    float CFDmean, CFDdata[1024];
    float CFDfactor = 1.05;
    int CFDoffset = 50;//这是20ns对应的探测数量
    int CFDthresh = -60;

    for(int ich = 0; ich < 8; ich ++){
        CFDmean = average_func(100, TrackData[ich]);//通过取前一百平均值取基线
        for(int i = 0; i < 1024; i++){
            CFDdata[i] = TrackData[ich][i] - CFDmean;
            if(i >= CFDoffset)
                CFDdata[i] += CFDfactor * (CFDmean - TrackData[ich][i - CFDoffset]);
        }
        int T0 = LocMin(800, CFDdata);
        if(CFDdata[T0] > CFDthresh){
            Tch[ich] = 0;
            continue;
        }
        for(int i = 0; i < 1000; i++){
            if(CFDdata[T0] == 0){
                Tch[ich] = i;
                break;
            }else if(CFDdata[i] * CFDdata[i + 1] < 0){
                Tch[ich] = (float)i + abs(CFDdata[i] / (CFDdata[i] - CFDdata[i + 1]));
                break;
            }
        }
    }
}
