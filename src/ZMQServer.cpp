#include "ZMQServer.hpp"
#include <stdexcept>

ZMQServer::ZMQServer(unsigned short port, const char* topic) : _socket(zsock_new(ZMQ_PUB)), _topic(topic) {
    if(zsock_bind(_socket, "tcp://127.0.0.1:%u", port) != port) throw std::runtime_error("ZMQ Bind failed");
}

ZMQServer::~ZMQServer() {
    zsock_destroy(&_socket);
}

void ZMQServer::SendData(const JKBMSData& data) {
    auto msg = zmsg_new();
    zmsg_addstr(msg, _topic.c_str());
    zmsg_addmem(msg, reinterpret_cast<const void*>(&data), sizeof(data));
    zmsg_send(&msg, _socket);
}
