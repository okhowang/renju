
#include <future>
#include <vector>
#include <limits.h>
#include <stack>
#include <stdlib.h>

#include "Renju.hpp"

const int Renju::direct_list[4][2] = {
    {1, 0},
    {0, 1},
    {1, 1},
    {1, -1},
};
const int Renju::sign_list[2] = {-1, 1};

Renju::Renju(int size, bool forbid) : has_forbid_(forbid), size_(size),
black_count_(0), white_count_(0)
{
    data_.assign(size*size, Pos::kEmpty);
}

Renju::~Renju()
{
}

void Renju::SetPos(int x, int y, Pos role)
{
    auto&pos = Get(x, y);
    switch (pos)
    {
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
    switch (role)
    {
    case Pos::kBlack:
        black_count_++;
        break;
    case Pos::kWhite:
        white_count_++;
        break;
    default:
        break;
    }
}

void Renju::Init()
{
    srand(time(NULL));
}

std::pair<int, int> Renju::GetNext(Role role, int deep)
{
    if (role == Role::kBlack)
    {
        if (black_count_ == 0)
        {
            //黑子第一手
            return std::make_pair(size_ / 2, size_ / 2);
        }
        if (black_count_ == 1)
        {
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
    else
    {
        if (white_count_ == 0)
        {
            //白子第一手下旁边随便一个
            auto &random = direct_list[rand() % 4];
            return std::make_pair(size_ / 2 + random[0], size_ / 2 + random[1]);
        }
    }
    int max = INT_MIN;
    auto res = GetNextImpl(role, deep);
    return std::make_pair(std::get<0>(res), std::get<1>(res));
}

//x y value

std::tuple<int, int, int> Renju::GetNextImpl(Role role, int deep)
{
    int max = INT_MIN;
    int resx = 0;
    int resy = 0;
    for (int x = 0; x < size_; ++x)
    {
        for (int y = 0; y < size_; ++y)
        {
            if (Get(x, y) == Pos::kEmpty && HasNear(x, y))
            {
                SetPos(x, y, GetByRole(role));
                bool win;
                ComputePosValue(x, y, &win);
                if (deep > 0 && !win)
                {
                    auto res = GetNextImpl(GetOpponent(role), deep - 1);
                    int v = -std::get<2>(res);
                    if (v > max)
                    {
                        max = v;
                        resx = x;
                        resy = y;
                    }
#if USE_RANDOM
                    if (v == max && rand() > RAND_MAX / 2)
                    {

                        max = v;
                        resx = x;
                        resy = y;
                    }
#endif
                }
                else
                {
                    int v = ComputeValue(role) - ComputeValue(GetOpponent(role));
                    if (v > max)
                    {
                        max = v;
                        resx = x;
                        resy = y;
                    }
#if USE_RANDOM
                    if (v == max && rand() > RAND_MAX / 2)
                    {

                        max = v;
                        resx = x;
                        resy = y;
                    }
#endif
                }
                SetPos(x, y, Pos::kEmpty);
            }
        }
    }
    return std::make_tuple(resx, resy, max);
}

bool Renju::IsValidPoint(int x, int y)
{
    return x >= 0 && x < size_ && y >= 0 && y< size_;
}

int Renju::ComputeValue(Role role)
{
    int value = 0;
    for (int x = 0; x < size_; ++x)
        for (int y = 0; y < size_; ++y)
        {
            auto pos = Get(x, y);
            if (pos == Pos::kEmpty)continue;
            if (IsSame(role, pos))
            {
                value += ComputePosValue(x, y);
            }
        }
    return value;
}

int Renju::ComputePosValue(int x, int y, bool*win)
{
    uint8_t res[Type::kMax] = {0};
    bool has_long = false;
    auto raw = Get(x, y);
    for (auto &direct : direct_list)
    {
        int count = 1;
        int block = 0;
        for (auto sign : sign_list)
        {
            int i = x;
            int j = y;
            int k;
            for (k = 1;; k++)
            {
                i += sign * direct[0];
                j += sign * direct[1];
                if (IsValidPoint(i, j))
                {
                    auto p = Get(i, j);
                    if (raw == p)
                    {
                    }
                    else if (p == Pos::kEmpty)
                    {
                        break;
                    }
                    else
                    {
                        block += 1;
                        break;
                    }
                }
                else
                {
                    block += 1;
                    break;
                }
            }
            count += k - 1;
        }
        if (count >= 5)
        {
            res[Type::k5]++;
            if (count > 5)has_long = true;
        }
        else if (count == 4)
        {
            switch (block)
            {
            case 0:res[Type::kFlex4]++;
                break;
            case 1:res[Type::kBlock4]++;
                break;
            default:res[Type::kDefault]++;
                break;
            }
        }
        else if (count == 3)
        {
            switch (block)
            {
            case 0:res[Type::kFlex3]++;
                break;
            case 1:res[Type::kBlock3]++;
                break;
            default:res[Type::kDefault]++;
                break;
            }
        }
        else if (count == 2)
        {
            switch (block)
            {
            case 0:res[Type::kFlex2]++;
                break;
            case 1:res[Type::kBlock2]++;
                break;
            default:res[Type::kDefault]++;
                break;
            }
        }
        else res[Type::kDefault]++;
    }
    if (win)*win = res[Type::k5] > 0;
    int value = TypeValue::kValue5 * res[Type::k5]
        + TypeValue::kValueFlex4 * res[Type::kFlex4]
        + TypeValue::kValueBlock4 * res[Type::kBlock4]
        + TypeValue::kValueFlex3 * res[Type::kFlex3]
        + TypeValue::kValueBlock3 * res[Type::kBlock3]
        + TypeValue::kValueFlex2 * res[Type::kFlex2]
        + TypeValue::kValueBlock2 * res[Type::kBlock2]
        + TypeValue::kValueDefault * res[Type::kDefault];
    if (raw == Pos::kBlack && has_forbid_)
    {
        if ((res[Type::k5] == 0 && (res[Type::kFlex4] > 1 || res[Type::kFlex3] > 1))
            || has_long)
            value += kValueForbid;
    }
    return value;
}

Renju::Pos &Renju::Get(int x, int y)
{
    return data_[x * size_ + y];
}

bool Renju::IsSame(Role role, Pos pos)
{
    return (pos == Pos::kBlack && role == Role::kBlack) ||
        (pos == Pos::kWhite && role == Role::kWhite);
}

Renju::Pos Renju::GetByRole(Role role)
{
    if (role == Role::kBlack)return Pos::kBlack;
    return Pos::kWhite;
}

bool Renju::HasNear(int x, int y, const int distance)
{
    for (int i = x - distance; i <= x + distance; ++i)
        for (int j = y - distance; j <= y + distance; ++j)
            if ((i != x || j != y) &&
                IsValidPoint(i, j) && Get(i, j) != Pos::kEmpty)
                return true;
    return false;
}

Renju::Role Renju::GetOpponent(Role role)
{
    return role == Role::kBlack ? Role::kWhite : Role::kBlack;
}