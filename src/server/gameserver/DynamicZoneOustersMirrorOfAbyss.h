/////////////////////////////////////////////////////////////////////////////
// DynamicZoneOustersMirrorOfAbyss.h
/////////////////////////////////////////////////////////////////////////////

#ifndef __DYNAMIC_ZONE_OUSTERS_MIRROR_OF_ABYSS_H__
#define __DYNAMIC_ZONE_OUSTERS_MIRROR_OF_ABYSS_H__

#include "DynamicZone.h"


///////////////////////////////////////////////////////////
// class DynamicZoneOustersMirrorOfAbyss
///////////////////////////////////////////////////////////
class DynamicZoneOustersMirrorOfAbyss : public DynamicZone {
public:
    DynamicZoneOustersMirrorOfAbyss();
    virtual ~DynamicZoneOustersMirrorOfAbyss();

public:
    void init();      // Dynamic zone initialization
    void heartbeat(); // Dynamic zone heartbeat

public:
    bool checkPC();
    bool addMono();
    bool clearMono();
    bool openGateToOut();
    bool checkComplete();
    bool checkMono();

    void processEntering();
    void killPC();

private:
    int m_OMOAStatus;
    ObjectID_t m_MonoObjectID;
    Timeval m_EndTime;
};


DECLARE_DYNAMIC_ZONE_FACTORY(DynamicZoneOustersMirrorOfAbyss)

#endif
