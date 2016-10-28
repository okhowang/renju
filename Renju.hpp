#ifndef RENJU_HPP
#define RENJU_HPP

#include <cstddef>
#include <climits>
#include <utility>
#include <memory>
#include <vector>

class Renju {
public:

    enum Type {
        k5 = 0,
        kFlex4 = 1,
        kBlock4 = 2,
        kFlex3 = 3,
        kBlock3 = 4,
        kFlex2 = 5,
        kBlock2 = 6,
        kLong = 7,
        kDefault = 8,
        kMax = 9,
    };

    enum TypeValue {
        kValue5 = 100000,
        kValueFlex4 = 10000,
        kValueBlock4 = 1000,
        kValueFlex3 = 1000,
        kValueBlock3 = 100,
        kValueFlex2 = 100,
        kValueBlock2 = 10,
        kValueDefault = 1,
        kValueForbid = -100000,
    };

    enum class Role {
        kBlack = 1,
        kWhite = 2,
    };

    enum class Pos {
        kEmpty = 0,
        kBlack = 1,
        kWhite = 2,
        kOutside = 3,
    };
    enum Key {
        kEmpty = 0,
        kSelf = 1,
        kOppnonent = 2
    };

    Renju(int size, bool forbid);

    virtual ~Renju();

    //初始化棋盘
    void SetPos(int x, int y, Pos role);

    //setpos完毕后调用
    void Init();

    //计算下一步
    std::pair<int, int> GetNext(Role role, int deep = 4);

private:
    std::tuple<int, int, int> GetNextImpl(Role role, int deep, int alpha = INT_MIN + 1, int beta = INT_MAX);

    std::vector<Pos> data_;
    bool has_forbid_;
    int size_;
    int black_count_;
    int white_count_;

    bool IsValidPoint(int x, int y);

    Pos &Get(int x, int y);

    Pos GetByRole(Role role);

    static bool IsSame(Role role, Pos pos);

    std::vector<std::pair<int, int> > GenMoveList();

    bool HasNear(int x, int y, const int distance = 2);

    Role GetOpponent(Role role);

    typedef struct {
        Type typeinfo[2][4]; //双方视角, 4个方向 
    } PosType; //某一点的棋型信息

    std::vector<PosType> pos_types;

    PosType &GetPosType(int x, int y) {
        return pos_types[x * size_ + y];
    }

    static Renju::Type TypeLine(Role role, Pos line[]);

    Renju::Type TypeLine(Role role, int x, int y, int i, int j); //获取点(x,y)某一方向上的棋型信息
    void UpdatePosTypes(int x, int y); //更新点(x,y)附近的棋型信息
    void SumupTypeinfos(Role role, int x, int y, int res[Type::kMax]);

    int Score();

    static const int direct_list[4][2];
    static const int sign_list[2];
    //pattern 缓存判断

    static std::pair<bool, Type> patternCache[1024 * 1024];

    uint32_t GetKey(int x, int y, int direct);

    static Type GetKeyType(uint32_t key);


    //保存棋盘上所有子的(x,y)范围
    typedef struct {
        int frame_x_min;
        int frame_x_max;
        int frame_y_min;
        int frame_y_max;
    } Frame;
    Frame frame;
    Frame bak_frame;

    void UpdateFrame(int x, int y);


    //判斷一個子是否勝利 或 禁手 剪枝用
    int GetPosResult(int x, int y);

};

#endif /* RENJU_HPP */

