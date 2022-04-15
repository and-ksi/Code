#include <iostream>

int main(){
    using namespace std;
    double a, b, c;
    cin >> a >> b >> c;
    if(a < b){
        if (c >= a && c < b)
        {
            cout << "Yes" << endl;
            return 0;
        }
        cout << "No" << endl;
    }else{
        if (c >= a || c < b)
        {
            cout << "Yes" << endl;
            return 0;
        }
        cout << "No" << endl;
    }
    return 0;
}