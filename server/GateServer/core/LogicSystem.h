#include "const.h"

class HttpConnection;
// HTTP请求处理函数类型定义，接收HttpConnection智能指针参数
typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;

/**
 * @brief 逻辑系统类，单例模式，负责HTTP请求的注册与处理
 *
 * 职责：
 *  - 管理GET/POST请求处理函数的注册与分发
 *  - 接收HTTP连接并调用对应处理函数
 */
class LogicSystem : public Singleton<LogicSystem>
{
    friend class Singleton<LogicSystem>;
public:
    /**
     * @brief 析构函数
     */
    ~LogicSystem() = default;

    /**
     * @brief 处理GET请求
     * @param url 请求URL路径
     * @param connection HTTP连接对象智能指针
     * @return 是否成功处理（true:找到对应处理器并执行，false:未找到处理器）
     */
    bool HandleGet(const std::string& url, const std::shared_ptr<HttpConnection>& connection);

    /**
     * @brief 处理POST请求
     * @param url 请求URL路径
     * @param connection HTTP连接对象智能指针
     * @return 是否成功处理（true:找到对应处理器并执行，false:未找到处理器）
     */
    bool HandlePost(const std::string& url, const std::shared_ptr<HttpConnection>& connection);

    /**
     * @brief 注册GET请求处理函数
     * @param url 关联的URL路径（作为键）
     * @param handler 处理函数对象（作为值）
     */
    void RegisterGet(const std::string& url, const HttpHandler& handler);

    /**
     * @brief 注册POST请求处理函数
     * @param url 关联的URL路径（作为键）
     * @param handler 处理函数对象（作为值）
     */
    void RegisterPost(const std::string& url, const HttpHandler& handler);

private:
    /**
     * @brief 私有构造函数（单例模式禁止外部实例化）
     */
    LogicSystem();

    std::map<std::string, HttpHandler> post_handlers_; ///< 注册的POST请求处理函数映射表
    std::map<std::string, HttpHandler> get_handlers_; ///< 注册的GET请求处理函数映射表
};