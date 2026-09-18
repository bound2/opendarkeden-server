//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectMeteorStrike.h
// Written by  : elca@ewestsoft.com
// Description : Class that handles the Effect of the clerical magic MeteorStrike.
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_METEOR_STRIKE__
#define __EFFECT_METEOR_STRIKE__

#include "Effect.h"
#include "EffectLoader.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectMeteorStrike
//////////////////////////////////////////////////////////////////////////////

class EffectMeteorStrike : public Effect {
public:
    EffectMeteorStrike(Zone* pZone, ZoneCoord_t zoneX, ZoneCoord_t zoneY, bool bPlayer = false);

public:
    EffectClass getEffectClass() const {
        return EFFECT_CLASS_METEOR_STRIKE;
    }

    void affect();
    void affect(Creature* pCreature);

    void unaffect();
    void unaffect(Creature* pCreature);

    string toString() const;

public:
    int getDamage(void) const {
        return m_Damage;
    }
    void setDamage(int damage) {
        m_Damage = damage;
    }

    Turn_t getDelay() const {
        return m_Delay;
    }
    void setDelay(Turn_t Delay) {
        m_Delay = Delay;
    }


    ObjectID_t getUserObjectID(void) const {
        return m_UserObjectID;
    }
    void setUserObjectID(ObjectID_t UserObjectID) {
        m_UserObjectID = UserObjectID;
    }

    void setSplashRatio(int index, uint ratio) {
        m_SplashRatio[index] = ratio;
    }

private:
    int m_Damage;
    Turn_t m_Delay;
    ObjectID_t m_UserObjectID;
    bool m_bPlayer; // True if a player used the skill
    uint m_SplashRatio[3];
};

//////////////////////////////////////////////////////////////////////////////
// class EffectMeteorStrikeLoader
//////////////////////////////////////////////////////////////////////////////

class EffectMeteorStrikeLoader : public EffectLoader {
public:
    virtual Effect::EffectClass getEffectClass() const {
        return Effect::EFFECT_CLASS_METEOR_STRIKE;
    }
    virtual string getEffectClassName() const {
        return "EffectMeteorStrike";
    }

public:
    virtual void load(Creature* pCreature) {}
};

#endif // __EFFECT_METEOR_STRIKE__
