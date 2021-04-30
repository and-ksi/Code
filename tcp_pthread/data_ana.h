#ifndef DATA_ANA_H
#define DATA_ANA_H

#include "recv_ana2.h"

#endif

#define SQRT_SIZE               (100)//mm
#define INTERVAL_NUM            (33)//99mm
#define DELAY_EVERY_INTERVAL    (4)//ns
#define SIZE_EVERY_INTERVAL     (3)//mm
#define FREQUENCY               (20)//MHz
#define PERIOD_CYCLE            (50)//ns    ^^^^^   与频率相关联

#define ATTENUATION_COEFFICIENT (1.04)
#define DELAY_TIME              (500)//ns

#define ENERGY_OF_MOUN          (105.7)

//2500MHz时,延迟时间50次,20ns
//20MHz,延迟时间?
long long cfd_get_begintime(unsigned int *in_)
{
    if (in_ == NULL)
    {
        printf("Bit read error!\n");
        return 0;
    }

    unsigned int _cfd[1024];
    long long timestamp_ = bit_head_read(in_, 't');//ns
    int _length = (int)bit_head_read(in_, 'l');
    for (int i = 0; i < (2 * _length); i++)
    {
        _cfd[i] = bit_data_read(in_ + (i / 2), 'l', i & 1);
        if (i > DELAY_TIME/PERIOD_CYCLE - 1)
        {
            _cfd[i] += -ATTENUATION_COEFFICIENT * _cfd[i - DELAY_TIME / PERIOD_CYCLE];
            if (_cfd[i] == 0)
            {
                return timestamp_ + i * PERIOD_CYCLE;
            }
            else if (_cfd[i] * _cfd[i - 1] < 0)
            {
                return timestamp_ + 
                (long long)(PERIOD_CYCLE * (float)(_cfd[i] * (i - 1) / _cfd[i - 1]));
            }
        }
    }
    return 0;
}

//calculate position ;left+   right-
//take the center of square as zero pointer
//20MHz
double cal_position(unsigned int *s1, unsigned int *s2)
{
    long long t1 = bit_head_read(s1, 't');
    long long t2 = bit_head_read(s2, 't');

    return ((double)(t1 - t2) * 
    (double)(SIZE_EVERY_INTERVAL / (2 * DELAY_EVERY_INTERVAL)));
}

//calculate kinetic energy
//需要传入指向指针数组的指针
//channel: X1, X2, Y1, Y2, X3, X4, Y3, Y4
//延迟块间4ns, 间距3mm, 从一端到另一端完整132ns
double cal_energy(unsigned int *_in[8])
{
    double s[4];//s0, s1, s2, s3; xup, yup, xdo, ydo
    for (int i = 0; i < 4; i++)
    {
        s[i] = cal_position(*(_in + 2 * i), *(_in + 2 * i + 1));
    }
    long long t[8];
    for (int i = 0; i < 8; i++)
    {
        t[i] = bit_head_read(*(_in + i), 't');
    }
    double arrive_time[2];
    arrive_time[0] = ((t[0] + t[1]) - INTERVAL_NUM * DELAY_EVERY_INTERVAL
     - 2 * ((SQRT_SIZE / 2 - s[1]) * DELAY_EVERY_INTERVAL / SIZE_EVERY_INTERVAL)) / 2;
    arrive_time[1] = ((t[4] + t[5]) - INTERVAL_NUM * DELAY_EVERY_INTERVAL
     - 2 * ((SQRT_SIZE / 2 - s[3]) * DELAY_EVERY_INTERVAL / SIZE_EVERY_INTERVAL)) / 2;
    double vv = .25 / (abs(arrive_time[0] - arrive_time[1]) * 1e-9);
    return ENERGY_OF_MOUN * vv * vv;
}