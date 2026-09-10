#include "SMSServiceThread.h"

#include <unistd.h>

#include <chrono>

#include "Assert.h"
#include "Properties.h"
#include "StringStream.h"
#include "Timeval.h"
#include "repository/SMSMessageRepository.h"

#define KEY_SIZE 32

string SMSMessage::toString() const {
    StringStream msg;

    msg << "SMSMessage(" << m_SenderName << ", " << m_ReceiverNumber << ", " << m_CallerNumber << ", " << m_Message
        << ")";

    return msg.toString();
}

void operator++(string& id) {
    cout << "start operator++" << id << endl;

    string::iterator itr = id.end();
    --itr;

    while ((*itr) == '9' && itr != id.begin()) {
        (*itr) = '0';
        --itr;
    }

    Assert((*itr) <= '8' && (*itr) >= '0');
    ++(*itr);

    cout << "end operator++" << id << endl;
}

void SMSServiceThread::run() {
    __BEGIN_TRY

    if (g_pConfig->getPropertyInt("IsNetMarble") != 0) {
        // Nothing to relay on a NetMarble deployment. Idle until shutdown
        // rather than returning: a managed worker that returns while no stop
        // has been requested is reported as a worker failure.
        while (pauseFor(std::chrono::seconds(1))) {
        }
        return;
    }

    string host = g_pConfig->getProperty("SMS_DB_HOST");
    string db = g_pConfig->getProperty("SMS_DB_DB");
    string user = g_pConfig->getProperty("SMS_DB_USER");
    string password = g_pConfig->getProperty("SMS_DB_PASSWORD");
    uint port = 0;
    if (g_pConfig->hasKey("SMS_DB_PORT"))
        port = g_pConfig->getPropertyInt("SMS_DB_PORT");

    defaultSMSMessageRepository().open(host, db, user, password, port);

    uint Dimension = g_pConfig->getPropertyInt("Dimension");
    uint WorldID = g_pConfig->getPropertyInt("WorldID");
    uint ServerID = g_pConfig->getPropertyInt("ServerID");

    Assert(Dimension < 10);
    Assert(WorldID < 10);
    Assert(ServerID < 10);

    string mid;

    {
        char buffer[KEY_SIZE + 1];
        buffer[0] = Dimension + '0';
        buffer[1] = WorldID + '0';
        buffer[2] = ServerID + '0';
        for (int i = 3; i < KEY_SIZE; ++i)
            buffer[i] = '0';
        buffer[KEY_SIZE] = 0;
        mid = buffer;
    }

    {
        // The counter resumes from the highest id this server already
        // relayed; anything of another width is not one of ours.
        string max;
        defaultSMSMessageRepository().loadMaxMessageID(Dimension + '0', WorldID + '0', ServerID + '0', KEY_SIZE, max);

        if (max.size() == KEY_SIZE)
            mid = max;
        ++mid;
        cout << "initial mid : " << mid << endl;
    }

    Timeval dummyQueryTime;
    getCurrentTime(dummyQueryTime);

    while (!stopRequested()) {
        __ENTER_CRITICAL_SECTION(m_QueueMutex)

        try {
            if (!m_MessageQueue.empty()) {
                list<SMSMessage*>::const_iterator itr = m_MessageQueue.begin();
                list<SMSMessage*>::const_iterator endItr = m_MessageQueue.end();

                for (; itr != endItr; ++itr) {
                    SMSMessage* pMsg = *itr;

                    if (pMsg != NULL) {
                        filelog("SMS.log", "Send message [%s] %s", mid.c_str(), pMsg->toString().c_str());

                        // The id advances only for a message that reached
                        // the relay, so a refused row is retried under the
                        // same id by the next message in the queue.
                        if (defaultSMSMessageRepository().insertMessage(mid, pMsg->m_ReceiverNumber, pMsg->m_SenderName,
                                                                        pMsg->m_CallerNumber,
                                                                        getDBString(pMsg->m_Message))) {
                            defaultSMSMessageRepository().enqueue(mid);
                            filelog("SMS.log", "insert queue %s", mid.c_str());

                            ++mid;
                        }
                    }
                }

                m_MessageQueue.clear();
            }
        } catch (SQLQueryException& e) {
            filelog("SMSThreadException.log", "SQLQueryException:%s", e.toString().c_str());
            defaultSMSMessageRepository().reopen(host, db, user, password);
        } catch (Throwable& t) {
            filelog("SMSThreadException.log", "Throwable:%s", t.toString().c_str());
        }

        __LEAVE_CRITICAL_SECTION(m_QueueMutex)

        Timeval currentTime;
        getCurrentTime(currentTime);

        if (dummyQueryTime < currentTime) {
            defaultSMSMessageRepository().keepAlive();

            // 1시간 ~ 1시간 30분 사이에서 dummy query 시간을 설정한다.
            // timeout이 되지 않게 하기 위해서이다.
            dummyQueryTime.tv_sec += (60 + rand() % 30) * 60;
        }

        // Check the queue once per second. Stop-aware, so a shutdown request
        // wakes this immediately instead of costing another second.
        pauseFor(std::chrono::seconds(1));
    }

    __END_CATCH
}

string SMSServiceThread::getDBString(const string& msg) const {
    string ret = "";

    string::const_iterator itr = msg.begin();
    string::const_iterator endItr = msg.end();

    for (; itr != endItr; ++itr) {
        char ch = *itr;

        if (ch == '\'' || ch == '\\') {
            ret += '\\';
        }

        ret += ch;
    }

    return ret;
}

bool SMSServiceThread::isValidNumber(const string& num) const {
    if (num.size() > 11 || num.size() < 9)
        return false;

    string::const_iterator itr = num.begin();
    string::const_iterator endItr = num.end();

    if ((*itr) != '0')
        return false;

    itr++;

    if ((*itr) != '1')
        return false;

    for (; itr != endItr; ++itr) {
        if ((*itr) < '0' || (*itr) > '9')
            return false;
    }

    return true;
}

void SMSServiceThread::pushMessage(SMSMessage* pMsg) {
    __ENTER_CRITICAL_SECTION(m_QueueMutex)

    m_MessageQueue.push_back(pMsg);

    __LEAVE_CRITICAL_SECTION(m_QueueMutex)
}
