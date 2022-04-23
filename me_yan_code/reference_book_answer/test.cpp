#include <iostream>
#include <cstring>

using namespace std;

int main(){
    const int b = 10;
    const int a = 20;
    const int *p = &b;
    p = &a;

    cout << *p << endl;
    
    return 0;
}