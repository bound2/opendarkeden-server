///////////////////////////////////////////////////////////////////
// General war information and the routines run when a war starts and ends.
///////////////////////////////////////////////////////////////////

#ifndef __RACE_WAR_H__
#define __RACE_WAR_H__

#include "War.h"

class Mutex;
class PlayerCreature;

class RaceWar : public War {
public:
    RaceWar(WarState warState, WarID_t warID = 0);
    virtual ~RaceWar();

    WarType_t getWarType() const {
        return WAR_RACE;
    }
    string getWarType2DBString() const {
        return "RACE";
    }
    string getWarName() const;

public:
    void sendWarEndMessage() const;

protected:
    void executeStart();
    void executeEnd();

    void recordRaceWarStart();
    void recordRaceWarEnd();

public:
    void makeWarScheduleInfo(WarScheduleInfo* pWSI) const;
    void makeWarInfo(WarInfo* pWarInfo) const;

    virtual string toString() const;

private:
};

#endif // __WAR_H__
