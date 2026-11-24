#pragma once
#include <array>
#include <stdexcept>

namespace lucky {
struct Result { std::array<int,3> symbols; bool win; int balance; int number; };
class Game {
public:
    static constexpr int bet=50, reward=500;
    int balance=1000, spins=0, misses=0;
    template<class Draw> Result spin(Draw draw) {
        if(balance<bet) throw std::runtime_error("insufficient credits");
        std::array<int,3> s{draw(),draw(),draw()};
        for(int x:s) if(x<0||x>3) throw std::runtime_error("invalid symbol");
        // The first win is exactly spin 15. Later wins retain normal odds,
        // with an eight-spin safety limit so credits never block the trail.
        if(spins<14){
            if(s[0]==s[1]&&s[1]==s[2]) s[2]=(s[2]+1)%4;
        } else if(spins==14||misses>=7) s[1]=s[2]=s[0];
        const bool won=s[0]==s[1]&&s[1]==s[2];
        balance-=bet;
        if(won){balance+=reward;misses=0;} else ++misses;
        ++spins;
        return {s,won,balance,spins};
    }
};
}
