//////////////////////////////////////////////////////////////////////////////
// Filename    : DatabaseManager.cpp
// Written By  : elca@ewestsoft.com
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "DatabaseManager.h"

#include <stdio.h>

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

    m_Mutex.setName("DatabaseManager");

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
            m_WorldConnections[WorldID] = pConnection;

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

    __ENTER_CRITICAL_SECTION(m_Mutex)

    unordered_map<int, Connection*>::iterator itr = m_Connections.find(TID);

    if (itr != m_Connections.end()) {
        cout << "duplicated connection info id" << endl;
        throw DuplicatedException("duplicated connection info id");
    }

    m_Connections[TID] = pConnection;

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    cout << "Adding TID connection END" << endl;

    __END_CATCH
}

////////////////////////////////////////////////////////////////////////////
// Potion and Thread Connection section
////////////////////////////////////////////////////////////////////////////
void DatabaseManager::addDistConnection(int TID, Connection* pConnection) {
    __BEGIN_TRY

    cout << "Adding TID connection BEGIN" << endl;

    __ENTER_CRITICAL_SECTION(m_Mutex)

    unordered_map<int, Connection*>::iterator itr = m_DistConnections.find(TID);

    if (itr != m_DistConnections.end()) {
        cout << "duplicated connection info id" << endl;
        throw DuplicatedException("duplicated connection info id");
    }

    m_DistConnections[TID] = pConnection;

    __LEAVE_CRITICAL_SECTION(m_Mutex)

    cout << "Adding TID connection END" << endl;

    __END_CATCH
}


Connection* DatabaseManager::getDistConnection(const string& connName)

{
    __BEGIN_TRY

    Connection* pTempConnection = NULL;

    unordered_map<int, Connection*>::iterator itr = m_DistConnections.find((int)(long)Thread::self());

    if (itr == m_DistConnections.end()) {
#ifdef __LOGIN_SERVER__
        pTempConnection = m_pDefaultConnection;
#else
        pTempConnection = m_pWorldDefaultConnection;
#endif
    } else {
        pTempConnection = itr->second;
    }

    return pTempConnection;

    __END_CATCH
}


Connection* DatabaseManager::getConnection(const string& connName)

{
    __BEGIN_TRY

    Connection* pTempConnection = NULL;

    unordered_map<int, Connection*>::iterator itr;


    itr = m_Connections.find((int)(long)Thread::self());

    if (itr == m_Connections.end())
        pTempConnection = m_pDefaultConnection;
    else
        pTempConnection = itr->second;

    return pTempConnection;

    __END_CATCH
}


Connection* DatabaseManager::getConnection(int TID)

{
    __BEGIN_TRY

    if (m_WorldConnections.empty()) // by sigi. 2002.10.23
    {
        Assert(m_pWorldDefaultConnection);
        return m_pWorldDefaultConnection;
    } else // by sigi. 2002.10.23
    {
        Connection* pTempConnection = NULL;

        unordered_map<int, Connection*>::iterator itr = m_WorldConnections.find(TID);

        if (itr == m_WorldConnections.end()) {
            pTempConnection = m_pWorldDefaultConnection;
        } else {
            pTempConnection = itr->second;
        }

        return pTempConnection;
    }

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