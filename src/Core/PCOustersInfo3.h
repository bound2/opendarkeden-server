//////////////////////////////////////////////////////////////////////////////
// Filename    : PCOustersInfo3.h
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __PC_OUSTERS_INFO_3_H__
#define __PC_OUSTERS_INFO_3_H__

#include "Assert.h"
#include "PCInfo.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////////////
// class PCOustersInfo3;
// Object that carries the vampire's appearance information
// It is carried in GCAddOusters and GCAddOustersCorpse.
//////////////////////////////////////////////////////////////////////////////

class PCOustersInfo3 : public PCInfo {
public:
    // Ousters Color Informations
    enum OustersColors {
        OUSTERS_COLOR_COAT,
        OUSTERS_COLOR_HAIR,
        OUSTERS_COLOR_ARM,
        OUSTERS_COLOR_BOOTS,
        OUSTERS_COLOR_MAX
    };

public:
    PCOustersInfo3() {}

    // The record is plain data, so a copy carries every member write()
    // emits.
    PCOustersInfo3(const PCOustersInfo3& oustersInfo) = default;

public:
    PCType getPCType() const {
        return PC_OUSTERS;
    }

    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;

    uint getSize() const {
        return szObjectID                         // ObjectID
               + de::wire::stringWireSize(m_Name) // Vampire name
               + szCoord + szCoord + szDir        // Coordinates and direction
               + szSex                            // Sex
               + szBYTE                           // shape
               + szColor * OUSTERS_COLOR_MAX      // Colour
               + szBYTE + szHP * 2                // Max HP
               + szAlignment                      // Alignment
               + szSpeed                          // Attack speed
               + szGuildID                        // Guild ID
               + szRank                           // Rank
               + szBYTE                           // Competence
               + szuint + szLevel;
    }

    // get max size of object
    static constexpr uint getMaxSize() {
        return szObjectID                    // ObjectID
               + szBYTE + 20                 // Vampire name
               + szCoord + szCoord + szDir   // Coordinates and direction
               + szSex                       // Sex
               + szBYTE                      // shape
               + szColor * OUSTERS_COLOR_MAX // Colour
               + szBYTE + szHP * 2           // Max HP
               + szAlignment                 // Alignment
               + szSpeed                     // Attack speed
               + szGuildID                   // Guild ID
               + szRank                      // Rank
               + szBYTE                      // Competence
               + szuint + szLevel;
    }

    PCOustersInfo3& operator=(const PCOustersInfo3& oustersInfo) = default;

    string toString() const;

public:
    ObjectID_t getObjectID() const {
        return m_ObjectID;
    }
    void setObjectID(ObjectID_t objectID) {
        m_ObjectID = objectID;
    }

    string getName() const {
        return m_Name;
    }
    void setName(const string& name) {
        m_Name = name;
        Assert(m_Name != "");
    }

    Coord_t getX() const {
        return m_X;
    }
    void setX(Coord_t x) {
        m_X = x;
    }

    Coord_t getY() const {
        return m_Y;
    }
    void setY(Coord_t y) {
        m_Y = y;
    }

    Dir_t getDir() const {
        return m_Dir;
    }
    void setDir(Dir_t dir) {
        m_Dir = dir;
    }

    Sex getSex() const {
        return m_Sex;
    }
    void setSex(Sex sex) {
        m_Sex = sex;
    }
    void setSex(const string& sex) {
        if (sex == Sex2String[MALE])
            m_Sex = MALE;
        else if (sex == Sex2String[FEMALE])
            m_Sex = FEMALE;
        else
            throw InvalidProtocolException("invalid sex value");
    }

    Color_t getCoatColor() const {
        return m_Colors[OUSTERS_COLOR_COAT];
    }
    void setCoatColor(Color_t coatColor) {
        m_Colors[OUSTERS_COLOR_COAT] = coatColor;
    }

    Color_t getHairColor() const {
        return m_Colors[OUSTERS_COLOR_HAIR];
    }
    void setHairColor(Color_t hairColor) {
        m_Colors[OUSTERS_COLOR_HAIR] = hairColor;
    }

