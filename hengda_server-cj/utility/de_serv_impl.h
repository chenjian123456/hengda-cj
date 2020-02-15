#pragma once

#include "de_serv.h"

#include "DataRecv.h"
#include "hiaiengine/api.h"
#include "base/rpc.h"
#include "base/hash.h"
#include <unordered_map>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;
using namespace hiai;

class DeServImpl : public DeServ {
  public:
    DeServImpl();
    virtual ~DeServImpl();

    virtual void set_egbd_task(const Json& req, Json& res);

    virtual void get_key_frame(const Json& req, Json& res);

    virtual void set_border_info(const Json& req, Json& res);

  

  private:
    HIAI_StatusT HIAI_InitAndStartGraph(const std::string& configFile,uint32_t GRAPH_ID);

    const string configFileName = "./graph.config";
    int process_num = 1;
    map<string,int> pidMap;
    const uint32_t SRC_ENGINE = 101;
    const uint32_t ENGINE_ID = 106;

};

