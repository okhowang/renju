#ifndef RENJU_HPP
#define RENJU_HPP
#include <cstddef>
#include <utility>
#include <memory>
#include <vector>

class Renju
{
public:

    enum Type
    {
        k5 = 0,
        kFlex4 = 1,
        kBlock4 = 2,
        kFlex3 = 3,
        kBlock3 = 4,
        kFlex2 = 5,
        kBlock2 = 6,
        kDefault = 7,
        kMax = 8,
    };

    enum TypeValue
    {
        kValue5 = 100000,
        kValueFlex4 = 100,
        kValueBlock4 = 70,
        kValueFlex3 = 100,
        kValueBlock3 = 30,
        kValueFlex2 = 20,
        kValueBlock2 = 2,
        kValueDefault = 1,
        kValueForbid = -100000,
    };

    enum class Role
    {
        kBlack,
        kWhite,
    };

    enum class Pos
    {
        kEmpty,
        kBlack,
        kWhite,
    };
    Renju(int size, bool forbid);
    virtual ~Renju();

    //初始化棋盘
    void SetPos(int x, int y, Pos role, std::vector<Pos> *data = nullptr);
    //setpos完毕后调用
    void Init();

    //计算下一步
    std::pair<int, int> GetNext(Role role, int deep = 4);
private:
    std::tuple<int, int, int> GetNextImplMT(Role role, int deep);
    std::tuple<int, int, int> GetNextImpl(Role role, int deep, std::vector<Pos> *data = nullptr);
    std::vector<Pos> data_;
    bool has_forbid_;
    int size_;
    int black_count_;
    int white_count_;

    bool IsValidPoint(int x, int y);
    int ComputeValue(Role role, std::vector<Pos> *data);
    int ComputePosValue(int x, int y, std::vector<Pos> *data, bool *win = nullptr);

    Pos &Get(int x, int y, std::vector<Pos> *data = nullptr);
    Pos GetByRole(Role role);
    bool IsSame(Role role, Pos pos);
    bool HasNear(int x, int y, std::vector<Pos> *data, const int distance = 2);
    Role GetOpponent(Role role);

    static const int direct_list[4][2];
    static const int sign_list[2];

};

#endif /* RENJU_HPP */

