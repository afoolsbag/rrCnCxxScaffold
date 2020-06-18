/// \copyright Unlicense

#include "http_server.hxx"

#include <boost/bind/bind.hpp>
#include <rrspdlog/rrspdlog.hxx>

using namespace std;
namespace asio = boost::asio;

http_server::http_server(asio::io_context &io_context, uint16_t port) :
    io_context_ {io_context},
    acceptor_ {io_context, asio::ip::tcp::endpoint{asio::ip::tcp::v4(), port }}
{
    SPDLOG_INFO("HTTP server: Start listening on port {}.", port);
    async_accept();
}

void http_server::async_accept()
{
    auto conn_ptr = make_shared<http_connection>(io_context_);
    acceptor_.async_accept(
        conn_ptr->socket(),
        boost::bind(&http_server::on_accepted, this, conn_ptr, asio::placeholders::error));
}

void http_server::on_accepted(shared_ptr<http_connection> conn_ptr, const boost::system::error_code &err) noexcept
{
    try {
        if (!err) {
            conn_ptr->start();
        } else {
            SPDLOG_INFO("HTTP server: Callback {} was invoked with error {}.", __func__, err.message());
        }
        async_accept();

    } catch (const exception &e) {
        SPDLOG_WARN("HTTP server: Caught standard exception in {} callback: {}.", __func__, e.what());

    } catch (...) {
        SPDLOG_WARN("HTTP server: Caught non-standard exception in {} callback.", __func__);
    }

}
