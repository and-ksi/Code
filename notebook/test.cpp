#include<iostream>
#include <sstream>

using namespace std;

//测试流操作算子能否在无stream下使用
//显然不行
/* int teststream(){
    stringstream str;
    str << "10";
    string sss;
    sprintf(&sss[0], "test");
    cout << sss;
    return 1;
} */

int Stastic_value_test(int value){
  static int a = 0;
  a++;
  cout << a << endl;
  if(value > 0)
    Stastic_value_test(value - 1);
  else
    return 0;
}

int main(){
    Stastic_value_test(4);
    return 0;
}