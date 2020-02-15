#include "DstEngine.h"
#include "../utility/bordersloader.h"
//#include "de_serv_impl.h"
#include "base/flag.h"
#include "base/log.h"
#include "base/fs.h"
#include "base/hash.h"
#include "base/time.h"
#include "thread"
#include <cmath>

// DEF_string(de_serv_ip, "127.0.0.1", "DstEngine tcp server ip");
// DEF_uint32(de_serv_port, 9910, "DstEngine tcp server port");
// DEF_string(de_serv_passwd, "", "passwd for DstEngine tcp server");

DEF_string(ae_serv_ip, "127.0.0.1", "ae alarm server ip");
DEF_uint32(ae_serv_port, 9900, "ae alarm server port");
DEF_string(ae_serv_passwd, "", "passwd for ae alarm server");

#define PERSON_LABEL    1
#define SAMPLE_P        6
// FRAME_RATE为视频帧率，SAVE_TIME为需要保存的视频时长
#define FRAME_RATE      2
#define SAMPLE_N        (FRAME_RATE/2)
#define SAVE_TIME       5
#define CAMERA_IP       "192.168.1.65"
#define YUEJIE          1
#define RUQIN           2

uint32_t OUT_PATH_MAX = 128;
using Stat = struct stat;
// 报错图片路径
const std::string RESULT_FOLDER = "result_files/";
const std::string FILE_PRE_FIX = "result_";
// 报错视频路径
 const std::string RESULT_VIDEO = "result_videos/";
const std::string RESULT_VIDEO_1 = "result_videos_ruqin/";
const std::string RESULT_VIDEO_2 = "result_videos_yuejie/";
const int MAX_CHAR_LENGTH = 256;

std::string videoname_1;
std::string videoname_2;
char* gettime()
{
    time_t rawtime;
    struct tm *ptminfo;
    time(&rawtime);
    ptminfo = localtime(&rawtime);
    char *time_str = new char[19];
    sprintf(time_str,"%02d-%02d-%02d_%02d:%02d:%02d",
           ptminfo->tm_year + 1900, ptminfo->tm_mon + 1, ptminfo->tm_mday,
           ptminfo->tm_hour, ptminfo->tm_min, ptminfo->tm_sec);
    return time_str;
}

DstEngine::~DstEngine() {
    atomic_swap(&stop_, true);
    alarm_ev_.signal();
    co::stop();
}

HIAI_StatusT DstEngine::Init(const hiai::AIConfig& config, const std::vector<hiai::AIModelDescription>& model_desc) 
{
    LOG << "DstEngine Init" ;
    printf("DstEngine Init\n");
    stop_ = false;
    frame_id = 0;
    auto aimap = kvmap(config);
    cover_path_ = "/home/cover.jpg";

    if(aimap.count("camera_ip")){
        ip_ = aimap["camera_ip"];
        cover_path_ = "/home/" + aimap["camera_ip"] + ".jpg";
        border_filename_ = ip_ + ".json";
        std::cout << "border filename is " << border_filename_ << std::endl;
    }

    if(aimap.count("client_ip")){
        client_ip_ = aimap["client_ip"];
        std::cout << "123client ip is : "<< client_ip_ << std::endl;
    }

    // 启动 警报上报协程
    go(&DstEngine::report_alarm_fun, this);

    return HIAI_OK;
}

HIAI_StatusT DstEngine::SaveJpg(const std::shared_ptr<DeviceStreamData>& inputArg, const std::string& path)
{
    if (!fs::exists(RESULT_FOLDER)) fs::mkdir(RESULT_FOLDER, true);
    std::string filename = RESULT_FOLDER + FILE_PRE_FIX + path + ".jpg";

    void *image_ptr = (void *)(inputArg->imgOrigin.buf.data.get());
    for (const auto& det : inputArg->detectResult) {
        if(det.classId == PERSON_LABEL)  {
            FILE *fp = fopen(filename.c_str(), "wb");
            if (NULL == fp) {
                ELOG << "[SaveFile] Save file engine: open file fail!";
                return HIAI_ERROR;
            }
            
            fwrite(image_ptr, 1, inputArg->imgOrigin.buf.len_of_byte, fp);
            fflush(fp);
            fclose(fp);
            LOG << "SaveJpg ok";
            return HIAI_OK;
        }
    }

    ELOG << "DstEngine::SaveJpg error, PERSON_LABEL not match..";
    return HIAI_ERROR;
}

fastring DstEngine::GetJpg(const std::shared_ptr<DeviceStreamData>& inputArg) {
    void *image_ptr = (void *)(inputArg->imgOrigin.buf.data.get());

    for (const auto& det : inputArg->detectResult) {
        if(det.classId == PERSON_LABEL)  {
            return base64_encode(image_ptr, inputArg->imgOrigin.buf.len_of_byte);
        }
    }
    return fastring();
}

