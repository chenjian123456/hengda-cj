#pragma once

#include "base/rpc.h"
#include "base/hash.h"
#include <unordered_map>

// !! AUTO generated by rpcgen. DO NOT modify it!!

class DeServ : public rpc::Service {
  public:
    typedef void (DeServ::*Fun)(const Json&, Json&);

    DeServ() {
        _methods[hash64("set_egbd_task")] = &DeServ::set_egbd_task;
        _methods[hash64("get_key_frame")] = &DeServ::get_key_frame;
        _methods[hash64("set_border_info")] = &DeServ::set_border_info;
    }

    virtual ~DeServ() {}

    virtual void process(const Json& req, Json& res) {
        printf("in process\n");
        Json& method = req["method"];
        if (!method.is_string()) {
            res.add_member("err", 400);
            res.add_member("errmsg", "400 req has no method");
            return;
        }

        auto it = _methods.find(hash64(method.get_string(), method.size()));
        if (it == _methods.end()) {
            res.add_member("err", 404);
            res.add_member("errmsg", "404 method not found");
            return;
        }

        (this->*it->second)(req, res);
    }

    virtual void set_egbd_task(const Json& req, Json& res) = 0;

    virtual void get_key_frame(const Json& req, Json& res) = 0;

    virtual void set_border_info(const Json& req, Json& res) = 0;

  private:
    std::unordered_map<uint64, Fun> _methods;
};
