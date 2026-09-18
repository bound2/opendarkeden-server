//////////////////////////////////////////////////////////////////////////////
// Filename    : LogClient.h
// Written by  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __LOGCLIENT_H__
#define __LOGCLIENT_H__

#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// LogType
// Log kinds can be split into groups by importance.
// Right now those groups are split in steps of 1000.
// Setting LogClient's LogLevel in steps of 1000 makes it possible
// to log one group and leave the rest unlogged.
//////////////////////////////////////////////////////////////////////////////

enum LogType {
    LOG_SYSTEM = 0,        // system log
    LOG_SYSTEM_ERROR = 10, // system error log

    LOG_GAMESERVER = 20,       // game server message
    LOG_GAMESERVER_ERROR = 30, // game server error message

    LOG_LOGINSERVER = 40,       // login server message
    LOG_LOGINSERVER_ERROR = 50, // login server error message

    LOG_SHAREDSERVER = 60,       // shared server message
    LOG_SHAREDSERVER_ERROR = 70, // shared server error message

    LOG_CGCONNECT = 80, // login to the game server

    LOG_PICKUP_ITEM = 1010, // picking an item up
    LOG_DROP_ITEM = 1020,   // dropping an item

    LOG_PICKUP_MONEY = 1030, // picking money up
    LOG_DROP_MONEY = 1040,   // dropping money

    LOG_USE_ITEM = 1050,     // using an item
    LOG_CREATE_ITEM = 1060,  // creating an item (holy water or a bomb)
    LOG_REPAIR_ITEM = 1065,  // an item was repaired
    LOG_DESTROY_ITEM = 1070, // an item broke when its durability ran out

    LOG_BUY_ITEM = 1080,          // buying an item in a shop
    LOG_SELL_ITEM = 1090,         // selling an item in a shop
    LOG_SHOP_CREATE_ITEM = 1100,  // an item was created in a shop
    LOG_SHOP_DESTROY_ITEM = 1110, // an item disappeared from a shop

    LOG_DROP_ITEM_DIE = 1120,   // dropping an item on death
    LOG_DROP_ITEM_MORPH = 1130, // dropping an item while transforming
    LOG_LOOT_CORPSE = 1140,     // taking an item out of a corpse

    LOG_STASH_ADD_ITEM = 1150,     // putting an item into the stash
    LOG_STASH_REMOVE_ITEM = 1160,  // taking an item out of the stash
    LOG_STASH_ADD_MONEY = 1170,    // putting money into the stash
    LOG_STASH_REMOVE_MONEY = 1180, // taking money out of the stash

    LOG_USE_BONUS_POINT = 1190, // using a bonus point

    LOG_TRADE = 1200, // exchange

    LOG_KILL_PLAYER = 2000, // a player killed a player

    LOG_BLOODDRAIN = 2010,   // drained blood
    LOG_BLOODDRAINED = 2020, // had blood drained

    LOG_SLAYER_TO_VAMPIRE = 2030, // from Slayer to Vampire
    LOG_VAMPIRE_TO_SLAYER = 2040, // from Vampire to Slayer
    LOG_HEAL = 2050,              // got healed

    LOG_DEBUG_MSG = 3000, // debugging message

    LOGTYPE_MAX
};


//////////////////////////////////////////////////////////////////////////////
// class LogClient
// The client class for logging. SIGNAL handling is deliberately left out,
// so SIGNAL has to be handled outside the class. Otherwise this process dies
// when the log server dies.
//
// Log level (a larger log level includes the lower levels' logs.)
// 0    : logs only what concerns the server system.
// 1000 : logs the most important parts of the game.
// 2000 : logs the less important parts of the game too.
//////////////////////////////////////////////////////////////////////////////

class LogClient {
    ///// member methods /////

public:
    LogClient(string ip, short port);
    virtual ~LogClient();

public:
    void connect(string ip, short port);
    void disconnect(void);

    void _log(short type, const string& source, const string& target);
    void _log(short type, const string& source, const string& target, const string& content);
    void _log(short type, const string& source, const string& target, const string& content, short ZoneID);

public:
    static int getLogLevel(void) {
        return m_LogLevel;
    }
    static void setLogLevel(int level) {
        m_LogLevel = level;
    }

    ///// member data /////

protected:
    int m_Socket;
    bool m_bConnected;
    long long m_Sent;

    static int m_LogLevel;
};

//////////////////////////////////////////////////////////////////////////////
// globals
//////////////////////////////////////////////////////////////////////////////

// The process's one log client, owned by LogClient.cpp. openLogClient()
// creates it; logClient() is null until then.
void openLogClient(const string& ip, short port);
LogClient* logClient();

void log(short type, const string& source, const string& target);
void log(short type, const string& source, const string& target, const string& content);
void log(short type, const string& source, const string& target, const string& content, short ZoneID);


#endif
