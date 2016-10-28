//
// Created by okhowang on 2016/10/28.
//
#include "Renju.hpp"

Renju::Type type_list[1024 * 1024];

constexpr uint32_t mask_k(int k) {
    return k == 1 ? Renju::Key::kSelf : mask_k(k - 1) << 2 | Renju::Key::kSelf;
}

constexpr uint32_t full_mask_k(int k) {
    return k == 1 ? 3 : full_mask_k(k - 1) << 2 | 3;
}

constexpr uint32_t full_mask_long() {
    return full_mask_k(6);
}

constexpr uint32_t mask_long() {
    return mask_k(6);
}

constexpr uint32_t mask_5() {
    return mask_k(5);
}

constexpr uint32_t full_mask_5() {
    return full_mask_k(5);
}

bool match(uint32_t key, uint32_t mask, uint32_t val) {
    return (key & mask) == val;
}

bool CheckLong(uint32_t key) {
    for (int i = 0; i < 4; ++i) {
        if (match(key, (full_mask_long() << i * 2), (mask_long() << i * 2)))
            return true;
    }
    return false;
}

bool Check5(uint32_t key) {
    for (int i = 0; i < 5; ++i) {
        if (match(key, full_mask_5() << i * 2, (mask_5() << i * 2)))
            return true;
    }
    return false;
}

Renju::Type Check4(uint32_t key) {
    int count = 0;
    for (int i = 0; i < 9; ++i) {
        if ((key & (0x3 << (i * 2))) == 0) {
            if (Check5(key | (Renju::Key::kSelf << (i * 2))) &&
                !CheckLong(key | (Renju::Key::kSelf << (i * 2))))
                count++;
        }
    }
    if (count > 1)
        return Renju::Type::kFlex4;
    if (count == 1)
        return Renju::Type::kBlock4;
    return Renju::Type::kDefault;
}

Renju::Type Check3(uint32_t key) {
    int count_flex4 = 0;
    int count_block4 = 0;
    for (int i = 0; i < 9; ++i) {
        if ((key & (0x3 << (i * 2))) == 0) {
            auto type = Check4(key | (Renju::Key::kSelf << (i * 2)));
            switch (type) {
                case Renju::Type::kBlock4:
                    count_block4++;
                    break;
                case Renju::Type::kFlex4:
                    count_flex4++;
                    break;
            }
        }
    }
    if (count_flex4)
        return Renju::Type::kFlex3;
    if (count_block4)
        return Renju::Type::kBlock3;
    return Renju::Type::kDefault;
}

Renju::Type Check2(uint32_t key) {
    int count_flex3 = 0;
    int count_block3 = 0;
    for (int i = 0; i < 9; ++i) {
        if ((key & (0x3 << (i * 2))) == 0) {
            auto type = Check3(key | (Renju::Key::kSelf << (i * 2)));
            switch (type) {
                case Renju::Type::kBlock3:
                    count_block3++;
                    break;
                case Renju::Type::kFlex3:
                    count_flex3++;
                    break;
            }
        }
    }
    if (count_flex3)
        return Renju::Type::kFlex2;
    if (count_block3)
        return Renju::Type::kBlock2;
    return Renju::Type::kDefault;
}

Renju::Type GetKeyType(uint32_t key) {
    if (CheckLong(key))return Renju::Type::kLong;
    if (Check5(key))return Renju::Type::k5;
    auto type = Check4(key);
    if (type != Renju::Type::kDefault)return type;
    type = Check3(key);
    if (type != Renju::Type::kDefault)return type;
    type = Check2(key);
    if (type != Renju::Type::kDefault)return type;
    return Renju::Type::kDefault;
}

void func(uint32_t key, int deep) {
    for (int i = 0; i < (1 << 18); ++i)
        type_list[i] = GetKeyType(i);
}

const char *GetDesc(uint32_t key) {
    static char buf[65535];
    char *p = buf;
    for (int i = 0; i < 9; ++i) {
        switch (key & 0x3) {
            case Renju::Key::kEmpty:
                p += sprintf(p, "0");
                break;
            case Renju::Key::kSelf:
                p += sprintf(p, "s");
                break;
            default:
                p += sprintf(p, "b");
                break;
        }
        key >>= 2;
    }
    return buf;
}

int main(int argc, char **argv) {
    FILE *file = fopen("table.cpp", "w");
    if (file == NULL) {
        fprintf(stderr, "open file error\n");
        return -1;
    }
    for (int i = 0; i < sizeof(type_list) / sizeof(type_list[0]); ++i) {
        type_list[i] = Renju::Type::kDefault;
    }
    func(0, 8);
    fprintf(file, "int g_patternTable[] = {\n");
    for (int i = 0; i < sizeof(type_list) / sizeof(type_list[0]); ++i) {
        fprintf(file, "%d, //%d %s\n", type_list[i], i, GetDesc(i));
    }
    fprintf(file, "};\n");
    fclose(file);
}