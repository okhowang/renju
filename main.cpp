#define RAPIDJSON_HAS_STDSTRING 1
#define RAPIDJSON_PARSE_DEFAULT_FLAGS (kParseTrailingCommasFlag|kParseStopWhenDoneFlag)
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>
#include <rapidjson/getter.h>

#include "Renju.hpp"

rapidjson::Pointer pointer_type("/head/type");
rapidjson::Pointer pointer_white_name("/body/player_white/name");
rapidjson::Pointer pointer_black_name("/body/player_black/name");
rapidjson::Pointer pointer_steps("/body/steps");
rapidjson::Pointer pointer_size("/body/size");
rapidjson::Pointer pointer_forbin("/body/has_hand_cut");
rapidjson::Pointer pointer_side("/side");
rapidjson::Pointer pointer_x("/x");
rapidjson::Pointer pointer_y("/y");
rapidjson::Pointer pointer_result("/head/result");
rapidjson::Pointer pointer_msg("/head/msg");

void SetName(rapidjson::Document &json)
{
    auto white = pointer_white_name.Get(json);
    if (white != nullptr)
    {
        white->SetString("okhowang_white");
        return;
    }

    auto black = pointer_black_name.Get(json);
    if (black != nullptr)
    {
        black->SetString("okhowang_black");
        return;
    }
    throw "bad get name";
}

void Process(rapidjson::Document &json)
{
    int size = rapidjson::GetIntByPointer(json, pointer_size);
    int forbid = rapidjson::GetIntByPointer(json, pointer_forbin);
    rapidjson::Value v;
    auto steps = pointer_steps.Get(json);
    if (steps == nullptr || !steps->IsArray())
    {
        v.SetArray();
        steps = &v;
    }
    Renju renju(size, forbid);
    Renju::Role cur_role = Renju::Role::kWhite;
    for (auto &step : steps->GetArray())
    {
        int x = rapidjson::GetIntByPointer(step, pointer_x) - 1;
        int y = rapidjson::GetIntByPointer(step, pointer_y) - 1;
        auto v_side = pointer_side.Get(step);
        if (v_side == nullptr || !v_side->IsString() || v_side->GetStringLength() == 0)
        {
            throw "bad side";
        }
        if (v_side->GetString()[0] == 'w')
        {
            renju.SetPos(x, y, Renju::Pos::kWhite);
            cur_role = Renju::Role::kWhite;
        }
        else
        {

            renju.SetPos(x, y, Renju::Pos::kBlack);
            cur_role = Renju::Role::kBlack;
        }
    }
    renju.Init();
    if (cur_role == Renju::Role::kWhite)
        cur_role = Renju::Role::kBlack;
    else cur_role = Renju::Role::kWhite;

    auto pos = renju.GetNext(cur_role);
    rapidjson::Value step;
    step.SetObject();
    step.AddMember("side", cur_role == Renju::Role::kBlack ? "b" : "w", json.GetAllocator());
    step.AddMember("x", std::to_string(pos.first + 1), json.GetAllocator());
    step.AddMember("y", std::to_string(pos.second + 1), json.GetAllocator());
    time_t t;
    time(&t);
    tm tm;
    localtime_r(&t, &tm);
    char timebuf[128];
    strftime(timebuf, sizeof (timebuf), "%Y%m%d%H%M%S", &tm);
    step.AddMember("time", rapidjson::Value(timebuf, json.GetAllocator()).Move(), json.GetAllocator());
    steps->PushBack(step.Move(), json.GetAllocator());
}

int main(int argc, char** argv)
{
    printf("Content-Type: text/json\r\n\r\n");
    char buffer[65535];
    rapidjson::Document json;
#if 0
    rapidjson::FileReadStream input(stdin, buffer, sizeof (buffer));
    json.ParseStream(input);
#else
    json.Parse(R"(

{"head":{"type":1},"body":{"steps":[{"side":"b","x":"8","y":"8","time":"20161027163928"},{"side":"w","x":"9","y":"7","time":"20161027163929"},{"side":"b","x":"7","y":"7","time":"20161027163929"},{"side":"w","x":"9","y":"9","time":"20161027163929"},{"side":"b","x":"9","y":"8","time":"20161027163929"},{"side":"w","x":"10","y":"8","time":"20161027163929"},{"side":"b","x":"8","y":"6","time":"20161027163930"},{"side":"w","x":"8","y":"10","time":"20161027163930"},{"side":"b","x":"8","y":"7","time":"20161027163930"},{"side":"w","x":"7","y":"11","time":"20161027163931"},{"side":"b","x":"7","y":"8","time":"20161027163931"},{"side":"w","x":"5","y":"13","time":"20161027163931"}],"size":15,"has_hand_cut":1}}"


)");
#endif
    if (json.HasParseError())
    {
        pointer_result.Set(json, 1);
        pointer_msg.Set(json, "json parse error");
        return 0;
    }
    try
    {
        int type = rapidjson::GetIntByPointer(json, pointer_type);
        switch (type)
        {
        case 0:
            SetName(json);
            break;
        case 1:
            Process(json);
            break;
        }
    }
    catch (const rapidjson::ValueError &e)
    {
        pointer_result.Set(json, 1);
        pointer_msg.Set(json, e.what());
    }
    catch (const char *e)
    {
        pointer_result.Set(json, 1);
        pointer_msg.Set(json, e);
    }
    rapidjson::FileWriteStream output(stdout, buffer, sizeof (buffer));
    rapidjson::Writer<rapidjson::FileWriteStream> writer(output);
    json.Accept(writer);
    return 0;
}

