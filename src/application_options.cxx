/// \copyright Unlicense

#include "application_options.hxx"

#include <fstream>
#include <sstream>
#ifdef _WIN32
#include <Windows.h>
#endif

#include <boost/algorithm/string.hpp>
#include <boost/dll.hpp>
#include <boost/optional/optional_io.hpp>

using namespace std;
using namespace boost::dll;
using namespace boost::program_options;

application_options::application_options(int argc, const char *const argv[])
{
    const auto default_config_file = program_location().replace_extension(".cfg");
    const auto default_environment_prefix = program_location().stem().string() + "_";
    general_options_.add_options()
        ("help,?", bool_switch(&entries_.help)->default_value(false), "Print Help (this message) and exit")
        ("version", bool_switch(&entries_.version)->default_value(false), "Print version information and exit");
    common_options_.add_options()
        ("debug,!", bool_switch(&entries_.debug)->default_value(false), "Enable debugging");
    hidden_options_.add_options()
        ("configuration-file", value(&entries_.configuration_file)->default_value(default_config_file), "Specify configuration file")
        ("environment-prefix", value(&entries_.environment_prefix)->default_value(default_environment_prefix), "Specify environment prefix");
#//=============================================================================
#//
    const auto default_logs_directory = program_location().replace_extension(".logs");
    common_options_.add_options()
        ("logs-directory", value(&entries_.logs_directory)->default_value(default_logs_directory), "Specify logs directory")
        ("tcp-export-port,t", value(&entries_.tcp_export_port)->default_value(9), "Specify TCP server listening port")
        ("http-export-port,h", value(&entries_.http_export_port)->default_value(80), "Specify HTTP server listening port");
    hidden_options_.add_options()
        ("parameters", value(&entries_.parameters)->composing(), "Add some parameter(s)");
#//
#//-----------------------------------------------------------------------------
    parse_from_command_lice(argc, argv);
    parse_from_configuration_file(entries_.configuration_file);
    parse_from_environment(entries_.environment_prefix);
}

void application_options::print_help(std::ostream &os) const
{
    const auto program_name = program_location().filename().string();

    os << '\n';
#//=============================================================================
#//
    os << "Usage: \n";
    os << '\n';
    os << "  " << program_name << " [option]...\n";
#//
#//-----------------------------------------------------------------------------
    os << '\n';
    options_description desc {"Options"};
    desc.add(general_options_).add(common_options_);
    os << desc;
    os << '\n';
}

void application_options::print_options(std::ostream &os) const
{
    os << '\n';
    os << "Debugging: \n";
    os << '\n';
    os << "  help               = " << entries_.help << '\n';
    os << "  debug              = " << entries_.debug << '\n';
    os << "  version            = " << entries_.version << '\n';
    os << "  configuration-file = " << entries_.configuration_file << '\n';
    os << "  environment-prefix = " << entries_.environment_prefix << '\n';
#//=============================================================================
#//
    os << "  logs-directory     = " << entries_.logs_directory << '\n';
    os << "  tcp-export-port    = " << entries_.tcp_export_port << '\n';
    os << "  http-export-port   = " << entries_.http_export_port << '\n';
    os << "  parameters         = "; for (const auto &e : entries_.parameters) os << e << ' '; os << '\n';
#//
#//-----------------------------------------------------------------------------
    os << '\n';
}

void application_options::parse_from_command_lice(int argc, const char *const argv[])
{
    options_description opts;
    opts.add(general_options_).add(common_options_).add(hidden_options_);
#//=============================================================================
#//
    positional_options_description pos;
    pos.add("parameters", -1);
#//
#//-----------------------------------------------------------------------------
    store(command_line_parser {argc, argv}.options(opts).positional(pos).run(), variables_map_);
    notify(variables_map_);
}

void application_options::parse_from_configuration_file(const boost::filesystem::path &configuration_file)
{
    if (!exists(configuration_file))
        ofstream {configuration_file.c_str()};

    ifstream ifs {configuration_file.c_str()};
    if (!ifs) {
        ostringstream oss;
        oss << "read configuration file " << configuration_file << " failed";
        throw runtime_error {oss.str()};
    }

    options_description opts;
    opts.add(common_options_).add(hidden_options_);

    store(parse_config_file(ifs, opts), variables_map_);
    notify(variables_map_);
}

void application_options::parse_from_environment(const string &environment_prefix)
{
    const auto prefix_lower = boost::to_lower_copy(environment_prefix);
    const auto prefix_length = environment_prefix.length();

    const auto variables_map = [&](const string &environment_variable) -> string {
        auto variable = boost::to_lower_copy(environment_variable);
        if (variable.compare(0, prefix_length, prefix_lower) != 0)
            return "";
        boost::erase_head(variable, static_cast<int>(prefix_length));
        boost::replace_all(variable, "_", "-");
        return variable;
    };

    options_description opts;
    opts.add(common_options_).add(hidden_options_);

    store(parse_environment(opts, variables_map), variables_map_);
    notify(variables_map_);
}

void application_options::launch_debugger()
{
#ifdef _WIN32
    WCHAR acBuf[MAX_PATH];
    if (GetSystemDirectoryW(acBuf, _countof(acBuf)) == 0)
        return;
    const auto vsjitdebugger = boost::filesystem::path {acBuf} / L"vsjitdebugger.exe";
    auto cmd = vsjitdebugger.wstring() + L" -p " + to_wstring(GetCurrentProcessId());

    STARTUPINFOW stStartupInfo {};
    stStartupInfo.cb = sizeof(stStartupInfo);

    PROCESS_INFORMATION stProcInfo {};

    if (!CreateProcessW(reinterpret_cast<LPCWSTR>(vsjitdebugger.wstring().data()),
                        reinterpret_cast<LPWSTR>(cmd.data()),
                        nullptr,
                        nullptr,
                        FALSE,
                        0,
                        nullptr,
                        nullptr,
                        &stStartupInfo,
                        &stProcInfo))
        return;
    CloseHandle(stProcInfo.hThread);
    CloseHandle(stProcInfo.hProcess);

    while (!IsDebuggerPresent())
        Sleep(1000 / 25);

    DebugBreak();
#endif
}
