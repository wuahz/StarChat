#include "const.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class CServer: public std::enable_shared_from_this<CServer>
{
public:
    CServer(boost::asio::io_context& ioc, unsigned short& port);
    void Start();
private: 
    tcp::acceptor  _acceptor; // 用于异步接受新的 TCP 连接的接受器。
    net::io_context& _ioc; // Boost.Asio 的 I/O 上下文，用于管理异步操作。
};