void DstEngine::report_alarm(Json&& alarm) {
    MutexGuard g(alarm_mtx_);
    alarms_.push_back(std::move(alarm));
    alarm_ev_.signal();
}

void DstEngine::make_and_report_alarm(const std::shared_ptr<DeviceStreamData>& inputArg, const std::string& ipc_ip, const fastring& alarm_type) {
    LOG << "make and report alarm";
    // this->SaveJpg(inputArg,std::to_string(now::ms()));
    thread th1(&DstEngine::SaveJpg, this,inputArg,std::to_string(now::ms()));
    th1.detach();
    fastring jpg = this->GetJpg(inputArg);
    if(jpg.empty()){
        ELOG << "[DstEngine]  get jpg file failed";
        return;
    }

    Json alarm;
	alarm.add_member("req_id", now::ms());
    alarm.add_member("method", "report_alarm");
    alarm.add_member("ipc_ip", ipc_ip.c_str());
    alarm.add_member("img", jpg);
    alarm.add_member("time", now::str());
    alarm.add_member("type", alarm_type); // 越界"border", 入侵"region"    
    this->report_alarm(std::move(alarm));
}

HIAI_StatusT DstEngine::SaveVideoToJpg(std::queue<std::shared_ptr<DeviceStreamData>>& result_videos, const std::string& RESULT_VIDEO_0, const std::string& videoname)
{
    std::cout <<"RESULT_VIDEO_0:" <<RESULT_VIDEO_0 << std::endl;
    // 把图片保存下来 
    if (!fs::exists(RESULT_VIDEO_0)) fs::mkdir(RESULT_VIDEO_0, true);
    for (int i = 0; i < FRAME_RATE*SAVE_TIME*2; i++)
    {
        char j[3];
        sprintf(j, "%03d", i+1);
        std::string videoname_0 = RESULT_VIDEO_0 + j + ".jpg";
        const std::shared_ptr<DeviceStreamData>& video_1 = result_videos.front();
        void *image_ptr = (void *)(video_1->imgOrigin.buf.data.get());
        FILE *fp = fopen(videoname_0.c_str(), "wb");
        if (NULL == fp) {
            HIAI_ENGINE_LOG(HIAI_IDE_INFO, "[SaveFile] Save video_file engine: open file fail!");
            return HIAI_ERROR;
        }    
        fwrite(image_ptr, 1, video_1->imgOrigin.buf.len_of_byte, fp);
        fflush(fp);
        fclose(fp);

        // 此处是把第一个元素放到最后
        const std::shared_ptr<DeviceStreamData>& tempvideo = result_videos.front();
        result_videos.pop();
        result_videos.push(tempvideo);

    }
    // const char* result = RESULT_VIDEO_0.c_str();
    
    string sh = std::string("./saveVideo.sh ") + videoname + "&";
    system(sh.c_str());
    return HIAI_OK;
}

bool DstEngine::getIp(std::string ip)
{
    fs::file f("network2.json",'r');
    char buf[512];
    if(f){
        printf("read file network.json\n");
        f.read(buf,256);
        printf("%s\n",buf);
    }
    fastring s(buf);

    Json y = json::parse(s.data(),s.size());
    std::cout << y.pretty() << std::endl;
    Json ip_ = y.find("client_ip");
    if(ip_.is_string()){
        ip = ip_.get_string();
        f.close();
        return true;
    }
    else{
        f.close();
        return false;
    }
}


