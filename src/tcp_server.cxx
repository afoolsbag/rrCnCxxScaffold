/// \copyright Unlicense

#include "tcp_server.hxx"

#include <boost/bind/bind.hpp>
#include <rrspdlog/rrspdlog.hxx>

using namespace std;
namespace asio = boost::asio;

tcp_server::tcp_server(asio::io_context &io_context, uint16_t port) :
    io_context_ {io_context},
    acceptor_ {io_context, asio::ip::tcp::endpoint{asio::ip::tcp::v4(), port }}
{
    SPDLOG_INFO("TCP server: Start listening on port {}.", port);
    async_accept();
}

void tcp_server::async_accept()
{
    auto conn_ptr = make_shared<tcp_connection>(io_context_);
    acceptor_.async_accept(
        conn_ptr->socket(),
        boost::bind(&tcp_server::on_accepted, this, conn_ptr, asio::placeholders::error));
}

void tcp_server::on_accepted(shared_ptr<tcp_connection> conn_ptr, const boost::system::error_code &err) noexcept
{
    try {
        if (!err) {
            conn_ptr->start();
        } else {
            SPDLOG_INFO("TCP server: Callback {} was invoked with error {}.", __func__, err.message());
        }
        async_accept();

    } catch (const exception &e) {
        SPDLOG_WARN("TCP server: Caught standard exception in {} callback: {}.", __func__, e.what());

    } catch (...) {
        SPDLOG_WARN("TCP server: Caught non-standard exception in {} callback.", __func__);
    }

}
