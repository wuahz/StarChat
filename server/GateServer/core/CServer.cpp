#include "CServer.h"
#include "HttpConnection.h"
#include "asio/AsioIOContextPool.h"

CServer::CServer(boost::asio::io_context& ioc, unsigned short& port) :_ioc(ioc),
    _acceptor(ioc, tcp::endpoint(tcp::v4(), port)) {

}

void CServer::Start()
{    
    auto self = shared_from_this();
    auto& io_context = AsioIOContextPool::GetInstance()->GetIOContext();
    std::shared_ptr<HttpConnection> new_connection = std::make_shared<HttpConnection>(io_context);
    _acceptor.async_accept(new_connection->GetSocket(), [self, new_connection](beast::error_code ec) {
        try {
            //出错则放弃这个连接，继续监听新链接
            if (ec) {
                self->Start();
                return;
            }

            //处理新链接，创建HpptConnection类管理新连接
            new_connection->Start();
            //继续监听
            self->Start();
        }
        catch (std::exception& exp) {
            std::cout << "exception is " << exp.what() << std::endl;
            self->Start();
        }
    });
}