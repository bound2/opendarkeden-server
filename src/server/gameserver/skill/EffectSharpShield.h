//////////////////////////////////////////////////////////////////////////////
// Filename    : EffectSharpShield.h
// Written by  : crazydog
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __EFFECT_SHARP_SHIELD__
#define __EFFECT_SHARP_SHIELD__

#include "Effect.h"
#include "EffectLoader.h"

//////////////////////////////////////////////////////////////////////////////
// class EffectSharpShield
//////////////////////////////////////////////////////////////////////////////

class EffectSharpShield : public Effect {
public:
    EffectSharpShield(Creature* pCreature);

public:
    EffectClass getEffectClass() const {
        return EFFECT_CLASS_SHARP_SHIELD_1;
    }
    EffectClass getSendEffectClass() const {
        return m_ClientEffectClass;
    }

    void affect() {}
    void affect(Creature* pCreature);
    void affect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject);

    void unaffect(Creature* pCreature);
    void unaffect(Zone* pZone, ZoneCoord_t x, ZoneCoord_t y, Object* pObject);
    void unaffect(Item* pItem) {}
    void unaffect();

    string toString() const;

public:
    void setLevel(Level_t Level);

    Damage_t getDamage() const {
        return m_Damage;
    }
    void setDamage(Damage_t Damage) {
        m_Damage = Damage;
    }

    EffectClass getClientEffectClass() const {
        return m_ClientEffectClass;
    }
    void setClientEffectClass(EffectClass effectClass) {
        m_ClientEffectClass = effectClass;
    }

private:
    EffectClass m_ClientEffectClass; // 클라이언트에 보내줄때 쓰는 이펙트 클래스 아이디
    Damage_t m_Damage;               // 때린놈한테 주는 데미지
};

//////////////////////////////////////////////////////////////////////////////
// class EffectSharpShieldLoader
//////////////////////////////////////////////////////////////////////////////

class EffectSharpShieldLoader : public EffectLoader {
public:
    virtual Effect::EffectClass getEffectClass() const {
        return Effect::EFFECT_CLASS_SHARP_SHIELD_1;
    }
    virtual string getEffectClassName() const {
        return "EffectSharpShield";
    }

public:
    virtual void load(Creature* pCreature) {}
};


#endif // __EFFECT_SHARP_SHIELD__
