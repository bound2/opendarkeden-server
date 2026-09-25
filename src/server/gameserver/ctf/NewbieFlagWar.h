#ifndef __NEWBIE_FLAG_WAR_H__
#define __NEWBIE_FLAG_WAR_H__

#include "FlagWar.h"

class NewbieFlagWar : public FlagWar {
public:
    NewbieFlagWar(FlagManager& flagManager, de::GameContext& context) : FlagWar(flagManager, context) {}

    virtual int getWarTime() const {
        return 3600;
    }

protected:
    virtual void executeEnd();
    virtual VSDateTime getNextFlagWarTime();
    virtual std::vector<de::ctf::FlagDrop> flagDrops() const;
};

#endif
