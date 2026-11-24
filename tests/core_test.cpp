#include "../native/core.hpp"
#include <cassert>
#include <random>
#include <iostream>
int main(){
    for(unsigned seed=0;seed<10000;++seed){
        lucky::Game g; std::mt19937 r(seed); int gap=0;
        for(int j=0;j<160;++j){
            int before=g.balance;
            auto s=g.spin([&](){return int(r()%4);});
            assert(s.number==j+1);
            assert(s.win==(s.symbols[0]==s.symbols[1]&&s.symbols[1]==s.symbols[2]));
            assert(g.balance==before-50+(s.win?500:0));
            if(j<14)assert(!s.win);
            if(j==14)assert(s.win);
            if(s.win)gap=0;else ++gap;
            assert(j<14||gap<8); assert(g.balance>=0);
        }
    }
    lucky::Game forced;int n=0;
    for(int i=0;i<15;i++){
        auto s=forced.spin([&](){return (n++)%4;});
        assert(s.win==(i==14));
    }
    lucky::Game all;
    for(int i=0;i<14;++i){auto early=all.spin([](){return 2;});assert(!early.win);}
    auto win=all.spin([](){return 2;});
    assert(win.win&&win.balance==750);
    all.balance=49;bool thrown=false;
    try{all.spin([](){return 0;});}catch(const std::runtime_error&){thrown=true;}
    assert(thrown&&all.balance==49);
    std::cout<<"PASS: 1,600,000 spins, first win exactly at spin 15, no earlier wins, payouts, invalid balance\n";
}
