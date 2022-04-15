#include <iostream>
#include <cstring>

typedef struct Road{
    struct Road *road1, *road2;
    int town1, town2, mark, time;
}Road;

typedef struct Town{
    struct Road *road;
}Town;

typedef struct City{
    int townnum, roadnum;
    Town *town;
    Road *road;
}City;

int roadpush(Road *ro, Road *&des, int sig){
    if(des == NULL){
        des = ro;
        return 1;
    }
    if(sig == des->town1){
        roadpush(ro, des->road1, sig);
    }else{
        roadpush(ro, des->road2, sig);
    }
    return 1;
}

int findroad(Road *ro, int mar, int sig, int &time, int to){
    if((ro->town1 == to && sig == ro->town2) || (ro->town2 == to && sig == ro->town1)){
        ro->mark = mar;
        time += ro->time;
        return 1;
    }
    if(ro->mark == mar){
        return 0;
    }
    int time1, time2 = INT_MAX, to1 = ro->town1, to2 = ro->town2;
    Road *rom = ro, *ro1 = ro->road1, *ro2 = ro->road2;
    if(sig == ro->town2){
        to1 = ro->town2, to2 = ro->town1;
        ro1 = ro->road2, ro2 = ro->road1;
    }
    time1 = time = time + ro->time;
    sig = to2;
    while (ro2){
        if (findroad(ro2, mar, sig, time, to)){
            if (time < time2){
                time2 = time;
            }
        }else{
            time = time1;
            ro2 = ro1;
        }
    }
    if (time2 == INT_MAX){
        return 0;
    }
    return 1;
}

int main(){
    using namespace std;
    City city;
    cin >> city.townnum >> city.roadnum;
    Town *to = city.town = new Town[city.townnum];
    memset(to, 0, sizeof(Town) * city.townnum);
    Road *ro = city.road = new Road[city.roadnum];
    memset(ro, 0, sizeof(Road) * city.roadnum);
    for(int i = 0; i < city.roadnum; i++){
        cin >> ro[i].town1 >> ro[i].town2 >> ro[i].time;
        roadpush(&ro[i], to[ro[i].town1 - 1].road, ro[i].town1);
        roadpush(&ro[i], to[ro[i].town2 - 1].road, ro[i].town2);
    }
    for(int i = 0; i < city.townnum; i++){

    }
    return 0;
}