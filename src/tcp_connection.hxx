/// \copyright Unlicense

#pragma once
#ifndef TCP_CONNECTION_HXX_
#define TCP_CONNECTION_HXX_

#include <memory>
#include <string>

#include <boost/asio.hpp>

class tcp_connection : public std::enable_shared_from_this<tcp_connection> {
public:
    explicit tcp_connection(boost::asio::io_context &io_context);

    void start();

    boost::asio::ip::tcp::socket &socket() noexcept;

    virtual ~tcp_connection() noexcept;

private:
    /// \biref 异步接收下一个请求
    void async_receive();

    /// \brief 异步接收到请求时的回调
    /// \param req_ptr 请求报文共享指针（request pointer）
    /// \param err     错误码（error code）
    /// \param bytes   已传输字节（bytes_transferred）
    void on_received(std::shared_ptr<boost::asio::streambuf> req_ptr, const boost::system::error_code &err, std::size_t bytes) noexcept;
public:
    /// \biref 异步发送下一个回应
    /// \param res 回应报文（response）
    inline void async_send(const std::string &res);
private:
    /// \biref 异步发送出回应时的回调
    /// \param res_ptr 回应报文共享指针（response pointer）
    /// \param err     错误码（error code）
    /// \param bytes   已传输字节（bytes_transferred）
    void on_sent(std::shared_ptr<std::string> res_ptr, const boost::system::error_code &err, std::size_t bytes) noexcept;

    boost::asio::ip::tcp::socket socket_;
    std::string remote_address_ {};
    std::uint16_t remote_port_ {};
};

#endif
