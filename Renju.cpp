#include "Renju.hpp"

#include <ctime>
#include <cassert>

#include <tuple>
#include <algorithm>


const int Renju::direct_list[4][2] = {
        {1, 0},
        {0, 1},
        {1, 1},
        {1, -1},
};
const int Renju::sign_list[2] = {-1, 1};

Renju::Renju(int size, bool forbid) : has_forbid_(forbid), size_(size),
                                      black_count_(0), white_count_(0), start_(std::chrono::system_clock::now()) {
    data_.assign(size * size, Pos::kEmpty);
    pos_types.assign(size * size,
                     {Type::kDefault, Type::kDefault, Type::kDefault, Type::kDefault, Type::kDefault, Type::kDefault,
                      Type::kDefault, Type::kDefault});
    frame.frame_x_max = INT_MIN;
    frame.frame_y_max = INT_MIN;
    frame.frame_x_min = INT_MAX;
    frame.frame_y_min = INT_MAX;
}

Renju::~Renju() {
}

void Renju::SetPos(int x, int y, Pos role, bool update) {
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
    if (role != Pos::kEmpty && update) {
        UpdatePosTypes(x, y);
        UpdateFrame(x, y);
    }
}

void Renju::Init() {
    srand(time(NULL));
    for (int i = 0; i < size_; ++i) {
        for (int j = 0; j < size_; ++j) {
            if (Get(i, j) != Pos::kEmpty) {
                UpdateFrame(i, j);
//                for (int k = 0; k < 4; ++k) {
//                    GetPosType(i, j).typeinfo[0][k] = GetKeyType(GetKey(Role::kBlack, i, j, k));
//                    GetPosType(i, j).typeinfo[1][k] = GetKeyType(GetKey(Role::kWhite, i, j, k));
//                }
            }
        }
    }
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
        auto res = GetNextImpl(role, i, deep);
        //最后一层也直接返回
        if (std::get<2>(res) == TypeValue::kValue5 || i == deep)
            return std::make_pair(std::get<0>(res), std::get<1>(res));
    }
}

//x y value

std::tuple<int, int, int> Renju::GetNextImpl(Role role, int deep, int check_deep, int alpha, int beta,
                                             std::pair<int, int> *suggest) {
    std::vector<std::pair<int, int>> res;
    int max = INT_MIN;
    auto list = GenMoveList(role == Role::kBlack ? Pos::kBlack : Pos::kWhite);
    for (auto &p : list) {
        //if(Timeout())throw std::exception();
        int x = std::get<0>(p);
        int y = std::get<1>(p);
        SetPos(x, y, GetByRole(role));
        int result = GetPosResult(x, y);
        int v;
        //如果禁手和勝利也不用再往下搜了
        if (deep > 0 && result != 1 && result != -1) {
            auto res = GetNextImpl(GetOpponent(role), deep - 1, check_deep, -beta, -alpha);
            v = -std::get<2>(res);
        }
            //普通搜索到头了的话 尝试搜杀招
        else if (deep == 0 && check_deep > 0 && result == 2) {
            auto res = GetNextImpl(GetOpponent(role), 0, check_deep - 1, -beta, -alpha);
            v = -std::get<2>(res);
        }
        else {
            v = Score(role);
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
    if (res.empty())
        throw "has no position";
    int i = 0;//rand() % res.size();
    return std::make_tuple(res[i].first, res[i].second, max);
}

bool Renju::IsValidPoint(int x, int y) {
    return x >= 0 && x < size_ && y >= 0 && y < size_;
}
//取最大值的点的值

uint32_t Renju::GetKey(Role role, int x, int y, int direct) {
    uint32_t key = kSelf << 8;
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
                if (IsSame(role, p)) {
                    key |= (sign > 0 ? (kSelf << 8 << (k * 2)) : (kSelf << 8 >> (k * 2)));
                }
                else if (p == Pos::kEmpty) {
                    key |= (sign > 0 ? (kEmpty << 8 << (k * 2)) : (kEmpty << 8 >> (k * 2)));
                }
                else {
                    key |= (sign > 0 ? (kOppnonent << 8 << (k * 2)) : (kOppnonent << 8 >> (k * 2)));
                }
            }
            else {
                key |= (sign > 0 ? (kOppnonent << 8 << (k * 2)) : (kOppnonent << 8 >> (k * 2)));
            }
        }
    }
    assert(key & (kSelf << 8));
    return key;
}

extern int g_patternTable[];

Renju::Type Renju::GetKeyType(uint32_t key) {
    return static_cast<Type>(g_patternTable[key]);
}

