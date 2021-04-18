//测试指针相关的知识
#include <stdio.h>

int main(){
	int a[10],*p=&a[1],*m;
	*m=&a[1];
	printf("%d\n",*p);
	return 0;
}