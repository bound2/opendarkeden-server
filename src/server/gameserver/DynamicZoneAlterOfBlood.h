/////////////////////////////////////////////////////////////////////////////
// DynamicZoneAlterOfBlood.h
/////////////////////////////////////////////////////////////////////////////

#ifndef __DYNAMIC_ZONE_ALTER_OF_BLOOD_H__
#define __DYNAMIC_ZONE_ALTER_OF_BLOOD_H__

#include "DynamicZone.h"


///////////////////////////////////////////////////////////
// class DynamicZoneAlterOfBlood
///////////////////////////////////////////////////////////
class DynamicZoneAlterOfBlood : public DynamicZone {
public:
    DynamicZoneAlterOfBlood();
    virtual ~DynamicZoneAlterOfBlood();

public:
    void init();      // Dynamic zone initialization
    void heartbeat(); // Dynamic zone heartbeat

public:
    bool checkPC();
    bool addOffering();
    bool checkNoOffering();
    bool clearOffering();
    bool openGateToOut();

    void processEntering();

public:
    void setRace(Race_t race) {
        m_Race = race;
    }

private:
    int m_AOBStatus;
    Race_t m_Race;
    ObjectID_t m_OfferingObjectID[5];
};


DECLARE_DYNAMIC_ZONE_FACTORY(DynamicZoneAlterOfBlood)

#endif
