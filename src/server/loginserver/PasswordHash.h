//////////////////////////////////////////////////////////////////////////////
// Filename    : PasswordHash.h
// Description : Argon2id hashing and verification of account passwords
//////////////////////////////////////////////////////////////////////////////

#ifndef __PASSWORD_HASH_H__
#define __PASSWORD_HASH_H__

#include <cstddef>
#include <string>

#include <string_view>

namespace de::password {

// Cost parameters for every hash written from now on: argon2id, 64 MiB,
// three passes, one lane. Verification takes the parameters from the stored
// string, so changing these needs no migration: a row hashed under other
// parameters verifies as AcceptedRehash and is rewritten on that login.
inline constexpr unsigned kTimeCost = 3;
inline constexpr unsigned kMemoryKiB = 64 * 1024;
inline constexpr unsigned kParallelism = 1;
inline constexpr std::size_t kSaltBytes = 16;
inline constexpr std::size_t kHashBytes = 32;

// Length of the string hash() produces with the parameters above, without
// the terminating NUL. The Player.Password column must hold at least this.
std::size_t encodedLength();

// True when a stored value is an argon2 encoded string ("$argon2..."),
// false for a legacy plaintext row.
bool isHashed(std::string_view stored);

// Encodes `password` under a fresh random salt. Throws std::runtime_error
// when argon2 reports a failure (memory allocation, or an input outside the
// library's limits).
std::string hash(std::string_view password);

enum class Verify {
    Rejected,       // wrong password, unknown encoding, or an empty stored value
    Accepted,       // argon2id match under the current parameters
    AcceptedRehash, // match, but the row should be rewritten with hash(): a
                    // legacy plaintext value, or a hash under other parameters
};

// Checks `password` against a stored value. A plaintext row is compared in
// constant time; a hashed row is verified by argon2 with whatever type and
// parameters its encoding names.
Verify verify(std::string_view stored, std::string_view password);

} // namespace de::password

#endif
