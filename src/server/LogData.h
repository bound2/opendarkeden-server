//////////////////////////////////////////////////////////////////////////////
// Filename   : LogData.h
// Written by : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __LOGDATA_H__
#define __LOGDATA_H__

#include "Types.h"

//////////////////////////////////////////////////////////////////////////////
// class LogData
//////////////////////////////////////////////////////////////////////////////

class LogData {
    ///// Member methods /////

public: // constructor & destructor
    LogData();
    ~LogData();

public: // identity methods
    short getType(void) {
        return m_Type;
    }
    short getZoneID(void) {
        return m_ZoneID;
    }
    const string& getSource(void) {
        return m_Source;
    }
    const string& getTarget(void) {
        return m_Target;
    }
    const string& getContent(void) {
        return m_Content;
    }

    void setType(short type) {
        m_Type = type;
    }
    void setZoneID(short id) {
        m_ZoneID = id;
    }
    void setSource(const string& source) {
        m_Source = source;
    }
    void setTarget(const string& target) {
        m_Target = target;
    }
    void setContent(const string& content) {
        m_Content = content;
    }

public: // i/o
    uint getSize(void);
    int read(int sockfd);
    void write(int sockfd);

public: // misc
    string toString(void) const;


    ///// Member data /////

protected:
    short m_Type;     // the kind of event
    short m_ZoneID;   // the zone id where the event happened
    string m_Source;  // the name of the player that caused the event
    string m_Target;  // the name of the player the event happened to
    string m_Content; // the event's content
};

#endif
