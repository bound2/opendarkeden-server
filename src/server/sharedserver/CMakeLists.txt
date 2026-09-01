# ============================================================================
# SharedServer 可执行文件
# ============================================================================

set(SHAREDSERVER_SOURCES
    handler/GSAddGuildHandler.cpp
    handler/GSAddGuildMemberHandler.cpp
    handler/GSExpelGuildMemberHandler.cpp
    handler/GSGuildMemberLogOnHandler.cpp
    handler/GSModifyGuildIntroHandler.cpp
    handler/GSModifyGuildMemberHandler.cpp
    handler/GSQuitGuildHandler.cpp
    handler/GSRequestGuildInfoHandler.cpp
    SharedPacketDispatch.cpp
    Guild.cpp
    GuildManager.cpp
    ResurrectLocationManager.cpp
    GameServerPlayer.cpp
    HeartbeatManager.cpp
    GameServerInfoManager.cpp
    GameServerGroupInfoManager.cpp
    GameServerManager.cpp
    SharedServer.cpp
    main.cpp
    StringPool.cpp
)

add_executable(sharedserver ${SHAREDSERVER_SOURCES})

# 编译定义
target_compile_definitions(sharedserver PRIVATE __SHARED_SERVER__)

# 包含目录
target_include_directories(sharedserver PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/Core
    ${CMAKE_SOURCE_DIR}/src/server
    ${CMAKE_SOURCE_DIR}/src/server/database
)

# 链接库
if(APPLE)
    set(EXTRA_LIBS z)
elseif(UNIX)
    set(EXTRA_LIBS nsl z dl)
endif()

target_link_libraries(sharedserver PRIVATE
    Database
    SharedServerPackets
    ServerCore
    Core
    # The wire archive itself (task 2.4) — also reached transitively through
    # Core/SharedServerPackets, named here so the layering is visible.
    de-kernel
    ${MYSQL_LIBRARY}
    Threads::Threads
    ${EXTRA_LIBS}
)
