//////////////////////////////////////////////////////////////////////////////
// Filename    : DatabaseManager.cpp
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "DatabaseManager.h"

#include <stdio.h>

#include <mutex>

#include "Assert.h"
#include "ConnectionSettings.h"
#include "DB.h"
#include "KernelContext.h"
#include "Properties.h"
#include "Result.h"
#include "Statement.h"
#include "Thread.h"
#include "Timeval.h"
#include "Utility.h"

DatabaseManager::DatabaseManager() {
    __BEGIN_TRY

    m_pDefaultConnection = NULL;
    m_pDistConnection = NULL;
    m_pWorldDefaultConnection = NULL;
    m_pUserInfoConnection = NULL;

    __END_CATCH
}

DatabaseManager::~DatabaseManager() {
    // Every Connection must be deleted.
    unordered_map<int, Connection*>::iterator itr = m_Connections.begin();
    for (; itr != m_Connections.end(); itr++)
        SAFE_DELETE(itr->second);

    // Erase every pair in the hash map.
    m_Connections.clear();

    SAFE_DELETE(m_pDefaultConnection);
    SAFE_DELETE(m_pUserInfoConnection);
}

void DatabaseManager::setUserInfoConnection(Connection* pConnection) {
    SAFE_DELETE(m_pUserInfoConnection);
    m_pUserInfoConnection = pConnection;
}

void DatabaseManager::setWorldDefaultConnection(Connection* pConnection) {
    SAFE_DELETE(m_pWorldDefaultConnection);
    m_pWorldDefaultConnection = pConnection;
}

void DatabaseManager::init() {
    __BEGIN_TRY

    Properties& config = de::kernelContext().config();

    try {
        cout << "--------------------------------------------------" << endl;
        cout << "            Init DatabaseManager " << endl;
        cout << "--------------------------------------------------" << endl;

        // Each database is configured under a prefix of its own, so the game
        // database's address can never be mixed with the account database's.
        const de::ConnectionSettings game = de::connectionSettings(config, "DB");
        m_pDefaultConnection = new Connection(game.host, game.db, game.user, game.password, game.port);
        Assert(m_pDefaultConnection != NULL);

        const de::ConnectionSettings userInfo = de::connectionSettings(config, "UI_DB");
        m_pUserInfoConnection =
            new Connection(userInfo.host, userInfo.db, userInfo.user, userInfo.password, userInfo.port);
        Assert(m_pUserInfoConnection != NULL);


        Statement* pStmt = NULL;
        pStmt = m_pDefaultConnection->createStatement();
        Result* pResult = NULL;

        // time check
        // No more than 1 hour with the database time.
        pResult = pStmt->executeQueryString("SELECT unix_timestamp()");
        if (pResult->next()) {
            time_t tDBTime = pResult->getInt(1);
            time_t tSYSTime = time(0);
            double dbDiff = difftime(tSYSTime, tDBTime);

            if ((int)dbDiff > 3600) {
                // The gap between DB time and server time is more than an hour.
                cout << "======================================================" << endl;
                cout << "!!! Time Check Error !!!" << endl;
                cout << "DB time is " << tDBTime << "and server time is " << tSYSTime << endl;
                cout << "!!! Please check DB server and service server time !!!" << endl;
                cout << "======================================================" << endl;

                throw Error("Time Check Error");
            }
        }

#ifdef __LOGIN_SERVER__
        pResult = pStmt->executeQueryString("SELECT WorldID, Host, DB, User, Password, Port FROM WorldDBInfo");
        cout << "[LOGIN_SERVER] query WorldDBInfo" << endl;
#else
        pResult = pStmt->executeQueryString(
            "SELECT WorldID, Host, DB, User, Password, Port FROM WorldDBInfo WHERE WorldID = 0");
#endif

        while (pResult->next()) {
            cout << "--------------------------------------------------" << endl;
            cout << "--------------------------------------------------" << endl;
            cout << "Connecting....... Another DB Server" << endl;
            cout << "--------------------------------------------------" << endl;
            cout << "--------------------------------------------------" << endl;

            WorldID_t WorldID = pResult->getInt(1);
            string whost = pResult->getString(2);
            string wdb = pResult->getString(3);
            string wuser = pResult->getString(4);
            string wpassword = pResult->getString(5);
            uint port = pResult->getInt(6);

            cout << "Connectiong: " << "WorldID=" << (int)WorldID << ", HOST=" << whost.c_str()
                 << ", DB=" << wdb.c_str() << ", User=" << wuser.c_str() << ", Port=" << port << endl;

            Connection* pConnection = new Connection(whost, wdb, wuser, wpassword, port);
            Assert(pConnection != NULL);

#ifdef __LOGIN_SERVER__
            {
                std::unique_lock lock(m_TablesMutex);
                m_WorldConnections[WorldID] = pConnection;
            }

            addConnection(WorldID, pConnection);

            cout << "WorldID=" << (int)WorldID << ", IP=" << whost.c_str() << endl;
#else
            m_pWorldDefaultConnection = pConnection;
#endif
        }


        SAFE_DELETE(pStmt);
    } catch (SQLConnectException& sce) {
        throw Error(sce.toString());
    }

    __END_CATCH
}

