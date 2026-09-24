//////////////////////////////////////////////////////////////////////////////
// Filename    : ConnectionSettings.h
// Description : The configuration keys one database connection is built from.
//////////////////////////////////////////////////////////////////////////////

#ifndef __CONNECTION_SETTINGS_H__
#define __CONNECTION_SETTINGS_H__

#include <string>

#include "Types.h"

class Properties;

namespace de {

//////////////////////////////////////////////////////////////////////////////
//
// Each database a server talks to is configured under a prefix of its own --
// DB for the game database, UI_DB for the account database -- with the same
// five keys behind it. connectionSettings reads one such block, so the five
// keys are named once and a connection cannot end up built from two
// different blocks.
//
// HOST, DB, USER and PASSWORD are required and a missing one throws, as the
// servers have nothing to connect with without them. PORT is optional: left
// out it stays zero, which is what Connection passes to MySQL to mean the
// driver's default port.
//
//////////////////////////////////////////////////////////////////////////////

struct ConnectionSettings {
    std::string host;
    std::string db;
    std::string user;
    std::string password;
    uint port = 0;
};

ConnectionSettings connectionSettings(const Properties& config, const std::string& prefix);

} // namespace de

#endif
