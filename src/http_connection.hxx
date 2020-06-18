/// \copyright Unlicense

#pragma once
#ifndef HTTP_CONNECTION_HXX_
#define HTTP_CONNECTION_HXX_

#include <memory>
#include <string>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

class http_connection : public std::enable_shared_from_this<http_connection> {
public:
    explicit http_connection(boost::asio::io_context &io_context);

    void start();

    boost::asio::ip::tcp::socket &socket() noexcept;

    virtual ~http_connection() noexcept;

private:
    /// \biref 异步读取请求
    void async_read_request();

    /// \brief 异步读取请求完毕时的回调
    /// \param err     错误码（error code）
    /// \param bytes   已传输字节（bytes_transferred）
    /// \sa [HTTP 消息头](https://developer.mozilla.org/docs/Web/HTTP/Headers)
    /// \sa [常见 MIME 类型列表](https://developer.mozilla.org/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Common_types)
    /// \sa [HTTP 请求方法](https://developer.mozilla.org/docs/Web/HTTP/Methods)
    /// \sa [HTTP 相应代码](https://developer.mozilla.org/docs/Web/HTTP/Status)
    void on_request_read(const boost::beast::error_code &err, size_t bytes) noexcept;

    /// \biref 异步写入回应
    void async_write_response();

    /// \biref 异步写入回应完毕时的回调
    /// \param err     错误码（error code）
    /// \param bytes   已传输字节（bytes_transferred）
    void on_response_written(const boost::beast::error_code &err, size_t bytes) noexcept;

    /// \brief 异步等待死线
    void async_wait_deadline();

    /// \brief 当死线到期
    void when_deadline_expired(const boost::system::error_code &err) noexcept;

    boost::asio::ip::tcp::socket socket_;
    std::string remote_address_ {};
    std::uint16_t remote_port_ {};
    boost::asio::steady_timer deadline_;

    boost::beast::flat_buffer buffer_ {8192};
    boost::beast::http::request<boost::beast::http::string_body> request_ {};
    boost::beast::http::response<boost::beast::http::dynamic_body> response_ {};
};

#endif
