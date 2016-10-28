#include "Renju.hpp"

#include <cassert>

const int Renju::direct_list[4][2] = {
        {1, 0},
        {0, 1},
        {1, 1},
        {1, -1},
};
const int Renju::sign_list[2] = {-1, 1};
std::pair<bool, Renju::Type> Renju::patternCache[1024 * 1024];

Renju::Renju(int size, bool forbid) : has_forbid_(forbid), size_(size),
                                      black_count_(0), white_count_(0) {
    data_.assign(size * size, Pos::kEmpty);
}

Renju::~Renju() {
}

void Renju::SetPos(int x, int y, Pos role) {
    auto &pos = Get(x, y);
    switch (pos) {
        case Pos::kBlack:
            black_count_--;
            break;
        case Pos::kWhite:
            white_count_--;
            break;
        default:
            break;
    }
    pos = role;
    switch (role) {
        case Pos::kBlack:
            black_count_++;
            break;
        case Pos::kWhite:
            white_count_++;
            break;
        default:
            break;
    }
    if (role != Pos::kEmpty) {
        UpdatePosTypes(x, y);
        UpdateFrame(x, y);
    }
}

void Renju::Init() {
    srand(time(NULL));
}

std::pair<int, int> Renju::GetNext(Role role, int deep) {
    if (role == Role::kBlack) {
        if (black_count_ == 0) {
            //黑子第一手
            return std::make_pair(size_ / 2, size_ / 2);
        }
        if (black_count_ == 1) {
            //            int max = 0;
            //            std::pair<int, int> res;
            //            for (int x = size_ / 2 - 2; x <= size_ / 2 + 2; ++x)
            //            {
            //                for (int y = 0; y < size_; ++y)
            //                {
            //                    if (Get(x, y) == Pos::kEmpty && HasNear(x, y))
            //                    {
            //
            //                    }
            //                }
            //            }
        }
    }
    else {
        if (white_count_ == 0) {
            //白子第一手下旁边随便一个
            auto &random = direct_list[rand() % 4];
            return std::make_pair(size_ / 2 + random[0], size_ / 2 + random[1]);
        }
    }
    //迭代加深 如果找到胜着就直接返回
    for (int i = 0; i <= deep; i += 2) {
        auto res = GetNextImpl(role, i);
        //最后一层也直接返回
        if (std::get<2>(res) == TypeValue::kValue5 || i == deep)
            return std::make_pair(std::get<0>(res), std::get<1>(res));
    }
}

//x y value

std::tuple<int, int, int> Renju::GetNextImpl(Role role, int deep, int alpha, int beta) {
    std::vector<std::pair<int, int>> res;
    int max = INT_MIN;
    auto list = GenMoveList();
    for (auto &p : list) {
        int x = p.first;
        int y = p.second;
        SetPos(x, y, GetByRole(role));
        auto &pos_type = GetPosType(x, y).typeinfo[role == Role::kBlack ? 0 : 1];
        bool win = (pos_type[0] == Type::k5);
        int v;
        if (deep > 0 && !win) {
            auto res = GetNextImpl(GetOpponent(role), deep - 1, -beta, -alpha);
            v = -std::get<2>(res);
        }
        else if (!win) {
            v = Score();
        }
        SetPos(x, y, Pos::kEmpty);
        if (v > max) {
            max = v;
            if (max >= beta)
                return std::make_tuple(x, y, max); //这里比beta大 可以忽略结果直接返回
            if (max > alpha)
                alpha = max; //更新alpha值
            res.clear();
            res.emplace_back(x, y);
        }
        else if (v == max) {
            res.emplace_back(x, y);
        }
    }
    if (res.empty())throw "has no position";
    int i = rand() % res.size();
    return std::make_tuple(res[i].first, res[i].second, max);
}

bool Renju::IsValidPoint(int x, int y) {
    return x >= 0 && x < size_ && y >= 0 && y < size_;
}
//取最大值的点的值

