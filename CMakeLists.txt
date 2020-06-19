#            _____       _____            _____            __  __      _     _
#           /  __ \     /  __ \          /  ___|          / _|/ _|    | |   | |
#  _ __ _ __| /  \/_ __ | /  \/_  ____  _\ `--.  ___ __ _| |_| |_ ___ | | __| |
# | '__| '__| |   | '_ \| |   \ \/ /\ \/ /`--. \/ __/ _` |  _|  _/ _ \| |/ _` |
# | |  | |  | \__/\ | | | \__/\>  <  >  </\__/ / (_| (_| | | | || (_) | | (_| |
# |_|  |_|   \____/_| |_|\____/_/\_\/_/\_\____/ \___\__,_|_| |_| \___/|_|\__,_|
# zhengrr
# 2019-11-18 – 2020-06-17
# Unlicense

cmake_minimum_required(VERSION 3.12)
cmake_policy(VERSION 3.12)

# 额外模块
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake/Modules")
include(rrCMake)

# 项目配置
project(
  rrCnCxxScaffold
  LANGUAGES C CXX)
rr_product_extra(
  TIME_AS_VERSION
  AUTHORS   "zhengrr"
  LICENSE   "Unlicense")

# 全局编译选项
add_compile_options_ex(
  RECOMMENDED_WARNING_LEVEL
  MULTIPLE_PROCESSES
  UTF-8
  VISIBLE_DEFAULT_HIDDEN
  WARNING_AS_ERROR)

# 依赖：boost、ms-gsl、spdlog
include_conan_script()
conan_cmake_run(
  REQUIRES    boost/1.73.0
              ms-gsl/3.0.1
              spdlog/1.6.1
  BASIC_SETUP CMAKE_TARGETS
  BUILD       missing)

# 依赖：rrspdlog
find_package(rrspdlog CONFIG HINTS "${PRODUCT_SOURCE_DIR}/deps/rrCnCxx/include/rrspdlog")

# 程序：${PROJECT_NAME}
configure_file("src/configuration.hxx.in" "src/configuration.hxx" @ONLY)
aux_source_directory_con("src"                             zSrc       RECURSE                         PREFIX "src")
aux_source_directory_con("res"                             zRes       RECURSE EXTENSIONS ".ico" ".rc" PREFIX "res")
aux_source_directory_con("${CMAKE_CURRENT_BINARY_DIR}/src" zGenerated RECURSE                         PREFIX "generated")
add_executable_con(
  "${PROJECT_NAME}"   ${zSrc} ${zRes} ${zGenerated}
                      "README.md"
  PROPERTIES          CXX_STANDARD          17 #20
                      CXX_STANDARD_REQUIRED ON
                      BUILD_RPATH           "$ORIGIN"
                      INSTALL_RPATH         "$ORIGIN"
  COMPILE_DEFINITIONS PRIVATE $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=0x0A00>
                              $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
                              $<$<CXX_COMPILER_ID:MSVC>:_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS>
  COMPILE_OPTIONS     PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/wd4702>
  INCLUDE_DIRECTORIES PRIVATE "src"
                              "${CMAKE_CURRENT_BINARY_DIR}/src"
                              "${CMAKE_CURRENT_BINARY_DIR}/res"
  LINK_LIBRARIES      PRIVATE rrspdlog
                              CONAN_PKG::boost
                              $<$<PLATFORM_ID:Linux>:dl>
                              CONAN_PKG::ms-gsl
                              $<$<PLATFORM_ID:Linux>:stdc++fs>)

# 安装
install(
  FILES       "UNLICENSE.txt"
  DESTINATION ".")

# 安装包
# https://cmake.org/cmake/help/latest/module/CPack.html
set(CPACK_PACKAGE_NAME          "${PRODUCT_NAME}")
math(EXPR nTmp                  "${PRODUCT_VERSION_MAJOR} % 100")
set(CPACK_PACKAGE_VERSION_MAJOR "${nTmp}")
math(EXPR nTmp                  "${PRODUCT_VERSION_MINOR} % 100")
set(CPACK_PACKAGE_VERSION_MINOR "${nTmp}")
math(EXPR nTmp                  "${PRODUCT_VERSION_PATCH} % 100")
set(CPACK_PACKAGE_VERSION_PATCH "${nTmp}")
set(CPACK_PACKAGE_DESCRIPTION   "${PRODUCT_DESCRIPTION}")
set(CPACK_PACKAGE_HOMEPAGE_URL  "${PRODUCT_HOMEPAGE_URL}")
set(CPACK_PACKAGE_VENDOR        "${PRODUCT_AUTHORS}")

set(CPACK_RESOURCE_FILE_LICENSE "${PRODUCT_SOURCE_DIR}/UNLICENSE.txt")
set(CPACK_RESOURCE_FILE_README  "${PRODUCT_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_EXECUTABLES   "${PROJECT_NAME}" "${PROJECT_NAME}")
set(CPACK_PACKAGE_CHECKSUM      SHA256)

if(UNIX)
  # https://cmake.org/cmake/help/latest/cpack_gen/archive.html
  set(CPACK_GENERATOR           TGZ)
elseif(WIN32)
  # https://cmake.org/cmake/help/latest/cpack_gen/wix.html
  set(CPACK_GENERATOR           WIX)
  set(CPACK_WIX_UPGRADE_GUID    "D5F43868-1EAB-4B33-A204-9A69E95B807D")
  set(CPACK_WIX_CULTURES        "zh-CN")
endif()

include(CPack)
