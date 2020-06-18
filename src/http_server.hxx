/// \copyright Unlicense

#pragma once
#ifndef HTTP_SERVER_HXX_
#define HTTP_SERVER_HXX_

#include <cstdint>
#include <memory>
#include <boost/asio.hpp>

#include "http_connection.hxx"

/// \brief 超文本传输协议伺服器
class http_server {
public:
    /// \biref 构造函数
    /// \details 初始化接收器，并监听端口
    explicit http_server(boost::asio::io_context &io_context, std::uint16_t port);

private:
    /// \brief 异步接受下一个连接
    void async_accept();

    /// \brief 异步接受到连接时的回调
    /// \param conn_ptr 连接共享指针（connection pointer）
    /// \param err      错误码（error code）
    void on_accepted(std::shared_ptr<http_connection> conn_ptr, const boost::system::error_code &err) noexcept;

    boost::asio::io_context &io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
};

#endif