Renju::Pos &Renju::Get(int x, int y) {
    return data_[x * size_ + y];
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

std::vector<std::tuple<int, int, int> > Renju::GenMoveList(Pos pos) {
    std::vector<std::tuple<int, int, int> > res1;
    std::vector<std::tuple<int, int, int> > res2;
    for (int x = std::max(0, frame.frame_x_min - 2); x < std::min(size_, frame.frame_x_max + 3); ++x)
        for (int y = std::max(0, frame.frame_y_min - 2); y < std::min(size_, frame.frame_y_max + 3); ++y) {
            if (Get(x, y) != Pos::kEmpty)continue;
            if (HasNear(x, y, 1)) {
                res1.emplace_back(x, y, 0);
            }
            if (HasNear(x, y, 2)) {
                res2.emplace_back(x, y, 0);
            }
        }
    res1.insert(res1.end(), std::make_move_iterator(res2.begin()), std::make_move_iterator(res2.end()));
    for (auto &p : res1) {
        SetPos(std::get<0>(p), std::get<1>(p), pos);
        std::get<2>(p) = GetPosResult(std::get<0>(p), std::get<1>(p));
        SetPos(std::get<0>(p), std::get<1>(p), Pos::kEmpty);
    }
    std::sort(res1.begin(), res1.end(), [this](const std::tuple<int, int, int> &a, const std::tuple<int, int, int> &b) -> bool {
        return std::get<2>(a) < std::get<2>(b);
    });
    return res1;
}

Renju::Role Renju::GetOpponent(Role role) {
    return role == Role::kBlack ? Role::kWhite : Role::kBlack;
}

void  Renju::UpdatePosTypes(int x, int y) {
    int a, b;
    for (int i = 0; i < 4; ++i) {
        int dx = direct_list[i][0];
        int dy = direct_list[i][1];
        a = x + dx;
        b = y + dy;

        for (int k = 0; k < 4 && IsValidPoint(a, b); a += dx, b += dy, ++k) {
            GetPosType(a, b).typeinfo[0][i] = GetKeyType(GetKey(Role::kBlack, a, b, i));
            GetPosType(a, b).typeinfo[1][i] = GetKeyType(GetKey(Role::kWhite, a, b, i));
        }

        a = x - dx;
        b = y - dy;
        for (int k = 0; k < 4 && IsValidPoint(a, b); a -= dx, b -= dy, ++k) {
            GetPosType(a, b).typeinfo[0][i] = GetKeyType(GetKey(Role::kBlack, a, b, i));
            GetPosType(a, b).typeinfo[1][i] = GetKeyType(GetKey(Role::kWhite, a, b, i));
        }
    }
}

void  Renju::SumupTypeinfos(Role role, int x, int y, int res[Type::kMax]) {
    Type type[4];
//    type[0] = GetPosType(x, y).typeinfo[role == Role::kBlack ? 0 : 1][0];
//    type[1] = GetPosType(x, y).typeinfo[role == Role::kBlack ? 0 : 1][1];
//    type[2] = GetPosType(x, y).typeinfo[role == Role::kBlack ? 0 : 1][2];
//    type[3] = GetPosType(x, y).typeinfo[role == Role::kBlack ? 0 : 1][3];
    type[0] = GetKeyType(GetKey(role, x, y, 0));
    type[1] = GetKeyType(GetKey(role, x, y, 1));
    type[2] = GetKeyType(GetKey(role, x, y, 2));
    type[3] = GetKeyType(GetKey(role, x, y, 3));

    ++res[type[0]];
    ++res[type[1]];
    ++res[type[2]];
    ++res[type[3]];
}

int   Renju::Score(Role role) {
    int value_me = 0;
    int value_op = 0;
    for (int x = 0; x < size_; ++x)
        for (int y = 0; y < size_; ++y) {
            auto pos = Get(x, y);
            if (pos == Pos::kEmpty) continue;

            if (IsSame(role, pos))     //只对棋盘上已有的子进行算分
            {
                int res[Type::kMax] = {0};
                SumupTypeinfos(role, x, y, res);
                value_me += TypeValue::kValue5 * res[Type::k5]
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
                SumupTypeinfos(role == Role::kBlack ? Role::kWhite : Role::kBlack, x, y, res);
                value_op += TypeValue::kValue5 * res[Type::k5]
                            + TypeValue::kValueFlex4 * res[Type::kFlex4]
                            + TypeValue::kValueBlock4 * res[Type::kBlock4]
                            + TypeValue::kValueFlex3 * res[Type::kFlex3]
                            + TypeValue::kValueBlock3 * res[Type::kBlock3]
                            + TypeValue::kValueFlex2 * res[Type::kFlex2]
                            + TypeValue::kValueBlock2 * res[Type::kBlock2]
                            + TypeValue::kValueDefault * res[Type::kDefault];
            }
        }
    return value_me - value_op;
}

void Renju::UpdateFrame(int x, int y) {
    if (x < frame.frame_x_min) frame.frame_x_min = x;
    if (x > frame.frame_x_max) frame.frame_x_max = x;
    if (y < frame.frame_y_min) frame.frame_y_min = y;
    if (y > frame.frame_y_max) frame.frame_y_max = y;
}

int Renju::GetPosResult(int x, int y) {
    const auto &pos = Get(x, y);
    assert(pos != Pos::kEmpty);
    int res[Type::kMax] = {0};
    SumupTypeinfos(pos == Pos::kBlack ? Role::kBlack : Role::kWhite, x, y, res);
    if (has_forbid_ && pos == Pos::kBlack) {
        if (res[Type::kLong] || (res[Type::k5] == 0 && (res[Type::kFlex4] > 1 || res[Type::kFlex3] > 1)))
            return -1;
    }
    if (res[Type::kLong] || res[Type::k5] || res[Type::kFlex4] || (res[Type::kBlock4] + res[Type::kFlex3] > 1))return 1;
    if (res[Type::kBlock4] || res[Type::kFlex3])return 2;
    return 0;
}

std::string Renju::Debug() {
    std::string res;
    fprintf(stderr, "   ");
    for (int i = 0; i < size_; ++i) {
        fprintf(stderr, "%-3d", i);
    }
    fprintf(stderr, "\n");
    for (int i = 0; i < size_; ++i) {
        fprintf(stderr, "%-3d", i);
        for (int j = 0; j < size_; ++j)
            switch (Get(i, j)) {
                case Pos::kBlack:
                    fprintf(stderr, "%-3c", 'b');
                    break;
                case Pos::kWhite:
                    fprintf(stderr, "%-3c", 'w');
                    break;
                default:
                    fprintf(stderr, "%-3c", '_');
                    break;
            }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
    return res;
}