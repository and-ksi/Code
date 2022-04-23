#include <iostream>

int main(){
    using namespace std;
    int i, t, n[200], k[200], s;
    cin >> t;
    for(i = 0; i < t; i++){
        cin >> n[i] >> k[i];
    }
    for(i = 0; i < t; i++){
        s = n[i] / k[i];
        if(s & 0x1) {cout << "Alice" << endl;}
        else {cout << "Bob" << endl;}
    }
    return 0;
}