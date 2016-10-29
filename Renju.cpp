#include "Renju.hpp"

#include <ctime>
#include <cassert>

#include <tuple>
#include <algorithm>
#include  <stdarg.h>


const int Renju::direct_list[4][2] = {
        {1, 0},
        {0, 1},
        {1, 1},
        {1, -1},
};
const int Renju::sign_list[2] = {-1, 1};

#ifndef NDEBUG
FILE *g_log_fp = fopen("log.txt", "w+");
#define LOG(...)     Log(g_log_fp, __VA_ARGS__)
#endif

void Log(FILE *fp, const char *fmt, ...) {
    char fmt_buf[256];
    snprintf(fmt_buf, 256, "%s", fmt);

    va_list arg_ptr;
    va_start(arg_ptr, fmt);
    vfprintf(fp, fmt_buf, arg_ptr);
    va_end(arg_ptr);
    fflush(fp);
}

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

    has_near_.assign(size * size, 0);
}

Renju::~Renju() {
}

void Renju::SetPos(int x, int y, Pos role, bool update) {
    auto &pos = Get(x, y);
    //update near by
    if (role != Pos::kEmpty && pos == Pos::kEmpty) {
        for (int i = x - 2; i <= x + 2; ++i)
            for (int j = y - 2; j <= y + 2; ++j)
                if ((i != x || j != y) &&
                    IsValidPoint(i, j))
                    has_near_[i * size_ + j]++;
    }
    else if (role == Pos::kEmpty && pos != Pos::kEmpty) {
        for (int i = x - 2; i <= x + 2; ++i)
            for (int j = y - 2; j <= y + 2; ++j)
                if ((i != x || j != y) &&
                    IsValidPoint(i, j))
                    has_near_[i * size_ + j]--;
    }

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
        last_move.x = x;
        last_move.y = y;
    }
    if (role != Pos::kEmpty && update) {
//        UpdatePosTypes(x, y);
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
    auto list = GenMoveList(role);
    for (auto &p : list) {
        //if(Timeout())throw std::exception();
        int x = std::get<0>(p);
        int y = std::get<1>(p);
        SetPos(x, y, GetByRole(role));
        int result = std::get<2>(p);
        if (result >= 10000)  //已经赢了，直接返回
            return std::make_tuple(x, y, 10000000);

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

bool Renju::HasNear(int x, int y) {
    return has_near_[x * size_ + y];
}

std::vector<std::tuple<int, int, int> > Renju::GenMoveList(Role role) {
    std::vector<std::tuple<int, int, int> > res;
    for (int x = std::max(0, frame.frame_x_min - 2); x < std::min(size_, frame.frame_x_max + 3); ++x)
        for (int y = std::max(0, frame.frame_y_min - 2); y < std::min(size_, frame.frame_y_max + 3); ++y) {
            if (Get(x, y) != Pos::kEmpty)continue;
            if (HasNear(x, y)) {
                res.emplace_back(x, y, 0);
            }
        }
    for (auto &p : res) {
        int x = std::get<0>(p);
        int y = std::get<1>(p);
        const auto &pos = Get(x, y);
        uint32_t key[4] = {
                GetKey(role, x, y, 0),
                GetKey(role, x, y, 1),
                GetKey(role, x, y, 2),
                GetKey(role, x, y, 3),
        };
        uint32_t op_key[4] = {
                GetKey(GetOpponent(role), x, y, 0),
                GetKey(GetOpponent(role), x, y, 1),
                GetKey(GetOpponent(role), x, y, 2),
                GetKey(GetOpponent(role), x, y, 3),
        };
        std::get<2>(p) = GetPosResult(key, op_key, role);
    }
    std::sort(res.begin(), res.end(),
              [this](const std::tuple<int, int, int> &a, const std::tuple<int, int, int> &b) -> bool {
                  return std::get<2>(a) > std::get<2>(b);
              });
    return res;
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

void Renju::DumpBoard(FILE *fp) {
    LOG("   ");
    for (int i = 0; i < size_; ++i) {
        LOG("%-3d", i);
    }
    LOG("\n");
    for (int i = 0; i < size_; ++i) {
        LOG("%-3d", i);
        for (int j = 0; j < size_; ++j)
            switch (Get(i, j)) {
                case Pos::kBlack:
                    LOG("%-3c", 'b');
                    break;
                case Pos::kWhite:
                    LOG("%-3c", 'w');
                    break;
                default:
                    LOG("%-3c", '-');
                    break;
            }
        LOG("\n");
    }
    LOG("\n");
}

bool IsDefaultPosType(Renju::PosType type) {
    bool ret = true;
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 4; ++j) {
            if (type.typeinfo[i][j] != Renju::Type::kDefault) {
                ret = false;
                break;
            }
        }
    return ret;
}

void Renju::DumpAllPosTypes() {
    LOG("\n\n");
	PosType default_pos_type = { Type::kDefault, Type::kDefault,
								Type::kDefault, Type::kDefault,
								Type::kDefault, Type::kDefault,
								Type::kDefault, Type::kDefault };
    for (int i = 0; i < size_; ++i)
        for (int j = 0; j < size_; ++j) {
            PosType pos_type = GetPosType(i, j);
            if (IsDefaultPosType(pos_type)) continue;

            LOG("pos(%d, %d) for black: \n", i, j);
            for (int dir = 0; dir < 4; ++dir) {
                if (pos_type.typeinfo[0][dir] != kDefault) {
                    LOG("dir(%d) ---- %d\n", dir, (int) pos_type.typeinfo[0][dir]);
                }
            }

            LOG("pos(%d, %d) for white: \n", i, j);
            for (int dir = 0; dir < 4; ++dir) {
                if (pos_type.typeinfo[1][dir] != kDefault) {
                    LOG("dir(%d) ---- %d\n", dir, (int) pos_type.typeinfo[1][dir]);
                }
            }
        }
}

void  Renju::SumupTypeinfos(uint32_t key[4], int res[Type::kMax]) {
    Type type[4];
//    type[0] = GetPosType(x, y).typeinfo[role == Role::kBlack ? 0 : 1][0];
//    type[1] = GetPosType(x, y).typeinfo[role == Role::kBlack ? 0 : 1][1];
//    type[2] = GetPosType(x, y).typeinfo[role == Role::kBlack ? 0 : 1][2];
//    type[3] = GetPosType(x, y).typeinfo[role == Role::kBlack ? 0 : 1][3];
    type[0] = GetKeyType(key[0]);
    type[1] = GetKeyType(key[1]);
    type[2] = GetKeyType(key[2]);
    type[3] = GetKeyType(key[3]);

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
                uint32_t key[4] = {
                        GetKey(role, x, y, 0),
                        GetKey(role, x, y, 1),
                        GetKey(role, x, y, 2),
                        GetKey(role, x, y, 3),
                };
                int res[Type::kMax] = {0};
                SumupTypeinfos(key, res);
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
                uint32_t key[4] = {
                        GetKey(role == Role::kBlack ? Role::kWhite : Role::kBlack, x, y, 0),
                        GetKey(role == Role::kBlack ? Role::kWhite : Role::kBlack, x, y, 1),
                        GetKey(role == Role::kBlack ? Role::kWhite : Role::kBlack, x, y, 2),
                        GetKey(role == Role::kBlack ? Role::kWhite : Role::kBlack, x, y, 3),
                };
                int res[Type::kMax] = {0};
                SumupTypeinfos(key, res);
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

int Renju::GetPosResult(uint32_t key[4], uint32_t op_key[4], Role role) {
    int res[Type::kMax] = {0};
    int oppo_res[Type::kMax] = { 0 };
    SumupTypeinfos(key, res);
    SumupTypeinfos(op_key, oppo_res);
    if (has_forbid_) {
        if (role == Role::kBlack) {
            if (res[Type::k5] > 0)  return 10000;  //10000不能随便改
            if (oppo_res[Type::kLong] || oppo_res[Type::k5] > 0)  return 5000;

            if (res[Type::kFlex4] == 1) return 3000;
            if (oppo_res[Type::kFlex4] > 0 || oppo_res[Type::kBlock4] > 1) return 1500;

            if (res[Type::kBlock4] == 1 && res[Type::kFlex3] == 1)  return 1000;
            if (oppo_res[Type::kBlock4] > 0 && oppo_res[Type::kFlex3] > 0)  return 500;

            if (res[Type::kLong] || (res[Type::k5] == 0 &&
                (res[Type::kFlex4] > 1 || res[Type::kBlock4] > 1 || res[Type::kFlex3] > 1)))
                return -1;
        }
        else {
            if (res[Type::kLong] || res[Type::k5] > 0)
                return 10000;     //10000不能随便改
            if (oppo_res[Type::k5] > 0)
                return 5000;

            if (res[Type::kFlex4] > 0 || res[Type::kBlock4] > 1)
                return 3000;
            if (oppo_res[Type::kFlex4] == 1)
                return 1500;

            if (res[Type::kBlock4] > 0 && res[Type::kFlex3] > 0)
                return 1000;
            if (oppo_res[Type::kBlock4] ==1 && oppo_res[Type::kFlex3] == 1)
                return 500;

            if (res[Type::kBlock4] || res[Type::kFlex3])
                return 200;
        }
    }
    else {
        if (res[Type::kLong] || res[Type::k5] > 0)
            return 10000;     //10000不能随便改
        if (oppo_res[Type::kLong] || oppo_res[Type::k5] > 0)
            return 5000;

        if (res[Type::kFlex4] > 0 || res[Type::kBlock4] > 1)
            return 3000;
        if (oppo_res[Type::kFlex4] > 0 || oppo_res[Type::kBlock4] > 1)
            return 1500;

        if (res[Type::kBlock4] > 0 &&  res[Type::kFlex3] > 0)
            return 1000;
        if (oppo_res[Type::kBlock4] > 0 && oppo_res[Type::kFlex3] > 0)
            return 1000;

        if (res[Type::kBlock4] || res[Type::kFlex3])
            return 200;
    }

    return 15 * res[Type::kBlock4]
        + 15 * res[Type::kFlex3]
        + 8 * res[Type::kBlock3]
        + 8 * res[Type::kFlex2];
}


std::pair<int, int> Renju::Solve(Role role, int depth) {
    if (role == Role::kBlack) {
        if (black_count_ == 0) {
            //黑子第一手
            return std::make_pair(size_ / 2, size_ / 2);
        }
        if (black_count_ == 1) {
            //TODO: 黑子第二手，优先级低
        }
    }
    else {
        if (white_count_ == 0) {
            //TODO: 白子要参考黑子位置
            //白子第一手下旁边随便一个
            auto &random = direct_list[rand() % 4];
            return std::make_pair(size_ / 2 + random[0], size_ / 2 + random[1]);
        }
    }

    total_cnt = 0;
    leaf_cnt = 0;
    max_depth = depth;   //最大深度
    for (int i = 2; i <= max_depth && best_val < 10000; i += 2) {
        max_iter_depth = i;
        best_val = MinMaxSearch(role, i, -10001, 10000);
    }

    LOG("solved best move: (%d, %d)\n", best_move.x, best_move.y);
    return std::make_pair(best_move.x, best_move.y);
}

const int max_move_num = 3;

int  Renju::MinMaxSearch(Role role, int cur_depth, int alpha, int beta) {
    ++total_cnt;

    std::vector<std::pair<int, int>> res;
    int max = INT_MIN;
    auto list = GenMoveList(role);
    if (list.size() > max_move_num)   list.erase(list.begin() + max_move_num, list.end());  //限制最多走法

    LOG("top level has %d moves:\n", list.size());
    for (auto &p : list) {
        LOG("top-move (%d, %d) val=%d\n", std::get<0>(p), std::get<1>(p), std::get<2>(p));
    }

    int val;
    for (auto &p : list) {
        int x = std::get<0>(p);
        int y = std::get<1>(p);
        SetPos(x, y, GetByRole(role));

        val = -AlphaBetaSearch(GetOpponent(role), cur_depth - 1, -beta, -alpha);
        LOG("top level move (%d, %d) score=%d\n", x, y, val);

        SetPos(x, y, Pos::kEmpty);

        if (val >= beta) {
            best_move.x = x;
            best_move.y = y;
            return val;
        }

        if (val > alpha) {
            best_move.x = x;
            best_move.y = y;
            alpha = val;
        }
    }

    return alpha;   //TODO?    
}

int  Renju::AlphaBetaSearch(Role role, int cur_depth, int alpha, int beta) {
    ++total_cnt;

    //TODO: 检查对方是否已赢

    if (0 == cur_depth) {
        ++leaf_cnt;
        return Score(role);
    }

    auto list = GenMoveList(role);
    if (list.size() > max_move_num)   list.erase(list.begin() + max_move_num, list.end());  //限制最多走法

    LOG("%d level moves:\n", max_iter_depth - cur_depth);
    for (auto &p : list) {
        LOG("%d-move (%d, %d) val=%d\n", max_iter_depth - cur_depth, std::get<0>(p), std::get<1>(p), std::get<2>(p));
    }

    int val;
    for (auto &p : list) {
        int x = std::get<0>(p);
        int y = std::get<1>(p);
        SetPos(x, y, GetByRole(role));

        val = -AlphaBetaSearch(GetOpponent(role), cur_depth - 1, -beta, -alpha);
        LOG("%d level move (%d, %d) score=%d\n", max_iter_depth - cur_depth, x, y, val);

        SetPos(x, y, Pos::kEmpty);

        if (val >= beta) {
            LOG("cut by val(%d) >= beta(%d)\n", val, beta);
            return val;
        }

        if (val > alpha) {
            alpha = val;
        }
    }

    return alpha;
}