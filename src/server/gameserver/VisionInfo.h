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
// class VisionInfo;
// Whether P(x,y) is visible to a creature at O(cx,cy) with sight level Sight
// facing direction dir need not be computed at runtime: precomputed values
// kept in a vision rectangle per sight level and direction make the check
// much faster. This class represents that vision rectangle.
////////////////////////////////////////////////////////////////////////////////

/*class VisionInfo
{
public:
    VisionState getVisionState(ZoneCoord_t cx, ZoneCoord_t cy, ZoneCoord_t x, ZoneCoord_t y) const
    {
        // inline for speed. by sigi. 2002.11.8
//		int px = x + (maxViewportWidth + 1) - cx;
//		int py = y + (maxViewportUpperHeight + 1) - cy;
        int px = x + (maxViewportWidth) - cx;
        int py = y + (maxViewportUpperHeight) - cy;

        // Check the range of the translated coordinates.
        if (px >= 0 && px < maxVisionWidth && py >= 0 && py < maxVisionHeight)
            return m_Rect[px][py];

        return OUT_OF_SIGHT;
    }
    string toString() const ;

private:
    // Sight level (0 - 13)
    Sight_t m_Sight;

    // Facing direction
    Dir_t m_Dir;

    // viewport rectangle
    VisionState m_Rect[maxVisionWidth][maxVisionHeight];
    // deprecated. This class is no longer used.
    VisionInfo(Sight_t sight, Dir_t dir) ;
    ~VisionInfo();
};
*/

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

    // get vision info
    /*	VisionInfo* getVisionInfo(Sight_t sight, Dir_t dir) //
        {
            // for speed. by sigi. 2002.11.8
            //__BEGIN_TRY
            //if (sight > maxSight) throw OutOfBoundException("sight overflow");
            //if (dir >= DIR_MAX) throw OutOfBoundException("dir overflow");

            return m_pVisionInfos[sight][dir];

            //__END_CATCH
        }*/

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
    // Two-dimensional array of VisionInfo
    //	VisionInfo *** m_pVisionInfos;
};

#endif
