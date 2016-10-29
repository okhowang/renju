#ifndef RENJU_HPP
#define RENJU_HPP

#include <cstddef>
#include <climits>

#include <utility>
#include <memory>
#include <vector>
#include <chrono>

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
        kValue5 = 2000,
        kValueFlex4 = 1000,
        kValueBlock4 = 200,
        kValueFlex3 = 120,
        kValueBlock3 = 30,
        kValueFlex2 = 20,
        kValueBlock2 = 4,
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

    typedef struct  _MOVE{
        _MOVE() { x = 0; y = 0; }
        _MOVE(int tx, int ty) : x(tx), y(ty) {}
        int x;
        int y;
    } MOVE;

    MOVE  best_move;
    int   best_val;
    int   total_cnt;  //搜索总次数
    int   leaf_cnt;   //叶子节点个数
    
    MOVE  last_move;  //上一次的走法
    MOVE  last_self_move;  //上一次自己的走法

    Renju(int size, bool forbid);

    virtual ~Renju();

    //初始化棋盘
    void SetPos(int x, int y, Pos role, bool update = true);

    //setpos完毕后调用
    void Init();

    //计算下一步
    std::pair<int, int> GetNext(Role role, int deep = 2);

    //新接口：计算下一步
    std::pair<int, int> Solve(Role role, int deep = 2);

    int  MinMaxSearch(Role role, int cur_depth, int alpha, int beta);
    int  AlphaBetaSearch(Role role, int cur_depth, int alpha, int beta);

    //Debug Functions
    void    DumpBoard(FILE* fp);
    void    DumpAllPosTypes();

    typedef struct {
        Type typeinfo[2][4]; //双方视角, 4个方向 
    } PosType; //某一点的棋型信息

    int  max_depth;
    int  max_iter_depth;

private:
    std::tuple<int, int, int> GetNextImpl(Role role, int deep, int check_deep, int alpha = INT_MIN + 1,
                                          int beta = INT_MAX,
                                          std::pair<int, int> *suggest = nullptr);

    std::vector<Pos> data_;
    bool has_forbid_;
    int size_;
    int black_count_;
    int white_count_;

    bool IsValidPoint(int x, int y);

    Pos &Get(int x, int y);

    Pos GetByRole(Role role);

    static inline bool IsSame(Role role, Pos pos) {
        return (pos == Pos::kBlack && role == Role::kBlack) ||
               (pos == Pos::kWhite && role == Role::kWhite);
    }

    std::vector<std::tuple<int, int, int> > GenMoveList(Pos pos);

    bool HasNear(int x, int y);

    Role GetOpponent(Role role);

    

    std::vector<PosType> pos_types;

    PosType &GetPosType(int x, int y) {
        return pos_types[x * size_ + y];
    }

    static Renju::Type TypeLine(Role role, Pos line[]);

    Renju::Type TypeLine(Role role, int x, int y, int d); //获取点(x,y)某一方向上的棋型信息
    void UpdatePosTypes(int x, int y); //更新点(x,y)附近的棋型信息
    void SumupTypeinfos(Role role, int x, int y, int res[Type::kMax]);

    int Score(Role role);

    static const int direct_list[4][2];
    static const int sign_list[2];
    //pattern 缓存判断

    static std::pair<bool, Type> patternCache[1024 * 1024];

    uint32_t GetKey(Role role, int x, int y, int direct);

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

    std::string Debug();

    std::chrono::time_point<std::chrono::system_clock> start_;

    bool Timeout() {
//        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().- start_).count() > 9000;
        return true;
    }


    std::vector<uint8_t> has_near_;
};

#endif /* RENJU_HPP */

