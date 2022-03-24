#include <iostream>
#include <string>

using namespace std;

int pra1_3(){
    cout << "Hello, World。" << endl;
    return 0;
}

int pra1_4(){
    cout << "Enter two numbers:" << endl;
    int n1, n2;
    cin >> n1 >> n2;
    cout << "The product is " << n1 * n2 << endl;
    return 0;
}

int test2_28(){//getline()不能返回非值
    string line;
    while(getline(cin, line)){
        cout << line << endl;
    }
    return 0;
}

int main(){
    //return pra1_3();
    //return pra1_4();
    return test2_28();
}