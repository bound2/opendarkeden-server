#ifndef __SMS_SERVICE_THREAD_H__
#define __SMS_SERVICE_THREAD_H__

#include <list>
#include <string>

#include "DB.h"
#include "Exception.h"
#include "ManagedThread.h"
#include "Mutex.h"
#include "Types.h"

class SMSServiceThread;

class SMSMessage {
public:
    SMSMessage(const string& senderName, const string& recvNum, const string& callerNum, const string& msg)
        : m_SenderName(senderName), m_ReceiverNumber(recvNum), m_CallerNumber(callerNum), m_Message(msg) {}
    string toString() const;

    friend class SMSServiceThread;

private:
    string m_SenderName;
    string m_ReceiverNumber;
    string m_CallerNumber;
    string m_Message;
};

class SMSServiceThread : public ManagedThread {
public:
    // The worker drains m_MessageQueue and owns m_pConnection, so it must be
    // joined before those members are destroyed. A base destructor would run
    // too late. (In the deployed gameserver this never runs: the process
    // exits with _Exit rather than unwinding the legacy singleton graph.)
    ~SMSServiceThread() noexcept override {
        stop();
        join();
    }

    static SMSServiceThread& Instance() {
        static SMSServiceThread theInstance;
        return theInstance;
    }

    void run() override;
    string getName() const override {
        return "SMSServiceThread";
    }

    void pushMessage(SMSMessage* pMsg);
    string getDBString(const string& msg) const;
    bool isValidNumber(const string& num) const;

private:
    SMSServiceThread() : m_QueueMutex(), m_pConnection(NULL) {
        m_QueueMutex.setName("SMS Queue Lock");
    }

    Mutex m_QueueMutex;
    list<SMSMessage*> m_MessageQueue;
    Connection* m_pConnection;
};

#endif
