//////////////////////////////////////////////////////////////////////////////
// Filename    : PasswordHash.cpp
// Description : Argon2id hashing and verification of account passwords
//////////////////////////////////////////////////////////////////////////////

#include "PasswordHash.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <optional>
#include <random>
#include <stdexcept>

#include <argon2.h>

namespace de::password {

namespace {

bool startsWith(std::string_view s, std::string_view prefix) {
    return s.substr(0, prefix.size()) == prefix;
}

// The argon2 variant named by an encoded string's prefix.
std::optional<argon2_type> typeOf(std::string_view stored) {
    if (startsWith(stored, "$argon2id$"))
        return Argon2_id;
    if (startsWith(stored, "$argon2i$"))
        return Argon2_i;
    if (startsWith(stored, "$argon2d$"))
        return Argon2_d;
    return std::nullopt;
}

// True when an encoded string carries exactly the parameters hash() uses.
bool usesCurrentParameters(std::string_view stored) {
    if (!startsWith(stored, "$argon2id$"))
        return false;

    // The parameter fields sit before the salt, well inside the first 64
    // characters; the copy gives sscanf a NUL-terminated string.
    const std::string head(stored.substr(0, 64));
    unsigned version = 0, memoryKiB = 0, timeCost = 0, parallelism = 0;
    if (std::sscanf(head.c_str(), "$argon2id$v=%u$m=%u,t=%u,p=%u$", &version, &memoryKiB, &timeCost, &parallelism) != 4)
        return false;

    return version == ARGON2_VERSION_NUMBER && memoryKiB == kMemoryKiB && timeCost == kTimeCost &&
           parallelism == kParallelism;
}

// Equality whose running time depends on the lengths only, not on where the
// first differing byte is.
bool constantTimeEqual(std::string_view a, std::string_view b) {
    if (a.size() != b.size())
        return false;

    volatile unsigned char diff = 0;
    for (std::size_t i = 0; i < a.size(); ++i)
        diff = diff | static_cast<unsigned char>(a[i] ^ b[i]);
    return diff == 0;
}

std::array<unsigned char, kSaltBytes> randomSalt() {
    std::random_device device;
    std::array<unsigned char, kSaltBytes> salt{};
    for (std::size_t i = 0; i < salt.size(); i += sizeof(unsigned int)) {
        const unsigned int word = device();
        std::memcpy(salt.data() + i, &word, std::min(sizeof(word), salt.size() - i));
    }
    return salt;
}

} // namespace

std::size_t encodedLength() {
    // argon2_encodedlen counts the terminating NUL.
    return argon2_encodedlen(kTimeCost, kMemoryKiB, kParallelism, kSaltBytes, kHashBytes, Argon2_id) - 1;
}

bool isHashed(std::string_view stored) {
    return startsWith(stored, "$argon2");
}

std::string hash(std::string_view password) {
    const std::array<unsigned char, kSaltBytes> salt = randomSalt();
    std::string encoded(encodedLength() + 1, '\0');

    const int rc = argon2id_hash_encoded(kTimeCost, kMemoryKiB, kParallelism, password.data(), password.size(),
                                         salt.data(), salt.size(), kHashBytes, encoded.data(), encoded.size());
    if (rc != ARGON2_OK)
        throw std::runtime_error(std::string("argon2id_hash_encoded: ") + argon2_error_message(rc));

    encoded.resize(std::strlen(encoded.c_str()));
    return encoded;
}

Verify verify(std::string_view stored, std::string_view password) {
    if (stored.empty())
        return Verify::Rejected;

    if (!isHashed(stored))
        return constantTimeEqual(stored, password) ? Verify::AcceptedRehash : Verify::Rejected;

    const std::optional<argon2_type> type = typeOf(stored);
    if (!type)
        return Verify::Rejected;

    // argon2_verify wants a NUL-terminated encoding; a decoding failure and
    // a mismatch are both a rejection.
    const std::string encoded(stored);
    if (argon2_verify(encoded.c_str(), password.data(), password.size(), *type) != ARGON2_OK)
        return Verify::Rejected;

    return usesCurrentParameters(stored) ? Verify::Accepted : Verify::AcceptedRehash;
}

} // namespace de::password
