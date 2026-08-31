cmake_minimum_required(VERSION 3.16)
project(DarkEden VERSION 1.0 LANGUAGES CXX)

# ============================================================================
# 全局设置
# ============================================================================
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# ============================================================================
# 全局编译选项
# ============================================================================
# 基础宏定义 - 所有目标都需要
if(APPLE)
    add_compile_definitions(__APPLE__ _REENTRANT)
elseif(UNIX)
    add_compile_definitions(__LINUX__ _REENTRANT)
endif()

# 警告设置
set(WARNING_FLAGS 
    -Wall
    -Wno-format 
    -Wno-format-security
)
add_compile_options(${WARNING_FLAGS})

# Build type flags.
#
# NDEBUG is deliberately NOT defined in the optimized configurations. The code
# base uses it for far more than assertions, and none of it is safe to drop:
#
#   * Assert(expr) expands to ((void)0) under NDEBUG, but ~165 call sites put
#     the work inside it, e.g. Assert(pTree->GetAttribute("class", iClass)) in
#     GQuestGiveItemElement.cpp - the attribute would never be read.
#   * __BEGIN_TRY / __END_CATCH / __END_CATCH_NO_RETHROW (Exception.h) also
#     collapse to ((void)0). 344 call sites use __END_CATCH_NO_RETHROW to
#     swallow exceptions, mostly in client packet handlers, so under NDEBUG
#     those exceptions would escape instead.
#
# Only the non-NDEBUG configuration has ever been exercised; keep it that way
# and let the optimizer do the rest.
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os")

# ============================================================================
# 查找依赖
# ============================================================================
find_package(Threads REQUIRED)

# MySQL
if(APPLE)
    find_path(MYSQL_INCLUDE_DIR mysql/mysql.h
        PATHS
        /opt/homebrew/include
        /opt/homebrew/Cellar/mysql-client/*/include
        /opt/homebrew/Cellar/mariadb-connector-c/*/include
        /usr/local/include
        NO_DEFAULT_PATH
    )
    find_library(MYSQL_LIBRARY mysqlclient
        PATHS
        /opt/homebrew/lib
        /opt/homebrew/Cellar/mysql-client/*/lib
        /opt/homebrew/Cellar/mariadb-connector-c/*/lib
        /opt/homebrew/Cellar/mariadb-connector-c/*/lib/mariadb
        /usr/local/lib
    )
elseif(UNIX)
    find_path(MYSQL_INCLUDE_DIR mysql/mysql.h
        PATHS /usr/include /usr/local/include
        NO_DEFAULT_PATH
    )
    find_library(MYSQL_LIBRARY mysqlclient
        PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu
    )
endif()

# Lua 5.1 / LuaJIT
find_package(PkgConfig)
if(PkgConfig_FOUND)
    pkg_check_modules(LUA51 lua5.1)
endif()
if(NOT LUA51_FOUND)
    if(APPLE)
        find_path(LUA51_INCLUDE_DIRS lua.h
            PATHS
            /opt/homebrew/include/luajit-2.1
            /opt/homebrew/include/lua5.1
            /usr/local/include/lua5.1
        )
        find_library(LUA51_LIBRARIES luajit
            PATHS /opt/homebrew/lib /usr/local/lib
        )
        if(NOT LUA51_LIBRARIES)
            find_library(LUA51_LIBRARIES lua5.1
                PATHS /opt/homebrew/lib /usr/local/lib
            )
        endif()
    else()
        find_path(LUA51_INCLUDE_DIRS lua.h PATHS /usr/include/lua5.1 /usr/local/include/lua5.1)
        find_library(LUA51_LIBRARIES lua5.1 PATHS /usr/lib /usr/local/lib)
    endif()
endif()

# XML parsing: vendored tinyxml2 (third_party/tinyxml2, pinned at 10.0.0).
#
# Replaces xerces-c. The five data/*.xml files declare no schema, DTD or
# namespace, so none of xerces' validation machinery had anything to act on,
# and it was the project's only C++-ABI-sensitive dependency -- which made it
# the one thing standing between this build and an alternative toolchain
# (libc++ vs libstdc++ would clash on the XMLException catch in SXml.cpp).
# Vendoring keeps the version pinned in-tree instead of taking whatever the
# host distribution happens to ship.
add_library(tinyxml2 STATIC third_party/tinyxml2/tinyxml2.cpp)
target_include_directories(tinyxml2 PUBLIC third_party/tinyxml2)
set_target_properties(tinyxml2 PROPERTIES POSITION_INDEPENDENT_CODE ON)

# ============================================================================
# 包含目录（全局）
# ============================================================================
include_directories(
    ${CMAKE_SOURCE_DIR}/src/Core
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/database
    ${MYSQL_INCLUDE_DIR}
)

include_directories(third_party/tinyxml2)

# ============================================================================
# 子目录
# ============================================================================
option(DARKEDEN_BUILD_TESTS "Build the wire-contract test suite (tests/)" OFF)

add_subdirectory(src/Core)
add_subdirectory(src/server)

if(DARKEDEN_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ============================================================================
# 安装规则
# ============================================================================
install(TARGETS gameserver loginserver sharedserver
    RUNTIME DESTINATION bin
)