int Renju::ComputeValue(Role role) {
    int max = INT_MIN;
    for (int x = 0; x < size_; ++x)
        for (int y = 0; y < size_; ++y) {
            auto pos = Get(x, y);
            if (IsSame(role, pos)) {
                int value = ComputePosValue(x, y);
                if (value > max)
                    max = value;
            }
        }
    return max;
}

uint32_t Renju::GetKey(int x, int y, int direct) {
    uint32_t key = kSelf << 8;
    auto raw = Get(x, y);
    auto &d = direct_list[direct];
    for (auto sign : sign_list) {
        int i = x;
        int j = y;
        int k;
        for (k = 1; k < 5; k++) {
            i += sign * d[0];
            j += sign * d[1];
            if (IsValidPoint(i, j)) {
                auto p = Get(i, j);
                if (raw == p) {
                    key |= sign > 0 ? (kSelf << 8 << k * 2) : (kSelf << 8 >> k * 2);
                }
                else if (p == Pos::kEmpty) {
                    key |= sign > 0 ? (kEmpty << 8 << k * 2) : (kEmpty << 8 >> k * 2);
                }
                else {
                    key |= sign > 0 ? (kOppnonent << 8 << k * 2) : (kOppnonent << 8 >> k * 2);
                }
            }
            else {
                key |= sign > 0 ? (kOppnonent << 8 << k * 2) : (kOppnonent << 8 >> k * 2);
            }
        }
    }
    assert(key & (kSelf << 8));
    return key;
}

Renju::Type Renju::GetKeyType(uint32_t key) {
    if (patternCache[key].first)
        return patternCache[key].second;
            \

    assert(key & (kSelf << 8));
    Pos line[9];
    for (int i = 0; i < 9; ++i) {
        switch (key & 0x3) {
            case Key::kEmpty:
                line[i] = Pos::kEmpty;
                break;
            case Key::kSelf:
                line[i] = Pos::kBlack;
                break;
            default:
                line[i] = Pos::kWhite;
                break;
        }
        key >>= 2;
    }
    Type type = TypeLine(Role::kBlack, line);
    patternCache[key].first = true;
    return patternCache[key].second = type;
}

int Renju::ComputePosValue(int x, int y, bool *win) {
    if (win)*win = false;
    uint8_t res[Type::kMax] = {0};
    auto raw = Get(x, y);
    for (int d = 0; d < 4; ++d) {
        int key = GetKey(x, y, d);
        Type type = GetKeyType(key);
        res[type]++;
    }
    if (raw == Pos::kBlack && has_forbid_) {
        if ((res[Type::k5] == 0 && (res[Type::kFlex4] > 1 || res[Type::kFlex3] > 1))
            || res[Type::kLong])
            return TypeValue::kValueForbid; //禁手
    }
    if (res[Type::k5] || res[Type::kLong] || res[Type::kFlex4] || res[Type::kBlock4] + res[Type::kFlex3] > 1) {
        if (win)*win = true;
        return TypeValue::kValue5; //赢了
    }
    return TypeValue::kValueBlock4 * res[Type::kBlock4]
           + TypeValue::kValueFlex3 * res[Type::kFlex3]
           + TypeValue::kValueBlock3 * res[Type::kBlock3]
           + TypeValue::kValueFlex2 * res[Type::kFlex2]
           + TypeValue::kValueBlock2 * res[Type::kBlock2]
           + TypeValue::kValueDefault * res[Type::kDefault];
}

Renju::Pos &Renju::Get(int x, int y) {
    return data_[x * size_ + y];
}

bool Renju::IsSame(Role role, Pos pos) {
    return (pos == Pos::kBlack && role == Role::kBlack) ||
           (pos == Pos::kWhite && role == Role::kWhite);
}

Renju::Pos Renju::GetByRole(Role role) {
    if (role == Role::kBlack)return Pos::kBlack;
    return Pos::kWhite;
}

bool Renju::HasNear(int x, int y, const int distance) {
    for (int i = x - distance; i <= x + distance; ++i)
        for (int j = y - distance; j <= y + distance; ++j)
            if ((i != x || j != y) &&
                IsValidPoint(i, j) && Get(i, j) != Pos::kEmpty)
                return true;
    return false;
}

