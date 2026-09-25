//////////////////////////////////////////////////////////////////////////////
// Filename    : DatabaseManager.h
// Written By  : elca
// Description : Database manager
//////////////////////////////////////////////////////////////////////////////

#ifndef __DATABASE_MANAGER_H__
#define __DATABASE_MANAGER_H__

#include <shared_mutex>
#include <unordered_map>

#include "Connection.h"
#include "Exception.h"
#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class DatabaseManager;
//
// Each worker registers its own connections, keyed by its thread id, when
// it starts, while the threads already running look theirs up for every
// statement. The per-thread and per-world tables are guarded by
// m_TablesMutex: a lookup takes it shared, a registration exclusive, and it
// is held for the map access alone, so nothing is ever locked under it and
// any thread may look its connection up at any moment. It guards the
// tables, not the connections they point to: a connection is still used
// only by the thread that registered it.
//////////////////////////////////////////////////////////////////////////////

class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();

public:
    void init();
    // Register the connection the thread `TID` reaches through
    // getConnection(const string&) / getDistConnection(). A second
    // registration under the same id throws DuplicatedException and
    // leaves the first in place.
    void addConnection(int TID, Connection* pConnection);
    void addDistConnection(int TID, Connection* pConnection);

    // The calling thread's game-database connection (the one it registered
    // with addConnection), or the default connection built from the DB_*
    // block for a thread that registered none. The argument is ignored:
    // every name reaches that same connection, so a table of another
    // schema is reached by naming its schema in the statement, never by
    // naming a connection here.
    Connection* getConnection(const string& ip);
    Connection* getDistConnection(const string& ip);
    //	Connection* getPCRoomConnection(const string& ip) ;
    Connection* getUserInfoConnection(void) {
        return m_pUserInfoConnection;
    }
    // For processes that do not run init() (the integration tests): the
    // USERINFO connection the repositories reach through
    // getUserInfoConnection(). Takes ownership like init() does and
    // frees a previously set one (defined in the .cpp for SAFE_DELETE).
    void setUserInfoConnection(Connection* pConnection);
    // Same purpose for the world-default connection getConnection(int)
    // falls through to in the gameserver (init() opens it from the
    // WorldDBInfo row with WorldID = 0). Frees a previously set one like
    // the setter above, but the destructor does not free this member, so
    // the last one set is never deleted.
    void setWorldDefaultConnection(Connection* pConnection);
    void executeDummyQuery(Connection* pConnection);

    //--------------------------------------------------------------------
    // * elca's NOTE
    // The login server needs to find out where a character's data lives,
    // so it looks the DB location up.
    // The main DB must hold a table that contains the DB locations.
    // It could be derived from GameServerIP, but a separate DB and GameServer
    // is allowed for, for extensibility.
    //--------------------------------------------------------------------

    //--------------------------------------------------------------------
    // * elca's NOTE
    // Each game server only needs to know the parent/main DB.
    // The main DB server's world id is fixed at 0 by convention,
    // so queries to the main DB server pass 0 as the argument.
    // A game server only needs to hold that connection 0.
    // To avoid any risk of bugs, take care never to open
    // another connection.
    // Queries on an ordinary game server are split per thread,
    // so there is nothing to watch out for there.
    //--------------------------------------------------------------------
    Connection* getConnection(int TID);

private:
    // DB connection held per thread
    unordered_map<int, Connection*> m_Connections;

    // Distribute DB connection held per thread
    unordered_map<int, Connection*> m_DistConnections;

    unordered_map<int, Connection*> m_WorldConnections;


    // DB connection held per world

    // The default DB connection, created first
    Connection* m_pDefaultConnection;

    // The default connection to the topmost DB, created first
    Connection* m_pWorldDefaultConnection;

    // DB connection for user statistics
    Connection* m_pUserInfoConnection;

    Connection* m_pDistConnection;

    // Guards m_Connections, m_DistConnections and m_WorldConnections (see
    // the class comment). A leaf: held for one map access, never while
    // another lock is taken or a statement runs.
    mutable std::shared_mutex m_TablesMutex;
};

#endif
