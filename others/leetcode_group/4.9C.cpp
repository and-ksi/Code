#include <iostream>
#include <math.h>

int sort_3(int *a, int *b){
    int min = 0, k;
    for(int i = 0; i < 3; i++){
        for(k = 0; k < 3; k++){
            if(a[k] <= a[min]){
                min = k;
            }
        }
        b[i] = a[min];
        a[min] = INT_MAX;
    }
    return 0;
}

int opnem(int dec, int *a){
    int ssum[3], sum = 0, i;
    for(i = 0; i < 3; i++){
        ssum[i] = 9 - a[i];
        if((sum += ssum[i]) >= dec){
            break;
        }
    }
    return i + 1;
}

int opnem1(int dec, int *a){
    int sum = 0, i;
    for(i = 2; i >= 0; i--){
        if ((sum += a[i]) >= dec){
            break;
        }
    }
    return 3 - i;
}

int main(){
    using namespace std;
    string input;
    int a[3], b[3], c[6], str = 0, dec = 1;
    int suma, sumb, opa, opb, sumaa, sumbb;
    cin >> input;
    for(int i = 0; i < 6; i++){
        str += dec * (input[i] - '0');
        dec = dec * 10;
    }
    for(int i = 0; i < 6; i++){
        c[i] = str % 10;
        str = str / 10;
    }
    sort_3(&c[0], a);
    sort_3(&c[3], b);
    sumaa = a[0] + a[1] + a[2];
    sumbb = b[0] + b[1] + b[2];
    if(sumaa == sumbb){
        cout << 0 << endl;
        return 0;
    }
    if(sumaa > sumbb){
        dec = sumaa - sumbb;
        suma = opnem(dec, b);
        sumb = opnem1(dec, a);
    }else
    {
        dec = sumbb - sumaa;
        suma = opnem(dec, a);
        sumb = opnem1(dec, b);
    }
    if(suma > 2 && sumb > 2){
        if (9 - a[0] + b[2] >= abs(sumbb - sumaa) || 9 - b[0] + a[2] >= abs(sumaa - sumbb)){
            cout << 2 << endl;
            return 0;
        }
    }
    if(suma <= sumb){
        cout << suma << endl;
        return 0;
    }
    cout << sumb << endl;
    return 0;
}