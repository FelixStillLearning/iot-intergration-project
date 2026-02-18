#pragma once
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <set>
#include <mutex>

typedef websocketpp::server<websocketpp::config::asio> server;

class WsServer {
public:
    WsServer();
    void run(uint16_t port);
    void broadcast(const std::string& message);

private:
    void on_open(websocketpp::connection_hdl hdl);
    void on_close(websocketpp::connection_hdl hdl);

    server m_server;
    std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> m_connections;
    std::mutex m_mutex;
};
