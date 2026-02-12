#include "websocket/ws_server.h"

#include <websocketpp/common/connection_hdl.hpp>
#include <functional>

WsServer::WsServer() {
    m_server.clear_access_channels(websocketpp::log::alevel::all);
    m_server.clear_error_channels(websocketpp::log::elevel::all);

    m_server.init_asio();

    m_server.set_open_handler(std::bind(&WsServer::on_open, this, std::placeholders::_1));
    m_server.set_close_handler(std::bind(&WsServer::on_close, this, std::placeholders::_1));
}

void WsServer::run(uint16_t port) {
    m_server.listen(port);
    m_server.start_accept();
    m_server.run();
}

void WsServer::broadcast(const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& hdl : m_connections) {
        websocketpp::lib::error_code ec;
        m_server.send(hdl, message, websocketpp::frame::opcode::text, ec);
    }
}

void WsServer::on_open(websocketpp::connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_connections.insert(hdl);
}

void WsServer::on_close(websocketpp::connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_connections.erase(hdl);
}
