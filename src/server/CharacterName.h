//////////////////////////////////////////////////////////////////////////////
// Filename    : CharacterName.h
// Description : The reserved tokens no character name may contain: the
//               rule the loginserver applies to a new name (CLCreatePC,
//               CLQueryCharacterName) and the gameserver to the name a
//               port check carries (PortCheck.h). Substring matches, so
//               "GMaster" is refused as well.
//////////////////////////////////////////////////////////////////////////////

#ifndef DARKEDEN_CHARACTER_NAME_H
#define DARKEDEN_CHARACTER_NAME_H

#include <cstring>

// Names a player may not take.
inline constexpr int kReservedNameTokenCount = 10;
inline constexpr const char* const kReservedNameTokens[kReservedNameTokenCount] = {
    "NONE",
    // Korean staff titles, spelled as their UTF-8 bytes so a name that
    // contains one is refused byte for byte.
    "\xea\xb4\x80\xeb\xa6\xac\xec\x9e\x90", // gwallija, "administrator"
    "\xeb\x8f\x84\xec\x9a\xb0\xeb\xaf\xb8", // doumi, "helper"
    "\xeb\x8b\xb4\xeb\x8b\xb9\xec\x9e\x90", // damdangja, "person in charge"
    "\xec\x9a\xb4\xec\x98\x81",             // unyeong, "operations"
    "\xea\xb8\xb0\xed\x9a\x8d\xec\x9e\x90", // gihoekja, "game designer"
    "\xea\xb0\x9c\xeb\xb0\x9c\xec\x9e\x90", // gaebalja, "developer"
    "\xed\x85\x8c\xec\x8a\xa4\xed\x84\xb0", // teseuteo, "tester"
    "\xec\xa7\x81\xec\x9b\x90",             // jigwon, "staff member"
    "GM"};

// Is the name free of the reserved tokens? A C string: the caller keeps
// NUL bytes out of it.
inline bool isAvailableID(const char* pID) {
    for (int i = 0; i < kReservedNameTokenCount; i++) {
        if (strstr(pID, kReservedNameTokens[i]) != NULL)
            return false;
    }
    return true;
}

#endif // DARKEDEN_CHARACTER_NAME_H
