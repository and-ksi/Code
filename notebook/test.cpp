#include<iostream>
#include <sstream>

using namespace std;

//测试流操作算子能否在无stream下使用
//显然不行
int teststream(){
    stringstream str;
    str << "10";
    string sss;
    sprintf(&sss[0], "test");
    cout << sss;
    return 1;
}

/*
 __(45)
   |
 (1234)

"     ______(A0)"
"********", '_' <repeats 13 times>, "(ROOT)"
*/

int main(){
    teststream();
    return 0;
}