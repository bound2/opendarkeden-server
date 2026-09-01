//////////////////////////////////////////////////////////////////////////////
// Filename    : FlagSet.cpp
// Written by  : excel96
// Description :
//////////////////////////////////////////////////////////////////////////////

#include "FlagSet.h"

#include "Assert.h"
#include "StringStream.h"
#include "Utility.h"
#include "repository/FlagSetRepository.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

string FlagSet::m_pLookup[256];
bool FlagSet::m_bInit = false;

//////////////////////////////////////////////////////////////////////////////
// class Flag member methods
//////////////////////////////////////////////////////////////////////////////

FlagSet::FlagSet()

{
    __BEGIN_TRY

    // if (!FlagSet::m_bInit) FlagSet::initialize();

    memset(m_pData, 0, sizeof(BYTE) * FLAG_SIZE_MAX / 8);

    __END_CATCH
}

FlagSet::~FlagSet()

{
    __BEGIN_TRY
    __END_CATCH_NO_RETHROW
}

void FlagSet::create(const string& owner)

{
    __BEGIN_TRY

    defaultFlagSetRepository().insert(owner, toString());

    __END_CATCH
}

void FlagSet::load(const string& owner)

{
    __BEGIN_TRY

    string text;

    if (!defaultFlagSetRepository().load(owner, text)) {
        StringStream msg;
        msg << "FlagSet::load() " << "There is no flag data which owner is " << "[" << owner << "]";

        filelog("flagSetError.txt", "%s", msg.toString().c_str());

        // No flag set yet: store a default one so the character can play.
        defaultFlagSetRepository().insertEmptyIfMissing(owner);

        // cerr << msg.toString() << endl;
        // throw (msg.toString());
    } else {
        for (uint i = 0; i < text.size() && i < FLAG_SIZE_MAX; i++) {
            if (text[i] == '0')
                turnOff(i);
            else
                turnOn(i);
        }
    }

    __END_CATCH
}

void FlagSet::save(const string& owner)

{
    __BEGIN_TRY

    defaultFlagSetRepository().update(owner, toString());

    __END_CATCH
}

void FlagSet::destroy(const string& owner)

{
    __BEGIN_TRY

    defaultFlagSetRepository().remove(owner);

    __END_CATCH
}

bool FlagSet::isOn(int index)

{
    __BEGIN_TRY

    if (!isValidIndex(index))
        return false;
    if (m_pData[index / 8] & (1 << (index % 8)))
        return true;
    return false;

    __END_CATCH
}

bool FlagSet::turnOn(int index)

{
    __BEGIN_TRY

    if (!isValidIndex(index))
        return false;
    if (isOn(index))
        return false;

    m_pData[index >> 3] |= 1 << (index % 8);
    return true;

    __END_CATCH
}

bool FlagSet::turnOff(int index)

{
    __BEGIN_TRY

    if (!isValidIndex(index))
        return false;
    if (!isOn(index))
        return false;

    m_pData[index >> 3] &= ~(1 << (index % 8));
    return true;

    __END_CATCH
}

bool FlagSet::isValidIndex(int index)

{
    __BEGIN_TRY

    if (index < 0 || index >= FLAG_SIZE_MAX)
        return false;
    return true;

    __END_CATCH
}

string FlagSet::toString(void)

{
    __BEGIN_TRY

    string result = "";

    for (int i = 0; i < (FLAG_SIZE_MAX / 8); i++) {
        result += m_pLookup[m_pData[i]];
    }

    return result;

    __END_CATCH
}

FlagSet FlagSet::fromString(const string& text)

{
    __BEGIN_TRY

    FlagSet result;

    for (uint i = 0; i < text.size() && i < FLAG_SIZE_MAX; i++) {
        if (text[i] == '0')
            result.turnOff(i);
        else
            result.turnOn(i);
    }

    return result;

    __END_CATCH
}

void FlagSet::initialize(void)

{
    __BEGIN_TRY

    if (m_bInit)
        return;
    Assert(FLAGSET_MAX <= FLAG_SIZE_MAX);

    for (int i = 0; i < 256; i++) {
        string result = "";

        if (i & 0x01)
            result += "1";
        else
            result += "0";

        if (i & 0x02)
            result += "1";
        else
            result += "0";

        if (i & 0x04)
            result += "1";
        else
            result += "0";

        if (i & 0x08)
            result += "1";
        else
            result += "0";

        if (i & 0x10)
            result += "1";
        else
            result += "0";

        if (i & 0x20)
            result += "1";
        else
            result += "0";

        if (i & 0x40)
            result += "1";
        else
            result += "0";

        if (i & 0x80)
            result += "1";
        else
            result += "0";

        m_pLookup[i] = result;
    }

    m_bInit = true;

    __END_CATCH
}
