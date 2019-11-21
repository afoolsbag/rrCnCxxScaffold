#            _____       _____            _____            __  __      _     _
#           /  __ \     /  __ \          /  ___|          / _|/ _|    | |   | |
#  _ __ _ __| /  \/_ __ | /  \/_  ____  _\ `--.  ___ __ _| |_| |_ ___ | | __| |
# | '__| '__| |   | '_ \| |   \ \/ /\ \/ /`--. \/ __/ _` |  _|  _/ _ \| |/ _` |
# | |  | |  | \__/\ | | | \__/\>  <  >  </\__/ / (_| (_| | | | || (_) | | (_| |
# |_|  |_|   \____/_| |_|\____/_/\_\/_/\_\____/ \___\__,_|_| |_| \___/|_|\__,_|
# zhengrr
# 2019-11-18 – 2019-11-21
# Unlicense

cmake_minimum_required(VERSION 3.12)
cmake_policy(VERSION 3.12)

# 额外模块
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake/Modules")
include(rrCMake)

# 项目配置
product_ex(
  rrCnCxxScaffold
  TIME_AS_VERSION
  LANGUAGES C CXX
  AUTHORS   "zhengrr"
  LICENSE   "Unlicense")

# 全局编译选项
add_compile_options_ex(
  RECOMMENDED_WARNING_LEVEL
  MULTIPLE_PROCESSES
  UTF-8
  VISIBLE_DEFAULT_HIDDEN
  WARNING_AS_ERROR)

# conan
include_conan_script()
conan_cmake_run(
  REQUIRES    boost/1.71.0@conan/stable
              gsl_microsoft/2.0.0@bincrafters/stable
              spdlog/1.3.1@bincrafters/stable
  BASIC_SETUP CMAKE_TARGETS
  BUILD       missing)

# 依赖：rrCnCxx
aux_source_directory_con("deps/rrCnCxx/include/rrspdlog" zWrapper RECURSE PREFIX "deps/rrCnCxx/include/rrspdlog")
add_library_ex(
  rrspdlog            INTERFACE
  COMPILE_FEATURES    INTERFACE cxx_std_17
  INCLUDE_DIRECTORIES INTERFACE "deps/rrCnCxx/include"
  LINK_LIBRARIES      INTERFACE CONAN_PKG::spdlog
  SOURCES             INTERFACE ${zWrapper})

# 程序：${PROJECT_NAME}
configure_file("src/configuration.hxx.in" "src/configuration.hxx" @ONLY)
aux_source_directory_con("src"                             zSrc               PREFIX "src")
aux_source_directory_con("${CMAKE_CURRENT_BINARY_DIR}/src" zGenerated RECURSE PREFIX "generated")
add_executable_con(
  "${PROJECT_NAME}"   ${zSrc} ${zGenerated}
                      "README.md"
  PROPERTIES          CXX_STANDARD          20
                      CXX_STANDARD_REQUIRED ON
  COMPILE_DEFINITIONS PRIVATE $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=0x0A00>
                              $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
                              $<$<CXX_COMPILER_ID:MSVC>:_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS>
  INCLUDE_DIRECTORIES PRIVATE "src"
                              "${CMAKE_CURRENT_BINARY_DIR}/src"
  LINK_LIBRARIES      PRIVATE rrspdlog
                              CONAN_PKG::boost
                              CONAN_PKG::gsl_microsoft
                              $<$<PLATFORM_ID:Linux>:stdc++fs>)

# 安装
install(
  FILES       "UNLICENSE.txt"
  DESTINATION ".")

# 打包
