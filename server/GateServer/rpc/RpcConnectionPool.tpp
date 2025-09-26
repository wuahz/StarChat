//
// RpcConnectionPool.tpp
//

template <typename ServiceType>
RpcConnectionPool<ServiceType>::RpcConnectionPool(const std::string& host,
                                                  const std::string& port,
                                                  const std::string& pool_size)
        : host_(host), port_(port), pool_size_(pool_size), is_stopped_(false) {
    // 创建gRPC通道并初始化连接池
    for (size_t i = 0; i < std::stoi(pool_size_); ++i) {
        auto channel = grpc::CreateChannel(host_ + ":" + port_, grpc::InsecureChannelCredentials());
        connection_queue_.push(ServiceType::NewStub(channel));
    }
}

template <typename ServiceType>
RpcConnectionPool<ServiceType>::~RpcConnectionPool() {
    Close();
    std::lock_guard<std::mutex> lock(mutex_);
    while (!connection_queue_.empty()) {
        connection_queue_.pop();
    }
}

template <typename ServiceType>
std::unique_ptr<typename ServiceType::Stub> RpcConnectionPool<ServiceType>::AcquireConnection() {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_var_.wait(lock, [this] {
        return is_stopped_ || !connection_queue_.empty();
    });

    if (is_stopped_) {
        return nullptr;
    }

    auto connection = std::move(connection_queue_.front());
    connection_queue_.pop();
    return connection;
}

template <typename ServiceType>
void RpcConnectionPool<ServiceType>::ReleaseConnection(std::unique_ptr<typename ServiceType::Stub> connection) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (is_stopped_) {
        return;
    }
    connection_queue_.push(std::move(connection));
    cond_var_.notify_one();
}

template <typename ServiceType>
void RpcConnectionPool<ServiceType>::Close() {
    is_stopped_ = true;
    cond_var_.notify_all();
}
