#include "PortCheck.h"

#include "CGPortCheck.h"
#include "CharacterName.h"
#include "Exception.h"
#include "repository/SessionRepository.h"

namespace de {

namespace {

// CGPortCheck::read's bound on the name.
const std::size_t kNameBytesMax = CGPortCheckFactory::kMaxSize - szBYTE;

// Whether the bytes are well-formed UTF-8 (RFC 3629): a lead byte with
// its continuation bytes, no overlong form, no surrogate, nothing past
// U+10FFFF.
bool isUtf8(const std::string& bytes) {
    std::size_t i = 0;
    while (i < bytes.size()) {
        const unsigned char lead = (unsigned char)bytes[i];
        if (lead < 0x80) {
            i++;
            continue;
        }

        std::size_t length;
        unsigned char secondMin = 0x80;
        unsigned char secondMax = 0xBF;
        if (lead >= 0xC2 && lead <= 0xDF) {
            length = 2;
        } else if (lead == 0xE0) {
            length = 3;
            secondMin = 0xA0;
        } else if (lead >= 0xE1 && lead <= 0xEC) {
            length = 3;
        } else if (lead == 0xED) {
            length = 3;
            secondMax = 0x9F;
        } else if (lead >= 0xEE && lead <= 0xEF) {
            length = 3;
        } else if (lead == 0xF0) {
            length = 4;
            secondMin = 0x90;
        } else if (lead >= 0xF1 && lead <= 0xF3) {
            length = 4;
        } else if (lead == 0xF4) {
            length = 4;
            secondMax = 0x8F;
        } else {
            return false;
        }

        if (i + length > bytes.size())
            return false;
        const unsigned char second = (unsigned char)bytes[i + 1];
        if (second < secondMin || second > secondMax)
            return false;
        for (std::size_t k = 2; k < length; ++k) {
            const unsigned char continuation = (unsigned char)bytes[i + k];
            if (continuation < 0x80 || continuation > 0xBF)
                return false;
        }
        i += length;
    }
    return true;
}

const char* describe(PortCheckRejection rejection) {
    switch (rejection) {
    case PortCheckRejection::EmptyName:
        return "empty name";
    case PortCheckRejection::NameTooLong:
        return "name longer than 20 bytes";
    case PortCheckRejection::ControlByte:
        return "control byte in name";
    case PortCheckRejection::NotUtf8:
        return "name is no UTF-8";
    case PortCheckRejection::ReservedName:
        return "reserved token in name";
    }
    return "refused name";
}

} // namespace

std::optional<PortCheckRejection> portCheckNameRejection(const std::string& name) {
    if (name.empty())
        return PortCheckRejection::EmptyName;
    if (name.size() > kNameBytesMax)
        return PortCheckRejection::NameTooLong;
    for (std::size_t i = 0; i < name.size(); ++i) {
        const unsigned char c = (unsigned char)name[i];
        if (c < 0x20 || c == 0x7f)
            return PortCheckRejection::ControlByte;
    }
    if (!isUtf8(name))
        return PortCheckRejection::NotUtf8;
    // No NUL passed above, so the C string is the whole name.
    if (!isAvailableID(name.c_str()))
        return PortCheckRejection::ReservedName;
    return std::nullopt;
}

void recordPortCheck(SessionRepository& repository, const std::string& name, DWORD ip, uint port, int serverID) {
    if (const auto rejection = portCheckNameRejection(name))
        throw InvalidProtocolException(std::string("CGPortCheck: ") + describe(*rejection));

    repository.recordUserIP(name, ip, port, serverID);
}

} // namespace de
