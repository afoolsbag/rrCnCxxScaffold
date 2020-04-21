//===------------------------------------------------------------*- C++ -*-===//
///
/// \file
/// \brief 脚手架示例工程
///
/// \version 2020-04-21
/// \since 2018-04-02
/// \authors zhengrr
/// \copyright Unlicense
///
//===----------------------------------------------------------------------===//

#include <cstdlib>
#include <iostream>
#include <locale>

#include <boost/filesystem/path.hpp>

#include <boost/asio.hpp>
#include <rrspdlog/rrspdlog.hxx>

#include "application_options.hxx"
#include "configuration.hxx"
#include "tcp_server.hxx"

using namespace std;

constexpr char figlet_logo[] {
    R"(           _____       _____            _____            __  __      _     _ )" "\n"
    R"(          /  __ \     /  __ \          /  ___|          / _|/ _|    | |   | |)" "\n"
    R"( _ __ _ __| /  \/_ __ | /  \/_  ____  _\ `--.  ___ __ _| |_| |_ ___ | | __| |)" "\n"
    R"(| '__| '__| |   | '_ \| |   \ \/ /\ \/ /`--. \/ __/ _` |  _|  _/ _ \| |/ _` |)" "\n"
    R"(| |  | |  | \__/\ | | | \__/\>  <  >  </\__/ / (_| (_| | | | || (_) | | (_| |)" "\n"
    R"(|_|  |_|   \____/_| |_|\____/_/\_\/_/\_\____/ \___\__,_|_| |_| \___/|_|\__,_|)" "\n"};

int main(int argc, char *argv[]) noexcept
{
    // 配置
    const application_options opts = [&argc, &argv]() noexcept -> application_options {
        try {
            return application_options {argc, argv};
        } catch (const exception &e) {
            cerr << "Fatal error, parse configurations failed: " << e.what() << '\n';
            cerr << "The program will be closed.";
            exit(EXIT_FAILURE);
        } catch (...) {
            cerr << "Fatal error, parse configurations failed with non-standard exception.\n";
            cerr << "The program will be closed.";
            exit(EXIT_FAILURE);
        }
    }();

    // 日志
    try {
        rrspdlog::initialize_as_default(opts->logs_directory.c_str());
    } catch (const exception &e) {
        cerr << "Fatal error, initialize logger failed: " << e.what() << '\n';
        cerr << "The program will be closed.";
        return EXIT_FAILURE;
    } catch (...) {
        cerr << "Fatal error, initialize logger failed with non-standard exception.\n";
        cerr << "The program will be closed.";
        return EXIT_FAILURE;
    }

    try {
        // 本地化环境
        locale::global([]() noexcept -> locale {
            try { return locale {".UTF-8"}; } catch (...) { /*just pass*/ }
            try { return locale {"C.UTF-8"}; } catch (...) { /*just pass*/ }
            return locale::classic();
        }());
#if defined(BOOST_FILESYSTEM_PATH_HPP) && 0 /*BUG!*/
        // 本地化环境：boost::filesystem::path
        // see https://boost.org/doc/libs/master/libs/locale/default_encoding_under_windows.html
        boost::filesystem::path::imbue(locale {});
#endif

        // 调试选项
        if (opts->debug) {
            opts.print_options(cout);
#ifndef NDEBUG
            opts.launch_debugger();
#endif
        }

        // 帮助选项
        if (opts->help) {
            cout << figlet_logo;
            opts.print_help(cout);
            return EXIT_SUCCESS;
        }

        // 版本选项
        if (opts->version) {
            cout << "Version " << project_version << ".\n";
            return EXIT_SUCCESS;
        }

        // 业务逻辑
        SPDLOG_INFO("Use {} locale.", locale {}.name());
        SPDLOG_INFO("Use {} locale.", setlocale(LC_ALL, nullptr));

        boost::asio::io_context io_context;
        tcp_server server {io_context, opts->export_port};
        io_context.run();

        return EXIT_SUCCESS;

    } catch (const exception &e) {
        SPDLOG_CRITICAL("Caught standard exception in main function: {}", e.what());
        return EXIT_FAILURE;

    } catch (...) {
        SPDLOG_CRITICAL("Caught non-standard exception in main function.");
        return EXIT_FAILURE;

    }
}
