#pragma once
#include "adapters/interface_adapters/interface_transport_adapter.h"
#include "websocket/ws_server.h"
#include <memory>

class WebSocketAdapter : public ITransportAdapter {
public:
    explicit WebSocketAdapter(std::shared_ptr<WsServer> ws_server);
    
    bool init() override;
    void send(const iot::SensorRequest& request) override;
    std::string name() const override;

private:
    std::shared_ptr<WsServer> ws_server_;
};
