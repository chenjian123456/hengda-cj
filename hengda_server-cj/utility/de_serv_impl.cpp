#include "de_serv_impl.h"
#include "bordersloader.h"
#include "base/log.h"
#include "base/fs.h"     // for fs::file
#include "base/hash.h"   // for base64_encode
#include "json11.hpp"
#include "bordersloader.h"


DeServImpl::DeServImpl() {
}

DeServImpl::~DeServImpl() {
}

void DeServImpl::set_egbd_task(const Json& req, Json& res) 
{ 
	LOG << "set_egbd_task: " << req;

    res.add_member("req_id", req["req_id"]);	
    res.add_member("method", req["method"]);

	fastring s = req["params"].str();

	if (s.empty())
	{
		res.add_member("code", 400);
        res.add_member("code_msg", "400 params not found");
        return;
	}

	Json params = req["params"];
	fastring streamName = params["rtsp_url"].get_string();
	
    if (streamName.empty())
    {
        res.add_member("code", 404);
        res.add_member("err_msg", "400 rtsp_url not found");
        return;
    }

    uint32_t GRAPH_ID = 100 + process_num;
    string ip = params["ip"].get_string();
    string content = params["rtsp_url"].get_string();

    ifstream config(configFileName); //
    if(!config.is_open()){
        printf("open file failed\n");
    }

    const string newconfigFileName = "graph" +to_string(process_num)+".config";
        
    ofstream config_new(newconfigFileName);
    string temp;
    int linenum = 1;//读取的内容行数
    //边读config边写进临时config文件
    while(getline(config, temp)){
        //如果是需要的行，进行修改
        if(linenum == 18){
            temp = temp.substr(0,temp.find(":") + 1)+"\"" + content + "\"";
            config_new << temp;
            config_new << endl;
            linenum++;
        }
        //如果是第二行，改变GraphID
        else if(linenum == 2){
                temp = temp.substr(0,temp.find(":") + 1)+to_string(GRAPH_ID);
                config_new << temp;
                config_new << endl;
                linenum++;
        }
        else if(linenum == 76){
            temp = temp.substr(0,temp.find(":") + 1)+"\""+ip + "\"";
            config_new << temp;
            config_new << endl;
            linenum++;
        }
        //其他行直接复制
        else{
            config_new << temp;
            config_new << endl;
            linenum++;
        }
    }
    config.close();
    config_new.close();

    // init Graph
    HIAI_StatusT status = HIAI_InitAndStartGraph(newconfigFileName,GRAPH_ID);
    if (status != HIAI_OK) {
        printf("[main] Fail to start graph\n");
        return;
    }

    // send data
    std::shared_ptr<hiai::Graph> graph = hiai::Graph::GetInstance(GRAPH_ID);
    if (nullptr == graph) {
        printf("Fail to get the graph-%u\n", GRAPH_ID);
        return;
    }
    hiai::EnginePortID engine_id;
    engine_id.graph_id = GRAPH_ID;
    engine_id.engine_id = SRC_ENGINE;
    engine_id.port_id = 0;
    std::shared_ptr<std::string> src_data(new std::string());

    graph->SendData(engine_id, "string", std::static_pointer_cast<void>(src_data));
    pidMap[content] = GRAPH_ID;
    process_num++;


    //startStream(streamName);

    res.add_member("code", 200);
    res.add_member("code_msg", "200 ok");
}

void DeServImpl::get_key_frame(const Json& req, Json& res) 
{
	LOG << "get_key_frame: " << req;

	res.add_member("req_id", req["req_id"]);
    res.add_member("method", req["method"]);

    std::string ip= req["params"]["ip"].get_string();
    std::string filename = "/home/" + ip + ".jpg";


    fs::file f(filename.c_str(), 'r');


    if (f) {
        fastring s = f.read(f.size());
        res.add_member("code", 200);
        res.add_member("code_msg", "200 ok");
        res.add_member("data", base64_encode(s));
    } else {
        res.add_member("code", 400);
        res.add_member("code_msg", "400 key frame not found");
    }

    f.close();
}

void DeServImpl::set_border_info(const Json& req, Json& res) 
{
	LOG << "set_border_info: " << req;

	res.add_member("req_id", req["req_id"]);
    res.add_member("method", req["method"]);

	Json params = req["params"];

    fastring s = params["info"].str();
    fastring filename_ = params["ip"].get_string();
    std::string filename = filename_.c_str();

    filename += ".json";


    std::string err;
    json11::Json borders;

    borders = json11::Json::parse(s.c_str(), err);
    if(!err.empty()) {
        borders = json11::Json::array();
        res.add_member("code", "400");
        res.add_member("code_msg", "400 parse borders error");
        return;
    } 

    LOG << "save borders: " << s;
    std::cout << "border filename is "<< filename;
    LOG << "filename is " << filename;
    BordersLoader::Instance().save(borders,filename);
    res.add_member("code", 200);
    res.add_member("code_msg", "200 ok");
}

HIAI_StatusT DeServImpl::HIAI_InitAndStartGraph(const std::string& configFile,uint32_t GRAPH_ID)
{
        std::cout << "HIAI_InitAndStartGraph: configFile is " << configFile << std::endl;
        
        // Step1: Global System Initialization before using HIAI Engine
        HIAI_StatusT status = HIAI_Init(0);

        // Step2: Create and Start the Graph
        status = hiai::Graph::CreateGraph(configFile);
        if (status != HIAI_OK) {
            HIAI_ENGINE_LOG(status, "Fail to start graph");
            printf("Fail to start graph: %#X\n", status);
            return status;
        }

        // Step3: Set DataRecv Functor
        std::shared_ptr<hiai::Graph> graph = hiai::Graph::GetInstance(GRAPH_ID);
        if (graph == nullptr) {
            HIAI_ENGINE_LOG("Fail to get the graph-%u", GRAPH_ID);
            return status;
        }

       
        hiai::EnginePortID target_port_config;
        target_port_config.graph_id = GRAPH_ID;
        target_port_config.engine_id = ENGINE_ID;
        target_port_config.port_id = 0;
        graph->SetDataRecvFunctor(target_port_config,
        std::make_shared<CustomDataRecvInterface>(""));
        return HIAI_OK;
}





