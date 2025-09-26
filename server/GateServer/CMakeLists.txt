cmake_minimum_required(VERSION 3.20)
project(GateServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-D_WIN32_WINNT=0x0601)  # 针对 Windows 7 及以上版本

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 要拷贝到构建输出目录的 bin/ 的资源文件列表
set(PROJECT_RUNTIME_FILES
        ${CMAKE_SOURCE_DIR}/config.ini
        ${CMAKE_SOURCE_DIR}/dll/mysqlcppconnx-2-vs14.dll
        ${CMAKE_SOURCE_DIR}/dll/mysqlcppconn-10-vs14.dll
)

# 逐个拷贝文件
foreach(file ${PROJECT_RUNTIME_FILES})
    configure_file(
            ${file}
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
            COPYONLY
    )
endforeach()

# 源文件
add_executable(main
        main.cpp
        core/CServer.cpp
        core/HttpConnection.cpp
        core/LogicSystem.cpp
        utils/ConfigManager.cpp
        protobuf/message.grpc.pb.cc
        protobuf/message.pb.cc
        asio/AsioIOContextPool.cpp
        rpc/VerifyGrpcClient.cpp
        rpc/RpcConnectionPool.tpp
        redis/RedisManager.cpp
        redis/RedisConnectionPool.cpp
        mysql/MysqlConnectionPool.cpp
        mysql/MysqlDao.cpp
        mysql/MysqlManager.cpp
        mysql/ConnectionGuard.cpp
        rpc/StatusGrpcClient.cpp
)

# 设置路径
set(BOOST_ROOT "E:/C++/GateServer/third_party/boost_1_88_0")
set(JSON_CPP_ROOT "E:/C++/GateServer/third_party/json_cpp")
set(GRPC_ROOT "E:/C++/GateServer/third_party/grpc")
set(HIREDIS_ROOT "E:/C++/GateServer/third_party/hiredis")
set(MYSQL_CONNECTOR_ROOT "E:/C++/GateServer/third_party/mysql_connector")

# 头文件路径
target_include_directories(main PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${BOOST_ROOT}/include
        ${JSON_CPP_ROOT}/include
        ${GRPC_ROOT}/include
        ${GRPC_ROOT}/third_party/protobuf/src
        ${GRPC_ROOT}/third_party/re2
        ${GRPC_ROOT}/third_party/address_sorting/include
        ${GRPC_ROOT}/third_party/abseil-cpp
        ${HIREDIS_ROOT}/deps
        ${MYSQL_CONNECTOR_ROOT}/include
)

# 手动指定 lib 路径
target_link_directories(main PRIVATE
        "${JSON_CPP_ROOT}/lib"
        "${BOOST_ROOT}/lib"
        "${GRPC_ROOT}/visualpro/Debug"  # gRPC 主库目录
        "${GRPC_ROOT}/visualpro/third_party/protobuf/Debug"  # libprotobufd.lib
        "${GRPC_ROOT}/visualpro/third_party/cares/cares/lib/Debug"  # cares.lib
        "${GRPC_ROOT}/visualpro/third_party/zlib/Debug"  # zlib.lib
        "${GRPC_ROOT}/visualpro/third_party/abseil-cpp/absl/types/Debug"
        "${GRPC_ROOT}/visualpro/third_party/abseil-cpp/absl/synchronization/Debug"
        "${GRPC_ROOT}/visualpro/third_party/abseil-cpp/absl/status/Debug"
        "${GRPC_ROOT}/visualpro/third_party/abseil-cpp/absl/random/Debug"
        "${GRPC_ROOT}/visualpro/third_party/abseil-cpp/absl/flags/Debug"
        "${GRPC_ROOT}/visualpro/third_party/abseil-cpp/absl/debugging/Debug"
        "${GRPC_ROOT}/visualpro/third_party/abseil-cpp/absl/container/Debug"
        "${GRPC_ROOT}/visualpro/third_party/abseil-cpp/absl/hash/Debug"
        "${GRPC_ROOT}/visualpro/third_party/boringssl-with-bazel/Debug"  # BoringSSL 库目录
        "${GRPC_ROOT}/visualpro/third_party/abseil-cpp/absl/numeric/Debug"
        "${GRPC_ROOT}/visualpro/third_party/abseil-cpp/absl/time/Debug"
        "${GRPC_ROOT}/visualpro/third_party/abseil-cpp/absl/base/Debug"
        "${GRPC_ROOT}/visualpro/third_party/abseil-cpp/absl/strings/Debug"
        "${GRPC_ROOT}/visualpro/third_party/re2/Debug"  # re2 库目录
        "${HIREDIS_ROOT}/lib"
        "${MYSQL_CONNECTOR_ROOT}/lib64/debug/vs14"
)

# 链接库文件
target_link_libraries(main PRIVATE
        jsoncpp.lib
        ws2_32.lib
        crypt32.lib
        libprotobufd.lib
        gpr.lib
        grpc.lib
        grpc++.lib
        grpc++_reflection.lib
        address_sorting.lib
        ws2_32.lib
        cares.lib
        zlibstaticd.lib
        upb.lib
        ssl.lib
        crypto.lib
        absl_bad_any_cast_impl.lib
        absl_bad_optional_access.lib
        absl_bad_variant_access.lib
        absl_base.lib
        absl_city.lib
        absl_civil_time.lib
        absl_cord.lib
        absl_debugging_internal.lib
        absl_demangle_internal.lib
        absl_examine_stack.lib
        absl_exponential_biased.lib
        absl_failure_signal_handler.lib
        absl_flags.lib
        absl_flags_config.lib
        absl_flags_internal.lib
        absl_flags_marshalling.lib
        absl_flags_parse.lib
        absl_flags_program_name.lib
        absl_flags_usage.lib
        absl_flags_usage_internal.lib
        absl_graphcycles_internal.lib
        absl_hash.lib
        absl_hashtablez_sampler.lib
        absl_int128.lib
        absl_leak_check.lib
        absl_leak_check_disable.lib
        absl_log_severity.lib
        absl_malloc_internal.lib
        absl_periodic_sampler.lib
        absl_random_distributions.lib
        absl_random_internal_distribution_test_util.lib
        absl_random_internal_pool_urbg.lib
        absl_random_internal_randen.lib
        absl_random_internal_randen_hwaes.lib
        absl_random_internal_randen_hwaes_impl.lib
        absl_random_internal_randen_slow.lib
        absl_random_internal_seed_material.lib
        absl_random_seed_gen_exception.lib
        absl_random_seed_sequences.lib
        absl_raw_hash_set.lib
        absl_raw_logging_internal.lib
        absl_scoped_set_env.lib
        absl_spinlock_wait.lib
        absl_stacktrace.lib
        absl_status.lib
        absl_strings.lib
        absl_strings_internal.lib
        absl_str_format_internal.lib
        absl_symbolize.lib
        absl_synchronization.lib
        absl_throw_delegate.lib
        absl_time.lib
        absl_time_zone.lib
        absl_statusor.lib
        re2.lib
        hiredis.lib
        Win32_Interop.lib
        mysqlcppconn.lib
)