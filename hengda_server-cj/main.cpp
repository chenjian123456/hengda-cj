#include "DataRecv.h"
#include "hiaiengine/api.h"
#include <atomic>
#include <iostream>
#include <libgen.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <dirent.h>
#include <cstring>
#include "fstream"
#include "stdlib.h"
#include <stdio.h>
#include <vector>
#include <map>
#include <sys/stat.h>
#include <cmath>

#include "utility/de_serv_impl.h"
#include "base/co.h"
#include "base/rpc.h"
#include "base/fastring.h"
#include "base/thread.h"
#include "base/flag.h"
#include "base/log.h"
#include "base/fs.h"
#include "base/time.h"

#include <iostream> /* cout */
#include <unistd.h>/* gethostname */
#include <netdb.h> /* struct hostent */
#include <arpa/inet.h> /* inet_ntop */




using  namespace std;
using namespace hiai;

DEF_string(de_serv_ip, "192.168.2.111", "DstEngine tcp server ip");
DEF_uint32(de_serv_port, 9910, "DstEngine tcp server port");
DEF_string(de_serv_passwd, "", "passwd for DstEngine tcp server");

bool GetIp(std::string& ip)
{
    fs::file f("network.json",'r');
    char buf[512];
    if(f){
        f.read(buf,256);
    }
    fastring s(buf);
    Json y = json::parse(s.data(),s.size());
    std::cout << y.pretty() << std::endl;
    Json ip_ = y.find("server_ip");
    if(ip_.is_string()){
        ip = ip_.get_string();
        f.close();
        return true;

    }
    else {
        f.close();
        return false;
    }

}


// flag to guard eos signal
static std::atomic<int> g_flag = { 1 };

HIAI_StatusT CustomDataRecvInterface::RecvData(const std::shared_ptr<void>& message)
{
    std::shared_ptr<std::string> data = std::static_pointer_cast<std::string>(message);
    std::cout << "RecvData g_flag " << g_flag << "\n";
    g_flag--;
    return HIAI_OK;
}

map<string,int> pidMap;//保存进程id，方便以后关闭进程serverip_->text()
//查找path路径下的所有文件
vector<string> findFileNames(const char* path){
    vector<string> fileNames;
    struct dirent *ptr;
    DIR *dir;
    dir=opendir(path);
    while((ptr=readdir(dir))!=NULL)
    {
        //跳过'.'和'..'两个目录
        if(ptr->d_name[0] == '.')
            continue;
        fileNames.push_back(ptr->d_name);
    }
    closedir(dir);
    return fileNames;
}


int main(int argc, char* argv[])
{
    // int process_num = 1;
    // cd to directory of main
    char* dirc = strdup(argv[0]);
    if (dirc != NULL) {
        char* dname = ::dirname(dirc);
        int r = chdir(dname);
        if (r != 0) {
            printf("chdir error code %d\n", r);
            return -1;
        }
        free(dirc);
    }

    //Get Host ip

    std::string Ip;

    bool ret = GetIp(Ip);
    if(ret == false){
        printf("get host info error!\n");
        ELOG << "[SaveFile] Save file engine: open file fail!";
        return -1;
    }
    else{
		std::cout << "Ip: " << Ip << std::endl;
        
        rpc::Server* server = rpc::new_server(Ip.c_str(), FLG_de_serv_port, FLG_de_serv_passwd.c_str());
        server->add_service(new DeServImpl);
        server->start();

    }
	


    while(true){
        sleep::sec(1);
    //    printf("running...\n");
    }

    return 0;                  // 判断 object 是否为空
}
