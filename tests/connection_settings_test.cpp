// The configuration block one database connection is built from
// (src/server/database/ConnectionSettings.h). DatabaseManager opens the game
// database and the account database from two such blocks, and the account
// database used to take the game database's port; these cases pin each block
// to its own prefix. Properties is a kernel class with a setter, so a
// configuration can be built in memory and no MySQL is involved.

#include <gtest/gtest.h>

#include "ConnectionSettings.h"
#include "Properties.h"

namespace {

// The shipped conf/gameserver.conf layout: two blocks, same server, ports
// named after their own block.
Properties twoBlockConfig() {
    Properties config;
    config.setProperty("DB_HOST", "game.example");
    config.setProperty("DB_PORT", "33061");
    config.setProperty("DB_DB", "DARKEDEN");
    config.setProperty("DB_USER", "scott");
    config.setProperty("DB_PASSWORD", "tiger");

    config.setProperty("UI_DB_HOST", "account.example");
    config.setProperty("UI_DB_PORT", "33062");
    config.setProperty("UI_DB_DB", "USERINFO");
    config.setProperty("UI_DB_USER", "uiscott");
    config.setProperty("UI_DB_PASSWORD", "uitiger");
    return config;
}

} // namespace

TEST(ConnectionSettings, TheGameBlockIsReadFromItsOwnKeys) {
    const de::ConnectionSettings settings = de::connectionSettings(twoBlockConfig(), "DB");

    EXPECT_EQ("game.example", settings.host);
    EXPECT_EQ("DARKEDEN", settings.db);
    EXPECT_EQ("scott", settings.user);
    EXPECT_EQ("tiger", settings.password);
    EXPECT_EQ(33061u, settings.port);
}

// The defect: the account database was reached on the game database's port,
// so an account database of its own could not be reached at all.
TEST(ConnectionSettings, TheAccountBlockTakesItsOwnPortNotTheGameOne) {
    const de::ConnectionSettings settings = de::connectionSettings(twoBlockConfig(), "UI_DB");

    EXPECT_EQ("account.example", settings.host);
    EXPECT_EQ("USERINFO", settings.db);
    EXPECT_EQ("uiscott", settings.user);
    EXPECT_EQ("uitiger", settings.password);
    EXPECT_EQ(33062u, settings.port);
}

// A configuration that never named the block's port keeps working: zero is
// what Connection passes to MySQL to mean the driver's default port.
TEST(ConnectionSettings, AMissingPortIsTheDriverDefault) {
    Properties bare;
    bare.setProperty("UI_DB_HOST", "account.example");
    bare.setProperty("UI_DB_DB", "USERINFO");
    bare.setProperty("UI_DB_USER", "uiscott");
    bare.setProperty("UI_DB_PASSWORD", "uitiger");

    const de::ConnectionSettings settings = de::connectionSettings(bare, "UI_DB");

    EXPECT_EQ("account.example", settings.host);
    EXPECT_EQ(0u, settings.port);

    // The game block's port is not borrowed for it.
    bare.setProperty("DB_PORT", "33061");
    EXPECT_EQ(0u, de::connectionSettings(bare, "UI_DB").port);
}

TEST(ConnectionSettings, AnAddressTheConfigurationNeverGaveThrows) {
    Properties config;
    config.setProperty("UI_DB_HOST", "account.example");

    EXPECT_THROW(de::connectionSettings(config, "UI_DB"), NoSuchElementException);
    EXPECT_THROW(de::connectionSettings(config, "DB"), NoSuchElementException);
}
