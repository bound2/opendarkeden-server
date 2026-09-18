//////////////////////////////////////////////////////////////////////
//
// File Name	: Connection.cpp
// Written by	: Gday29@ewestsoft.com
// Description	: Connection class implementation
//
//////////////////////////////////////////////////////////////////////

// include files
#include "Connection.h"

#include <charconv>
#include <cstdlib>
#include <cstring>

#include "Statement.h"

namespace {
unsigned int timeoutOption(const char* name, unsigned int fallback) {
    const char* text = std::getenv(name);
    if (text == nullptr)
        return fallback;
    unsigned int seconds = 0;
    const char* end = text + std::strlen(text);
    const auto result = std::from_chars(text, end, seconds);
    if (result.ec != std::errc() || result.ptr != end || seconds == 0)
        throw SQLConnectException(string(name) + " must be a positive number of seconds");
    return seconds;
}
} // namespace

//////////////////////////////////////////////////////////////////////
//
// constructor
//
// Connect to the database and select the schema.
// The exception-throwing part needs revisiting.
//
//////////////////////////////////////////////////////////////////////

Connection::Connection() : m_bConnected(false), m_bBusy(false) {
    __BEGIN_TRY

    m_Mutex.setName("Connection");

    // Initialize the MYSQL object.
    if (mysql_init(&m_Mysql) == NULL) {
        throw Error(mysql_error(&m_Mysql));
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// *CAUTION*
//
// Calling connect() without an explicit port uses the default value 0,
// which means MYSQL's default port is used. For security MYSQL should be
// run on another port, so the port has to be given explicitly.
//
//////////////////////////////////////////////////////////////////////

Connection::Connection(string host, string db, string user, string password, uint port)
    : m_bConnected(false), m_Host(host), m_Port(port), m_Database(db), m_User(user), m_Password(password) {
    __BEGIN_TRY

    m_Mutex.setName("Connection");

    // Initialize the MYSQL object.
    if (mysql_init(&m_Mysql) == NULL)
        throw Error(mysql_error(&m_Mysql));

    // Connect right away.
    connect(m_Host, m_Database, m_User, m_Password, m_Port);

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
//	destructor
//
//////////////////////////////////////////////////////////////////////

Connection::~Connection() {
    // If still connected, close the connection.
    if (m_bConnected) {
        close();
    }
}

//////////////////////////////////////////////////////////////////////
//
// connect()
//
// When a Connection object is constructed without knowing where to connect,
// create it with the default constructor first, then
// initialize the data members from the connect() arguments and connect.
//
// *CAUTION*
//
// Calling connect() without an explicit port uses the default value 0,
// which means MYSQL's default port is used. For security MYSQL should be
// run on another port.
//
// The port parameter comes last because it needs a default value.
//
// ex> Connection conn;
//     conn.connect( "vampire.ewestsoft.com" , "bbs" , "bbsuser" , "bbspassword" );
//
//////////////////////////////////////////////////////////////////////

void Connection::connect(string host, string db, string user, string password, uint port)

{
    __BEGIN_TRY

    m_Host = host;
    m_Port = port;
    m_Database = db;
    m_User = user;
    m_Password = password;

    connect();

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
// connect()
//
// When the constructor received the connection info directly, it calls this
// method to open the connection.
//
//////////////////////////////////////////////////////////////////////

void Connection::connect()

{
    __BEGIN_TRY

    // Connecting while already connected is an error.
    if (m_bConnected) {
        // In that case the connection should be closed and an exception thrown.
        close();

        throw SQLConnectException("Already Connected");
    }

    //--------------------------------------------------
    // Should check whether an error occurs...
    //--------------------------------------------------
    // Bound synchronous MySQL calls. These are per-operation limits, not a
    // total shutdown deadline (the gameserver watchdog enforces that).
    const unsigned int connectSeconds = timeoutOption("DARKEDEN_DB_CONNECT_TIMEOUT_SECONDS", 5);
    const unsigned int ioSeconds = timeoutOption("DARKEDEN_DB_IO_TIMEOUT_SECONDS", 300);
    if (mysql_options(&m_Mysql, MYSQL_OPT_CONNECT_TIMEOUT, &connectSeconds) != 0 ||
        mysql_options(&m_Mysql, MYSQL_OPT_READ_TIMEOUT, &ioSeconds) != 0 ||
        mysql_options(&m_Mysql, MYSQL_OPT_WRITE_TIMEOUT, &ioSeconds) != 0)
        throw SQLConnectException(mysql_error(&m_Mysql));
    m_bConnected = (mysql_real_connect(&m_Mysql, m_Host.c_str(), m_User.c_str(), m_Password.c_str(), m_Database.c_str(),
                                       m_Port, 0, 0) != NULL);

    // Not being connected is an error.
    if (!m_bConnected) {
        throw SQLConnectException(mysql_error(&m_Mysql));
    }

    // Pin the session to the tables' character set. Left to the handshake,
    // the 8.0 client library asks for a collation MySQL 5.7 does not know and
    // the server silently drops the session to latin1, after which text is
    // passed through as raw bytes rather than transcoded.
    if (mysql_set_character_set(&m_Mysql, "utf8mb4") != 0) {
        const string error = mysql_error(&m_Mysql);
        mysql_close(&m_Mysql);
        m_bConnected = false;
        throw SQLConnectException(error);
    }

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
//	close()
//
//	Close the connection.
//
//////////////////////////////////////////////////////////////////////

void Connection::close()

{
    __BEGIN_TRY

    if (!m_bConnected) {
        throw SQLConnectException("Not Connected");
    }

    // Returns void, so ignore it.
    mysql_close(&m_Mysql);

    m_bConnected = false;

    __END_CATCH
}

//////////////////////////////////////////////////////////////////////
//
//	createStatement
//
//	Create a new Statement object and return a pointer to it.
//
//////////////////////////////////////////////////////////////////////

Statement* Connection::createStatement() {
    __BEGIN_TRY

    // Create the new Statement object.
    Statement* pStatement = new Statement();

    // Point the new object's connection at this connection.
    pStatement->setConnection(this);

    // Return it.
    return pStatement;

    __END_CATCH
}