void DatabaseManager::addConnection(int TID, Connection* pConnection) {
    __BEGIN_TRY

    cout << "Adding TID connection BEGIN" << endl;

    bool inserted = false;
    {
        std::unique_lock lock(m_TablesMutex);
        inserted = m_Connections.try_emplace(TID, pConnection).second;
    }

    if (!inserted) {
        cout << "duplicated connection info id" << endl;
        throw DuplicatedException("duplicated connection info id");
    }

    cout << "Adding TID connection END" << endl;

    __END_CATCH
}

////////////////////////////////////////////////////////////////////////////
// Potion and Thread Connection section
////////////////////////////////////////////////////////////////////////////
void DatabaseManager::addDistConnection(int TID, Connection* pConnection) {
    __BEGIN_TRY

    cout << "Adding TID connection BEGIN" << endl;

    bool inserted = false;
    {
        std::unique_lock lock(m_TablesMutex);
        inserted = m_DistConnections.try_emplace(TID, pConnection).second;
    }

    if (!inserted) {
        cout << "duplicated connection info id" << endl;
        throw DuplicatedException("duplicated connection info id");
    }

    cout << "Adding TID connection END" << endl;

    __END_CATCH
}


Connection* DatabaseManager::getDistConnection(const string& connName)

{
    __BEGIN_TRY

    const int tid = (int)(long)Thread::self();

    {
        std::shared_lock lock(m_TablesMutex);
        unordered_map<int, Connection*>::const_iterator itr = m_DistConnections.find(tid);
        if (itr != m_DistConnections.end())
            return itr->second;
    }

#ifdef __LOGIN_SERVER__
    return m_pDefaultConnection;
#else
    return m_pWorldDefaultConnection;
#endif

    __END_CATCH
}


Connection* DatabaseManager::getConnection(const string& connName)

{
    __BEGIN_TRY

    const int tid = (int)(long)Thread::self();

    {
        std::shared_lock lock(m_TablesMutex);
        unordered_map<int, Connection*>::const_iterator itr = m_Connections.find(tid);
        if (itr != m_Connections.end())
            return itr->second;
    }

    return m_pDefaultConnection;

    __END_CATCH
}


Connection* DatabaseManager::getConnection(int TID)

{
    __BEGIN_TRY

    bool noWorldConnections = false;
    {
        std::shared_lock lock(m_TablesMutex);
        if (m_WorldConnections.empty())
            noWorldConnections = true;
        else {
            unordered_map<int, Connection*>::const_iterator itr = m_WorldConnections.find(TID);
            if (itr != m_WorldConnections.end())
                return itr->second;
        }
    }

    Assert(!noWorldConnections || m_pWorldDefaultConnection != NULL);

    return m_pWorldDefaultConnection;

    __END_CATCH
}

void DatabaseManager::executeDummyQuery(Connection* pConnection)

{
    __BEGIN_TRY

    Statement* pStmt = NULL;
    BEGIN_DB {
        pStmt = pConnection->createStatement();
        pStmt->executeQueryString("SELECT 1");

        SAFE_DELETE(pStmt);
    }
    END_DB(pStmt) {
        SAFE_DELETE(pStmt);
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////////////
// global variable definition
//////////////////////////////////////////////////////////////////////////////