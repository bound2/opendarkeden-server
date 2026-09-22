////////////////////////////////////////////////////////////////////////////////
// Filename    : VisionInfo.h
// Written By  : Reiot
// Description :
////////////////////////////////////////////////////////////////////////////////

#ifndef __VISIONINFO_H__
#define __VISIONINFO_H__

#include "Exception.h"
#include "Types.h"

const Coord_t maxViewportWidth = 10;
const Coord_t maxViewportUpperHeight = 14;
const Coord_t maxViewportLowerHeight = 14;
const Coord_t maxVisionWidth = maxViewportWidth * 2 + 1;
const Coord_t maxVisionHeight = maxViewportUpperHeight + maxViewportLowerHeight + 1;

////////////////////////////////////////////////////////////////////////////////
// class VisionInfoManager;
// Manages vision info and returns the entry for a given sight level and direction.
////////////////////////////////////////////////////////////////////////////////

class VisionInfoManager {
public:
    VisionInfoManager() {}
    ~VisionInfoManager();

public:
    // init vision info
    void init();

    static VisionState getVisionState(ZoneCoord_t SourceX, ZoneCoord_t SourceY, ZoneCoord_t TargetX,
                                      ZoneCoord_t TargetY) {
        int diffX = abs(TargetX - SourceX);
        int diffY = (int)TargetY - (int)SourceY;

        bool isInX = diffX <= maxViewportWidth;
        bool isInY = (diffY < 0) ? ((-diffY) <= maxViewportUpperHeight) : (diffY <= maxViewportLowerHeight);

        return (isInX && isInY) ? IN_SIGHT : OUT_OF_SIGHT;
    }

    // get debug string
    string toString() const;

private:
};

#endif
