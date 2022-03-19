#pragma once

#include <czmq.h>
#include <string>
#include "BMSData.h"

class ZMQServer {
public:
    ZMQServer(unsigned short port, const char* topic);
    ~ZMQServer();

    void SendData(const JKBMSData& data);

private:
    zsock_t* _socket;
    std::string _topic;
};
