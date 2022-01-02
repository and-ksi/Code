//给定一个整数数组 nums 和一个整数目标值 target，
//请你在该数组中找出 和为目标值 target  的那 两个 整数，并返回它们的数组下标。



#include <stdio.h>
#include <malloc.h>

int *twoSum(int *nums, int numsSize, int target, int *returnSize)
{
    int *returnNum = NULL;
    for (int i = 0; i < numsSize - 1; i++)
    {
        for (int k = i + 1; k < numsSize; k++)
        {
            if (*(nums + i) + *(nums + k) == target)
            {
                returnNum = malloc(sizeof(int) * 2);
                *returnNum = i;
                *(returnNum + 1) = k;
                *returnSize = 2;
                return returnNum;
            }
        }
    }
    return returnNum;
}