#pragma once

#include "judge.h"
#include "trail.h"
#include "../utility/json11.hpp"
#include "base/co.h"
#include "base/rpc.h"
#include "base/fastring.h"
#include "base/thread.h"

#include "hiaiengine/engine.h"
#include "Common.h"
#include "stream_data.h"
#include "AppCommon.h"

#include <string>
#include <queue>
#include <vector>
#include <memory>

#define DST_INPUT_SIZE 1
#define DST_OUTPUT_SIZE 1

class DstEngine : public hiai::Engine {
public:
    HIAI_StatusT Init(const hiai::AIConfig& config, const std::vector<hiai::AIModelDescription>& model_desc);

    HIAI_DEFINE_PROCESS(DST_INPUT_SIZE, DST_OUTPUT_SIZE)

    virtual ~DstEngine();

private:
    HIAI_StatusT ProcessResult(const std::shared_ptr<DeviceStreamData>& inputArg);
    HIAI_StatusT SaveJpg(const std::shared_ptr<DeviceStreamData>& inputArg, const std::string& path);
    HIAI_StatusT SaveVideoToJpg(std::queue<std::shared_ptr<DeviceStreamData>>& result_videos, const std::string& RESULT_VIDEO_0, const std::string& videoname);

    // return alarm jpg, base64 encoded
    fastring GetJpg(const std::shared_ptr<DeviceStreamData>& inputArg);
    void report_alarm(Json&& alarm);
    void make_and_report_alarm(const std::shared_ptr<DeviceStreamData>& inputArg, const std::string& ipc_ip, const fastring& alarm_type);

    void report_alarm_fun();

    bool getIp(std::string ip);
   
private:
    // process the classify result
    int video_id = 1;
    uint32_t frame_id;
    std::vector<Trail> trails;

    // result_condition主要是为了保存越界或者是入侵前后时间标记用的，没有越界或是入侵为0 ，有了为为1 ，当1在中中间位置时保存视频
    std::vector<int> result_condition;
    std::queue<std::shared_ptr<DeviceStreamData>> result_videos;
    queue<std::shared_ptr<DeviceStreamData>> result_videos_1;
    queue<std::shared_ptr<DeviceStreamData>> result_videos_2;

    json11::Json borders_;
    std::string cover_path_;
    uint32_t alarm_flag_ = 0;

    std::unique_ptr<rpc::Server> rpc_serv_;
    std::vector<Json> alarms_;  // not json11::Json
    Mutex alarm_mtx_;
    co::Event alarm_ev_;
    bool stop_;
    std::string ip_;
    std::string client_ip_;
    std::string border_filename_;
};
