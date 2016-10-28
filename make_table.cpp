//
// Created by okhowang on 2016/10/28.
//
#include "Renju.hpp"

Renju::Type type_list[1024 * 1024];

bool CheckLong(uint32_t key){
    for (int i = 0; i < 4; ++i) {
        if (key & (0b010101010101 << i))return true;
    }
    return false;
}

Renju::Type GetKeyType(uint32_t key) {
    //长连
    for (int i = 0; i < 4; ++i) {
        if (key & (0b010101010101 << i))return Renju::Type::kLong;
    }
    //5
    for (int i = 0; i < 5; ++i) {
        if (key & (0b0101010101 << i))return Renju::Type::k5;
    }
    //活4
    for (int i = 0; i < 6; ++i) {
        if (key & (0b01010101 << i))return Renju::Type::kFlex4;
    }
    //眠4
    for(int i=0;i<9;++i){
        if(key & Renju::Key::kEmpty){

        }
    }
}

void func(uint32_t key, int deep) {
    for (int pos = 0; pos < 3; ++pos) {
        key |= (pos << deep);
        if (deep == 0)type_list[key] = GetKeyType(key);
        else func(key, deep - 1);
    }
}

int main(int argc, char **argv) {
    func(0, 8);
}