HIAI_StatusT DstEngine::ProcessResult(const std::shared_ptr<DeviceStreamData>& inputArg)
{
    LOG << "[DstEngine]: The process result of frame " << frame_id;

    if(frame_id == 0){
        void *image_ptr = (void *)(inputArg->imgOrigin.buf.data.get());
        FILE *fp = fopen(cover_path_.c_str(), "wb");
        if (NULL == fp) {
            ELOG << "[SaveFile] Save file engine: open file fail: " << cover_path_;
            return HIAI_ERROR;
        }      
        fwrite(image_ptr, 1, inputArg->imgOrigin.buf.len_of_byte, fp);
        fflush(fp);
        fclose(fp);
    }

    result_videos.push(inputArg);
    result_condition.push_back(0);
    if (result_videos.size() > FRAME_RATE * SAVE_TIME * 2)
    {
        result_videos.pop();
        std::vector<int>::iterator k = result_condition.begin();
	    result_condition.erase(k);
    }

    Point foot, a, b;
    std::vector<Trail>::iterator it;
    auto borders = BordersLoader::Instance().load(border_filename_);
    int i = 1;
    float sit_x, sit_y;
    int result_center = 0;
    // 创建保存视频的路径
    if (!fs::exists(RESULT_VIDEO)) fs::mkdir(RESULT_VIDEO, true);

    LOG << "detectResult.size() =================== " << inputArg->detectResult.size();
    LOG << "Size of vector<Trail> trails: " << trails.size();

    //清除超时的轨迹,并对count++, 开启每条轨迹的insertable
    for(it=trails.begin(); it!=trails.end();){
        if(it->check_states()){
            it->update_states_common();
            LOG << "the length of Trail.points_: " << it->points().size() << ", count = " << it->count;
            it++;
        }else{
            it = trails.erase(it);
        }
    }

    //判断入侵报警并对检测框进行轨迹分配 xwh
    if (inputArg->detectResult.size() != 0)
    {   
        //=====================================  多人检测 xwh ========================================
        for (const auto& det : inputArg->detectResult) {
            if(det.classId != PERSON_LABEL) continue;
            // 识别对象编号，即识别到了几个人
            LOG << "detect object " << i++;
            // 识别对象，det.classId（id=1）表示人，det.confidence（score）表示置信度
            LOG << "object id: " << det.classId << " score: " << det.confidence;
            //输出人的坐标，矩形框的两个点
            LOG << "the object regin: "
                << " x1: " << det.location.anchor_lt.x
                << " y1: " << det.location.anchor_lt.y
                << " x2: " << det.location.anchor_rb.x
                << " y2: " << det.location.anchor_rb.y;

            foot.x = round(det.location.anchor_lt.x + det.location.anchor_rb.x) / 2;
            foot.y = det.location.anchor_rb.y;

            //检测当前detection的"入侵"报警
            for(const auto& boder: borders){    
                int graph = boder.graph();
                switch(graph){
                    case RECTANGLE:
                        LOG << "RECTANGLE Detection";
                        if(judge_out_rectangle(foot, boder.points())){
                            alarm_flag_++;
                            if(alarm_flag_ % SAMPLE_P != 1) break;
                        //    std::string time = now::str().c_str();
                            videoname_1 = std::to_string(video_id);
                            video_id ++;
                            result_condition.pop_back();
                            result_condition.push_back(1);

                            // TODO:
                            //   1. ipc ip manager
                            //   2. 报警类型, 入侵("region"), 越界("border")
                            this->make_and_report_alarm(inputArg, ip_, "region");
                            WLOG << "cross the border, type: RECTANGLE";
                        }
                        break;
                    case POLYGON:
                        LOG << "POLYGON Detection";
                        if(judge_out_polygon(foot, boder.points())){
                            alarm_flag_++;
                            if(alarm_flag_ % SAMPLE_P != 1) break;
                            
                            videoname_1 = std::to_string(video_id);
                            video_id ++;
                            result_condition.pop_back();
                            result_condition.push_back(1);

                            this->make_and_report_alarm(inputArg, ip_, "region");
                            WLOG << "cross the border, type: POLYGON";
                        }
                        break;
                    default:
                        break;
                }
            }

            //得到 x, y 两点坐标 xwh
            a.x = det.location.anchor_lt.x;
            a.y = det.location.anchor_lt.y;
            b.x = det.location.anchor_rb.x;
            b.y = det.location.anchor_rb.y;

            //添加检测框到合适的轨迹中 xwh
            if(trails.empty()){
                trails.push_back( Trail(a, b));
            }else{
                Trail* target=NULL;
                float res_max = THETA-1.0, res = 0.0;
                for(it=trails.begin(); it!=trails.end();it++){
                    if(it->get_insertable()){
                        res = it->cal_dis(a,b);
                        // LOG << "res ======= " << res << "  res_max =======" << res_max;
                        if(res > res_max){
                            res_max = res;
                            target = &(*it);
                        }  
                    }
                }
                if(res_max > THRESOD && target){  //距离大于阈值,且有轨迹是打开状态
                    target->insert(a, b);
                }else if(!target){
                    trails.push_back(Trail(a, b));  //没有打开状态的轨迹,则新建轨迹

                }else{   //权重距离小于阈值
                    if(min(min(1920-foot.x, foot.x), min(1080-foot.y, foot.y)) < 100){  //框距离边界很近则认为是新出现的对象
                        trails.push_back(Trail(a, b));
                    }else{   //框距离边界很远则认为是已有对象,强制分配检测框到"距离"最近的轨迹
                        target->insert(a, b);
                    }
                }
            }
        }
    }
    // //保存视频
    if (result_condition.size() == FRAME_RATE * SAVE_TIME * 2)
    {
        result_center = result_condition[FRAME_RATE * SAVE_TIME];
        // printf("============result_center========%d\n",result_center);
    }
    
    if(result_center == 1)
    {
        result_videos_1 = result_videos;
        if(SaveVideoToJpg(result_videos_1, RESULT_VIDEO_1, videoname_1) != HIAI_OK)
        {
            HIAI_ENGINE_LOG(HIAI_IDE_ERROR, "[DstEngine]  save video file failed");
            return HIAI_ERROR;
        }
        
        // video_id ++;
            
    }
    result_videos_1 = queue<std::shared_ptr<DeviceStreamData>>();
    // 对"越界"情况进行检测  xwh 
    if(!trails.empty() && frame_id%SAMPLE_N == 0){
        for(it=trails.begin(); it!=trails.end();it++){
            for(const auto& boder: borders){    
                int graph = boder.graph();
                switch(graph){
                    case LINE:
                        LOG << "Line detection";
                        if(judge_out_line(it->points(), boder.points())){
                       //     std::string time = std::to_string(getCurentTime());
                        //    videoname_2 = std::to_string(LINE) + "_" + std::to_string(video_id);
                            videoname_2 = std::to_string(video_id); 
                            video_id ++;

                            printf("videonwm==============%s\n",videoname_2.c_str());
                            result_condition.pop_back();
                            result_condition.push_back(2);

                            this->make_and_report_alarm(inputArg, ip_, "border");
                            WLOG << "cross the border, type: LINE";
                            it->clear_();
                        }
                        break;
                    case BROKEN_LINE:
                        LOG << "BROKEN_LINE detection";
                        if(judge_out_broken_line(it->points(), boder.points())){
                        //    std::string time = std::to_string(getCurentTime());
                            videoname_2 = std::to_string(video_id);
                            video_id ++;
                            result_condition.pop_back();
                            result_condition.push_back(2);

                            this->make_and_report_alarm(inputArg, ip_, "border");
                            WLOG << "cross the border, type: BROKEN_LINE";
                            it->clear_();
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }
    // //保存视频到图片
    if (result_condition.size() == FRAME_RATE * SAVE_TIME * 2)
    {
        result_center = result_condition[FRAME_RATE * SAVE_TIME];
        // printf("============result_center========%d\n",result_center);
    }
    
    if(result_center == 2)
    {
        result_videos_2 = result_videos;
        if(SaveVideoToJpg(result_videos_2, RESULT_VIDEO_2, videoname_2) != HIAI_OK)
        {
            HIAI_ENGINE_LOG(HIAI_IDE_ERROR, "[DstEngine]  save video file failed");
            return HIAI_ERROR;
        }
        
        // video_id ++;
    }
    result_videos_2 = queue<std::shared_ptr<DeviceStreamData>>();
    
    frame_id ++;
    LOG << "[DstEngine]: End of result info.";
    return HIAI_OK;
}

HIAI_IMPL_ENGINE_PROCESS("DstEngine", DstEngine, DST_INPUT_SIZE)
{
    HIAI_ENGINE_LOG(HIAI_INFO, "[DstEngine] start process!");
    if (arg0 == nullptr){
        HIAI_ENGINE_LOG(HIAI_IDE_ERROR, "[DstEngine]  The input arg0 is nullptr");
        return HIAI_ERROR;
    }
    auto frame_result = std::static_pointer_cast<DeviceStreamData>(arg0);
       if(frame_result == nullptr){
        return HIAI_OK;
    }
    // if it is the end of stream, send end signal to main
    if(frame_result->info.isEOS){
        std::shared_ptr<std::string> result_data(new std::string);
        hiai::Engine::SendData(0, "string", std::static_pointer_cast<void>(result_data));
        return HIAI_OK;
    }

    // Process result
    if(ProcessResult(frame_result) != HIAI_OK){
        HIAI_ENGINE_LOG(HIAI_IDE_ERROR, "[DstEngine]  process result failed");
        return HIAI_ERROR;
    } 

    HIAI_ENGINE_LOG(HIAI_INFO, "[DstEngine] end process!");
    return HIAI_OK;
}

void DstEngine::report_alarm_fun() {
    LOG << "go report alarm fun";
    std::unique_ptr<rpc::Client> cli(
    //    rpc::new_client(FLG_ae_serv_ip.c_str(), FLG_ae_serv_port, FLG_ae_serv_passwd.c_str())
        rpc::new_client(client_ip_.c_str(), FLG_ae_serv_port, FLG_ae_serv_passwd.c_str())
    );

    while (!stop_) {
        alarm_ev_.wait(256);
        if (stop_) break;
        LOG << "report alarm";
        std::vector<Json> alarms;
        {
            MutexGuard g(alarm_mtx_);
            alarms.swap(alarms_);
        }

        Json res;
        for (size_t i = 0; i < alarms.size(); ++i) {
            LOG << "report alarm\n";
            cli->call(alarms[i], res);
        }
    }
}