std::vector<std::pair<int, int> > Renju::GenMoveList() {
    std::vector<std::pair<int, int> > res1;
    std::vector<std::pair<int, int> > res2;
    for (int x = 0; x < size_; ++x)
        for (int y = 0; y < size_; ++y) {
            if (Get(x, y) != Pos::kEmpty)continue;
            if (HasNear(x, y, 1)) {
                res1.emplace_back(x, y);
            }
            if (HasNear(x, y, 2)) {
                res2.emplace_back(x, y);
            }
        }
    //    res1.insert(res1.end(), std::make_move_iterator(res2.begin()), std::make_move_iterator(res2.end()));
    return res1;
}

Renju::Role Renju::GetOpponent(Role role) {
    return role == Role::kBlack ? Role::kWhite : Role::kBlack;
}

Renju::Type Renju::TypeLine(Role role, Pos line[]) {
    int k;
    int empty = 0; //空格数
    int block = 0;
    int max_len = 1; //己方棋子的最大长度, 如XX_X_中的X最大长度位5
    int border_len = 1; //己方棋子的边界长度，如XX_X_中的X边界长度为4
    int count = 1; //连续的己方棋子数

    //正方向
    for (k = 5; k < 9; k++) {
        Pos curPos = line[k];
        bool stop_flag = false;

        switch (curPos) {
            case Pos::kOutside: //到边界了
                if (border_len == empty + count) //上一个点是己方棋子
                    block++;
                stop_flag = true;
                break;
            case Pos::kEmpty: //遇到空格
                max_len++;
                empty++;
                break;
            default:
                if (IsSame(role, curPos)) { //是己方棋子
                    if (empty > 2 || empty + count > 4)
                        stop_flag = true;
                    else {
                        count++;
                        max_len++;
                        border_len = empty + count;
                    }
                }
                else { //遇到对方棋子
                    if (border_len == empty + count) //上一个点是己方棋子
                        block++;
                    stop_flag = true;
                }
                break;
        }
        if (stop_flag) break;
    }

    // 计算中间空格
    empty = border_len - count;

    //反方向
    for (k = 3; k >= 0; --k) {
        Pos curPos = line[k];
        bool stop_flag = false;

        switch (curPos) {
            case Pos::kOutside: //到边界了
                if (border_len == empty + count) //上一个点是己方棋子
                    block++;
                stop_flag = true;
                break;
            case Pos::kEmpty: //遇到空格
                max_len++;
                empty++;
                break;
            default:
                if (IsSame(role, curPos)) { //是己方棋子
                    if (empty > 2 || empty + count > 4)
                        stop_flag = true;
                    else {
                        count++;
                        max_len++;
                        border_len = empty + count;
                    }
                }
                else { //遇到对方棋子
                    if (border_len == empty + count) //上一个点是己方棋子
                        block++;
                    stop_flag = true;
                }
                break;
        }
        if (stop_flag) break;
    }

    if (max_len >= 5 && count > 1) {
        if (count == 5)
            return k5;
        if (max_len > 5 && border_len < 5 && block == 0) {
            switch (count) {
                case 2:
                    return kFlex2;
                case 3:
                    return kFlex3;
                case 4:
                    return kFlex4;
            }
        }
        else {
            switch (count) {
                case 2:
                    return kBlock2;
                case 3:
                    return kBlock3;
                case 4:
                    return kBlock4;
            }
        }
    }

    return Type::kDefault;
}

Renju::Type Renju::TypeLine(Role role, int x, int y, int i, int j) {
    int a, b, k;

    Pos line[9];
    if (role == Role::kBlack) line[4] = Pos::kBlack;
    else line[4] = Pos::kWhite;

    //正方向
    a = x;
    b = y;
    for (k = 0; k < 4; k++) {
        a += i;
        b += j;

        Pos curPos = Get(a, b);
        line[k + 5] = curPos;
    }

    //反方向
    a = x;
    b = y;
    for (k = 0; k < 4; k++) {
        a -= i;
        b -= j;

        Pos curPos = Get(a, b);
        line[3 - k] = curPos;
    }

    return TypeLine(role, line);
}

