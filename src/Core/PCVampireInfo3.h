//////////////////////////////////////////////////////////////////////////////
// Filename    : PCVampireInfo3.h
// Written By  : Reiot
// Description :
//////////////////////////////////////////////////////////////////////////////

#ifndef __PC_VAMPIRE_INFO_3_H__
#define __PC_VAMPIRE_INFO_3_H__

#include "Assert.h"
#include "PCInfo.h"
#include "WireString.h"

//////////////////////////////////////////////////////////////////////////////
// class PCVampireInfo3;
// Object that carries the vampire's appearance information
// It is carried in GCAddSlayer and GCAddVampireCorpse.
//////////////////////////////////////////////////////////////////////////////

class PCVampireInfo3 : public PCInfo {
public:
    // Vampire Color Informations
    enum VampireColors {
        VAMPIRE_COLOR_BAT,
        VAMPIRE_COLOR_SKIN,
        VAMPIRE_COLOR_COAT1,
        VAMPIRE_COLOR_COAT2,
        VAMPIRE_COLOR_MAX
    };

public:
    PCVampireInfo3() {}

    // The record is plain data, so a copy carries every member write()
    // emits.
    PCVampireInfo3(const PCVampireInfo3& vampireInfo) = default;

public:
    PCType getPCType() const {
        return PC_VAMPIRE;
    }

    void read(SocketInputStream& iStream);
    void write(SocketOutputStream& oStream) const;

    uint getSize() const {
        return szObjectID                         // ObjectID
               + de::wire::stringWireSize(m_Name) // Vampire name
               + szCoord + szCoord + szDir        // Coordinates and direction
               + szSex                            // Sex
               + szBYTE                           // coatType
               + szColor * VAMPIRE_COLOR_MAX      // Colour
               + szBYTE + szHP * 2                // Max HP
               + szAlignment                      // Alignment
               + szShape                          // Shape
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
               + szBYTE                      // coatType
               + szColor * VAMPIRE_COLOR_MAX // Colour
               + szBYTE + szHP * 2           // Max HP
               + szAlignment                 // Alignment
               + szShape                     // Shape
               + szSpeed                     // Attack speed
               + szGuildID                   // Guild ID
               + szRank                      // Rank
               + szBYTE                      // Competence
               + szuint + szLevel;
    }

    PCVampireInfo3& operator=(const PCVampireInfo3& vampireInfo) = default;

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

    Color_t getBatColor() const {
        return m_Colors[VAMPIRE_COLOR_BAT];
    }
    void setBatColor(Color_t batColor) {
        m_Colors[VAMPIRE_COLOR_BAT] = batColor;
    }

    Color_t getSkinColor() const {
        return m_Colors[VAMPIRE_COLOR_SKIN];
    }
    void setSkinColor(Color_t skinColor) {
        m_Colors[VAMPIRE_COLOR_SKIN] = skinColor;
    }

    ItemType_t getCoatType() const {
        return m_CoatType;
    }
    void setCoatType(ItemType_t CoatType) {
        m_CoatType = CoatType;
    }

    Color_t getCoatColor(ColorType colorType = MAIN_COLOR) const {
        return m_Colors[VAMPIRE_COLOR_COAT1 + (int)colorType];
    }
    void setCoatColor(Color_t coatColor, ColorType colorType = MAIN_COLOR) {
        m_Colors[VAMPIRE_COLOR_COAT1 + (int)colorType] = coatColor;
    }

    BYTE getMasterEffectColor() const {
        return m_MasterEffectColor;
    }
    void setMasterEffectColor(BYTE color) {
        m_MasterEffectColor = color;
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

    Shape_t getShape() const {
        return m_Shape;
    }
    void setShape(Shape_t Shape) {
        m_Shape = Shape;
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
    ItemType_t m_CoatType = 0;

    // colors
    Color_t m_Colors[VAMPIRE_COLOR_MAX] = {};

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

    // Vampire shape
    Shape_t m_Shape = 0;

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
