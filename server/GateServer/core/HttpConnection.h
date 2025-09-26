#include "const.h"

class HttpConnection : public std::enable_shared_from_this<HttpConnection>
{
    friend class LogicSystem;
public:
    HttpConnection(boost::asio::io_context& ioc);
    void Start();
    tcp::socket& GetSocket();

private:
    void CheckDeadline();
    void WriteResponse();
    void HandleRequest();
    void PreParseGetParam();
    
    tcp::socket  socket_;
    // The buffer for performing reads.
    beast::flat_buffer  buffer_{ 8192 };

    // The request message.
    http::request<http::dynamic_body> request_;

    // The response message.
    http::response<http::dynamic_body> response_;

    // The timer for putting a deadline on connection processing.
    net::steady_timer deadline_{
            socket_.get_executor(), std::chrono::seconds(60) };

    std::string get_url_;
    std::unordered_map<std::string, std::string> get_params_;
};