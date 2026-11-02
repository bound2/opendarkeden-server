#include "PortCheck.h"

#include "CharacterName.h"
#include "Exception.h"
#include "repository/SessionRepository.h"

namespace de {

namespace {

// CGPortCheck::read's bound on the name.
const std::size_t kNameBytesMax = 20;

const char* describe(PortCheckRejection rejection) {
    switch (rejection) {
    case PortCheckRejection::EmptyName:
        return "empty name";
    case PortCheckRejection::NameTooLong:
        return "name longer than 20 bytes";
    case PortCheckRejection::ControlByte:
        return "control byte in name";
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
    for (size_t i = 0; i < name.size(); ++i) {
        const unsigned char c = (unsigned char)name[i];
        if (c < 0x20 || c == 0x7f)
            return PortCheckRejection::ControlByte;
    }
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