void  Renju::UpdatePosTypes(int x, int y) {
    int a, b;
    for (int i = 0; i < 4; ++i) {
        int dx = direct_list[i][0];
        int dy = direct_list[i][1];
        a = x + dx;
        b = y + dy;

        for (int k = 0; k < 4 && IsValidPoint(a, b); a += dx, b += dy, ++k) {
            GetPosType(a, b).typeinfo[0][i] = TypeLine(Role::kBlack, a, b, dx, dy);
            GetPosType(a, b).typeinfo[1][i] = TypeLine(Role::kWhite, a, b, dx, dy);
        }

        a = x - dx;
        b = y - dy;
        for (int k = 0; k < 4 && IsValidPoint(a, b); a -= dx, b -= dy, ++k) {
            GetPosType(a, b).typeinfo[0][i] = TypeLine(Role::kBlack, a, b, dx, dy);
            GetPosType(a, b).typeinfo[1][i] = TypeLine(Role::kWhite, a, b, dx, dy);
        }
    }
}

void  Renju::SumupTypeinfos(Role role, int x, int y, int res[Type::kMax]) {
    Type type[4];
    if (role == Role::kBlack) {
        type[0] = GetPosType(x, y).typeinfo[0][0];
        type[1] = GetPosType(x, y).typeinfo[0][1];
        type[2] = GetPosType(x, y).typeinfo[0][2];
        type[3] = GetPosType(x, y).typeinfo[0][3];
    }
    else {
        type[0] = GetPosType(x, y).typeinfo[1][0];
        type[1] = GetPosType(x, y).typeinfo[1][1];
        type[2] = GetPosType(x, y).typeinfo[1][2];
        type[3] = GetPosType(x, y).typeinfo[1][3];
    }

    ++res[type[0]];
    ++res[type[1]];
    ++res[type[2]];
    ++res[type[3]];
}

int   Renju::Score() {
    int value_black;
    int value_white;
    for (int x = frame.frame_x_min; x <= frame.frame_x_max; ++x)
        for (int y = frame.frame_y_min; y < frame.frame_y_max; ++y) {
            auto pos = Get(x, y);
            if (pos == Pos::kEmpty) continue;

            if (IsSame(Role::kBlack, pos))     //只对棋盘上已有的子进行算分
            {
                int res[Type::kMax] = {0};
                SumupTypeinfos(Role::kBlack, x, y, res);
                value_black += TypeValue::kValue5 * res[Type::k5]
                               + TypeValue::kValueFlex4 * res[Type::kFlex4]
                               + TypeValue::kValueBlock4 * res[Type::kBlock4]
                               + TypeValue::kValueFlex3 * res[Type::kFlex3]
                               + TypeValue::kValueBlock3 * res[Type::kBlock3]
                               + TypeValue::kValueFlex2 * res[Type::kFlex2]
                               + TypeValue::kValueBlock2 * res[Type::kBlock2]
                               + TypeValue::kValueDefault * res[Type::kDefault];
            }
            else {
                int res[Type::kMax] = {0};
                SumupTypeinfos(Role::kBlack, x, y, res);
                value_white += TypeValue::kValue5 * res[Type::k5]
                               + TypeValue::kValueFlex4 * res[Type::kFlex4]
                               + TypeValue::kValueBlock4 * res[Type::kBlock4]
                               + TypeValue::kValueFlex3 * res[Type::kFlex3]
                               + TypeValue::kValueBlock3 * res[Type::kBlock3]
                               + TypeValue::kValueFlex2 * res[Type::kFlex2]
                               + TypeValue::kValueBlock2 * res[Type::kBlock2]
                               + TypeValue::kValueDefault * res[Type::kDefault];
            }
        }
    return value_black - value_white;
}

void Renju::UpdateFrame(int x, int y) {
    if (x < frame.frame_x_min) frame.frame_x_min = x;
    if (x > frame.frame_x_max) frame.frame_x_max = x;
    if (y < frame.frame_y_min) frame.frame_y_min = y;
    if (y > frame.frame_y_max) frame.frame_y_max = y;
}

