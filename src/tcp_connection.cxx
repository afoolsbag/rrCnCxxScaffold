/// \copyright Unlicense

#include "tcp_connection.hxx"

#include <boost/bind/bind.hpp>
#include <boost/core/ignore_unused.hpp>
#include <rrspdlog/rrspdlog.hxx>

using namespace std;
namespace asio = boost::asio;

tcp_connection::tcp_connection(asio::io_context &io_context) :
    socket_ {io_context}
{}

void tcp_connection::start()
{
    remote_address_ = socket_.remote_endpoint().address().to_string();
    remote_port_ = socket_.remote_endpoint().port();
    SPDLOG_INFO("TCP server: New connection from {}:{}.", remote_address_, remote_port_);
    async_receive();
}

asio::ip::tcp::socket &tcp_connection::socket() noexcept
{
    return socket_;
}

tcp_connection::~tcp_connection() noexcept
{
    if (!remote_address_.empty()) {
        assert(remote_port_ != 0);
        SPDLOG_INFO("TCP server: Disconnect with {}:{}.", remote_address_, remote_port_);
    }
}

void tcp_connection::async_receive()
{
    auto req_ptr = make_shared<asio::streambuf>();
    asio::async_read_until(socket_, *req_ptr, '\0', boost::bind(
        &tcp_connection::on_received, shared_from_this(), req_ptr, asio::placeholders::error, asio::placeholders::bytes_transferred));
}

void tcp_connection::on_received(shared_ptr<asio::streambuf> req_ptr, const boost::system::error_code &err, size_t bytes) noexcept
{
    boost::ignore_unused(bytes);

    try {
        if (!err) {
            const string request_message {istreambuf_iterator<char>{req_ptr.get()}, istreambuf_iterator<char>{}};
            SPDLOG_DEBUG("TCP server: New request by {}:{}: {}", remote_address_, remote_port_, request_message);
            async_send("hello, world");
            async_receive();
        } else {
            SPDLOG_INFO("TCP server: Callback {} was invoked with error {}.", __func__, err.message());
        }

    } catch (const exception &e) {
        SPDLOG_WARN("TCP server: Caught standard exception in {} callback: {}.", __func__, e.what());

    } catch (...) {
        SPDLOG_WARN("TCP server: Caught non-standard exception in {} callback.", __func__);
    }
}

void tcp_connection::async_send(const string &res)
{
    auto res_ptr = make_shared<string>(res);
    asio::async_write(socket_, asio::buffer(*res_ptr), boost::bind(
        &tcp_connection::on_sent, shared_from_this(), res_ptr, asio::placeholders::error, asio::placeholders::bytes_transferred));
}

void tcp_connection::on_sent(shared_ptr<string> res_ptr, const boost::system::error_code &err, size_t bytes) noexcept
{
    boost::ignore_unused(res_ptr);
    boost::ignore_unused(bytes);

    try {
        if (!err) {
            /*PASS*/;
        } else {
            SPDLOG_INFO("TCP server: Callback {} was invoked with error {}.", __func__, err.message());
        }

    } catch (const exception &e) {
        SPDLOG_WARN("TCP server: Caught standard exception in {} callback: {}.", __func__, e.what());

    } catch (...) {
        SPDLOG_WARN("TCP server: Caught non-standard exception in {} callback.", __func__);
    }
}
