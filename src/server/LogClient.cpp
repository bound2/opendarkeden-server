//////////////////////////////////////////////////////////////////////////////
// Filename    : LogClient.cpp
// Written by  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "LogClient.h"

#include <errno.h>
#include <fcntl.h> /* for nonblocking */
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> /* timespec{} for pselect() */
#include <unistd.h>

#include <arpa/inet.h>  /* inet(3) functions */
#include <netinet/in.h> /* sockaddr_in{} and other Internet defns */
#include <sys/socket.h> /* basic socket definitions */
#include <sys/stat.h>   /* for S_xxx file mode constants */
#include <sys/time.h>   /* timeval{} for select() */
#include <sys/types.h>  /* basic system data types */
#include <sys/uio.h>    /* for iovec{} and readv/writev */
#include <sys/un.h>     /* for Unix domain sockets */
#include <sys/wait.h>

#include "LogData.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int LogClient::m_LogLevel = 0;

namespace {
LogClient* s_pLogClient = NULL;
} // namespace

void openLogClient(const string& ip, short port) {
    s_pLogClient = new LogClient(ip, port);
}

LogClient* logClient() {
    return s_pLogClient;
}

void log(short type, const string& source, const string& target) {}

void log(short type, const string& source, const string& target, const string& content) {}

void log(short type, const string& source, const string& target, const string& content, short zoneid) {}


//////////////////////////////////////////////////////////////////////////////
// class LogClient member methods
//////////////////////////////////////////////////////////////////////////////

LogClient::LogClient(string ip, short port) {
    m_bConnected = false;
    m_Sent = 0;

    connect(ip, port);
}

LogClient::~LogClient() {
    disconnect();
}

void LogClient::connect(string ip, short port) {}

void LogClient::disconnect(void) {}

void LogClient::_log(short type, const string& source, const string& target) {}

void LogClient::_log(short type, const string& source, const string& target, const string& content) {}

void LogClient::_log(short type, const string& source, const string& target, const string& content, short zoneid) {}
