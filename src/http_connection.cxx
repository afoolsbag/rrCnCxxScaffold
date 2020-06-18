/// \copyright Unlicense

#include "http_connection.hxx"

#include <chrono>
#include <map>
#include <string>

#include <boost/bind/bind.hpp>
#include <boost/core/ignore_unused.hpp>
#include <rrspdlog/rrspdlog.hxx>

#include "configuration.hxx"

using namespace std;
namespace asio = boost::asio;
namespace beast = boost::beast;

http_connection::http_connection(asio::io_context &io_context) :
    socket_ {io_context}, deadline_ {socket_.get_executor(), chrono::seconds{60}}
{}

void http_connection::start()
{
    remote_address_ = socket_.remote_endpoint().address().to_string();
    remote_port_ = socket_.remote_endpoint().port();
    SPDLOG_INFO("HTTP server: New connection from {}:{}.", remote_address_, remote_port_);
    async_read_request();
    async_wait_deadline();
}

asio::ip::tcp::socket &http_connection::socket() noexcept
{
    return socket_;
}

http_connection::~http_connection() noexcept
{
    if (!remote_address_.empty()) {
        assert(remote_port_ != 0);
        SPDLOG_INFO("HTTP server: Disconnect with {}:{}.", remote_address_, remote_port_);
    }
}

void http_connection::async_read_request()
{
    beast::http::async_read(socket_, buffer_, request_, boost::bind(
        &http_connection::on_request_read, shared_from_this(), asio::placeholders::error, asio::placeholders::bytes_transferred));
}

void http_connection::on_request_read(const boost::beast::error_code &err, size_t bytes) noexcept
{
    boost::ignore_unused(bytes);

    try {
        if (!err) {
            response_.version(request_.version());
            response_.keep_alive(false);
            response_.set(beast::http::field::server, PROJECT_NAME "/" PROJECT_VERSION);

            switch (request_.method()) {
#if 0
            // 创建 CREATE
            case beast::http::verb::post:
                break;
#endif

            // 读取 READ
            case beast::http::verb::get: {
                response_.set(beast::http::field::content_type, "text/html");
                auto &&os = beast::ostream(response_.body());
                os << "<!DOCTYPE html>\n";
                os << "<html>\n";
                os << "<head><link rel='icon' href='data:;base64,='></head>\n";
                os << "<body>THE METHOD (" << request_.method_string() << ") NOT YET IMPLEMENTED.</body>\n";
                os << "</html>\n";
                response_.result(beast::http::status::not_implemented);
                break;
            }

#if 0
            // 读取消息头 READ HEAD (WITHOUT BODY)
            case beast::http::verb::head:
                break;
#endif

#if 0
            // 全量更新 FULL UPDATE
            case beast::http::verb::put:
                break;
#endif

#if 0
            // 增量更新 INCREMENTAL UPDATE
            case beast::http::verb::patch:
                break;
#endif

#if 0
            // 删除 DELETE
            case beast::http::verb::delete_:
                break;
#endif

            // 测试 TRACE
            case beast::http::verb::trace: {
                const auto &target = request_.target();

                // 解析查询参数
                map<string, string> query_args;
                const size_t query_mark_pos = target.find('?');
                if (query_mark_pos != beast::string_view::npos) {
                    // /path?key1=value1&key2=value2&key3=val3...
                    //      ^    ^      ^
                    //     pos1 pos2   pos3
                    size_t pos1 = query_mark_pos;
                    while (pos1 != beast::string_view::npos) {
                        const size_t pos2 = target.find('=', pos1 + 1);
                        const size_t pos3 = target.find('&', pos1 + 1);
                        if (pos2 < pos3) {
                            string key {target.substr(pos1 + 1, pos2 - pos1 - 1)};
                            string value {target.substr(pos2 + 1, pos3 - pos2 - 1)};
                            query_args.insert(pair(move(key), move(value)));
                        } else {
                            string key {target.substr(pos1 + 1, pos3 - pos1 - 1)};
                            query_args.insert(pair(move(key), string {}));
                        }
                        pos1 = pos3;
                    }
                }

                response_.set(beast::http::field::content_type, "text/plain");
                auto &&os = beast::ostream(response_.body());
                os << "TRACE RESPONSE BODY SHOWS REQUEST INFORMATION" << '\n';
                os << '\n';
                os << "method: " << request_.method_string() << '\n';
                os << '\n';
                os << "target: " << request_.target() << '\n';
                for (const auto &pair : query_args)
                    os << "        \"" << pair.first << "\" -> \"" << pair.second << "\"\n";
                os << '\n';
                os << "version: " << request_.version() << '\n';
                os << '\n';
                os << "head:\n";
                os << "--------------------------------------------------------------------------------\n";
                os << request_.base();
                os << '\n';
                os << "body:\n";
                os << "--------------------------------------------------------------------------------\n";
                os << request_.body();
                response_.result(beast::http::status::ok);
                break;
            }

            // 其它 OTHERS
            default:
                response_.set(beast::http::field::content_type, "text/plain");
                beast::ostream(response_.body()) << "THE METHOD (" << request_.method_string() << ") NOT YET IMPLEMENTED.";
                response_.result(beast::http::status::not_implemented);
                break;
            }

            async_write_response();

        } else if (err == asio::error::operation_aborted) {
            /*PASS*/;
        } else {
            SPDLOG_INFO("HTTP server: Callback {} was invoked with error {} {}.", __func__, err.value(), err.message());
        }

    } catch (const exception &e) {
        SPDLOG_WARN("HTTP server: Caught standard exception in {} callback: {}.", __func__, e.what());

    } catch (...) {
        SPDLOG_WARN("HTTP server: Caught non-standard exception in {} callback.", __func__);
    }
}

void http_connection::async_write_response()
{
    response_.set(beast::http::field::content_length, response_.body().size());
    beast::http::async_write(socket_, response_, boost::bind(
        &http_connection::on_response_written, shared_from_this(), asio::placeholders::error, asio::placeholders::bytes_transferred));
}

void http_connection::on_response_written(const beast::error_code &err, size_t bytes) noexcept
{
    boost::ignore_unused(bytes);

    try {
        if (!err) {
            socket_.shutdown(asio::ip::tcp::socket::shutdown_send);
            deadline_.cancel();
        } else {
            SPDLOG_INFO("HTTP server: Callback {} was invoked with error {}.", __func__, err.message());
        }

    } catch (const exception &e) {
        SPDLOG_WARN("HTTP server: Caught standard exception in {} callback: {}.", __func__, e.what());

    } catch (...) {
        SPDLOG_WARN("HTTP server: Caught non-standard exception in {} callback.", __func__);
    }
}

void http_connection::async_wait_deadline()
{
    deadline_.async_wait(boost::bind(
        &http_connection::when_deadline_expired, shared_from_this(), asio::placeholders::error));
}

void http_connection::when_deadline_expired(const boost::system::error_code &err) noexcept
{
    try {
        if (!err) {
            socket_.close();
        } else if (err == asio::error::operation_aborted) {
            /*PASS*/;
        } else {
            SPDLOG_INFO("HTTP server: Callback {} was invoked with error {}.", __func__, err.message());
        }

    } catch (const exception &e) {
        SPDLOG_WARN("HTTP server: Caught standard exception in {} callback: {}.", __func__, e.what());

    } catch (...) {
        SPDLOG_WARN("HTTP server: Caught non-standard exception in {} callback.", __func__);
    }
}
