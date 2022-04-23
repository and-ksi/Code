#include <iostream>
#include <cmath>
#define MAX 998244353
int Find(int a, int n, int x){
    int sum = 0;
    if(pow(10, n) * x > MAX) return 0;
    
    if(n == 1){
        if(a - 1 > 0) sum ++;
        if(a + 1 < 10) sum ++;
        sum ++;
    }else{
        if(a - 1 > 0) sum += Find(a - 1, n - 1, x * 10 + a - 1);
        if(a + 1 < 10) sum += Find(a + 1, n - 1, x * 10 + a + 1);
        sum += Find(a, n - 1, x * 10 + a);
    }
    return sum;
}

int main(){
    using namespace std;
    int n, i, sum = 0;
    cin >> n;
    for(i = 1; i < 10; i++){
        sum += Find(i, n - 1, i);
    }
    cout << sum << endl;
    return 0;
}