#include <iostream>
#include <cstring>

int main(){
    using namespace std;
    int n, i, sig, start = 0, end;
    cin >> n;
    end = n;
    int *a = new int[n + 1];
    string b;
    b = b + 'R';
    cin >> b;
    for(i = 0; i <= n; i++){
        if(b[i] == 'L'){
            a[end] = i;
            //a[start] = i + 1;
            //start++;
            end--;
        }else{
            a[start] = i;
            start++;
        }
    }
    for(i = 0; i <= n; i++){
        cout << a[i] << endl;
    }
    return 0;
}