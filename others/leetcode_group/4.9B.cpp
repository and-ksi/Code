#include <iostream>

int main(){
    using namespace std;
    int *n, quo = 1, i, t;
    while(cin >> t){
        n = new int[t];
        for(int k = 0; k < t; k++){
            cin >> n[k];
        }

        for(int k = 0; k < t; k++){
            for(i = 3; ; ){
                if(!(n[k] % i)){
                    cout << n[k] / i << endl;
                    break;
                }
                i = (i << 1) | 1;
            }
        }

        delete n;
    }
}

/* int main(){
    using namespace std;
    int *n, quo = 1, i = 1, t = 0;
    while(cin >> t){
        n = new int[t];
        for (int k = 0; k < t; k++)
        {
            cin >> n[k];
        }
        for (int k = 0; k < t; k++)
        {
            while (quo)
            {
                if (n[k] % i)
                {
                    i++;
                    continue;
                }
                quo = n[k] / i;
                while (quo & 0x1)
                {
                    quo = quo >> 1;
                }
                if (!quo)
                {
                    cout << i << endl;
                }
                i++;
            }
            i = 1;
            quo = 1;
        }
        delete n;
    }
    return 0;
} */