    Color_t getArmColor() const {
        return m_Colors[OUSTERS_COLOR_ARM];
    }
    void setArmColor(Color_t armColor) {
        m_Colors[OUSTERS_COLOR_ARM] = armColor;
    }

    Color_t getBootsColor() const {
        return m_Colors[OUSTERS_COLOR_BOOTS];
    }
    void setBootsColor(Color_t bootsColor) {
        m_Colors[OUSTERS_COLOR_BOOTS] = bootsColor;
    }

    BYTE getMasterEffectColor() const {
        return m_MasterEffectColor;
    }
    void setMasterEffectColor(BYTE color) {
        m_MasterEffectColor = color;
    }

    OustersCoatType getCoatType() const {
        return m_CoatType;
    }
    void setCoatType(OustersCoatType CoatType) {
        m_CoatType = CoatType;
    }

    OustersArmType getArmType() const {
        return m_ArmType;
    }
    void setArmType(OustersArmType ArmType) {
        m_ArmType = ArmType;
    }

    OustersSylphType getSylphType() const {
        return m_SylphType;
    }
    void setSylphType(OustersSylphType SylphType) {
        m_SylphType = SylphType;
    }

    HP_t getCurrentHP() const {
        return m_CurrentHP;
    }
    void setCurrentHP(HP_t CurrentHP) {
        m_CurrentHP = CurrentHP;
    }

    HP_t getMaxHP() const {
        return m_MaxHP;
    }
    void setMaxHP(HP_t MaxHP) {
        m_MaxHP = MaxHP;
    }

    Speed_t getAttackSpeed() const {
        return m_AttackSpeed;
    }
    void setAttackSpeed(Speed_t AttackSpeed) {
        m_AttackSpeed = AttackSpeed;
    }

    Alignment_t getAlignment() const {
        return m_Alignment;
    }
    void setAlignment(Alignment_t Alignment) {
        m_Alignment = Alignment;
    }

    BYTE getCompetence(void) const {
        return m_Competence;
    }
    void setCompetence(BYTE competence) {
        m_Competence = competence;
    }

    GuildID_t getGuildID(void) const {
        return m_GuildID;
    }
    void setGuildID(GuildID_t GuildID) {
        m_GuildID = GuildID;
    }

    uint getUnionID(void) const {
        return m_UnionID;
    }
    void setUnionID(uint UnionID) {
        m_UnionID = UnionID;
    }

    Rank_t getRank() const {
        return m_Rank;
    }
    void setRank(Rank_t rank) {
        m_Rank = rank;
    }

    Level_t getAdvancementLevel() const {
        return m_AdvancementLevel;
    }
    void setAdvancementLevel(Level_t level) {
        m_AdvancementLevel = level;
    }

private:
    // PC's object id
    ObjectID_t m_ObjectID = 0;

    // PC name
    string m_Name;

    Coord_t m_X = 0;
    Coord_t m_Y = 0;
    Dir_t m_Dir = 0;

    // PC sex
    Sex m_Sex = FEMALE;

    // CoatType
    OustersCoatType m_CoatType = OUSTERS_COAT_BASIC;

    // ArmType
    OustersArmType m_ArmType = OUSTERS_ARM_GAUNTLET;

    // SylphType
    OustersSylphType m_SylphType = OUSTERS_SYLPH_NONE;

    // colors
    Color_t m_Colors[OUSTERS_COLOR_MAX] = {};

    // Master effect colour
    BYTE m_MasterEffectColor = 0;

    // Current HP
    HP_t m_CurrentHP = 0;

    // Max HP
    HP_t m_MaxHP = 0;

    // Attack Speed
    Speed_t m_AttackSpeed = 0;

    // Alignment
    Alignment_t m_Alignment = 0;

    // Competence
    BYTE m_Competence = 0;

    // Guild ID
    GuildID_t m_GuildID = 0;

    uint m_UnionID = 0;

    // Rank
    Rank_t m_Rank = 0;

    Level_t m_AdvancementLevel = 0;
};

#endif
