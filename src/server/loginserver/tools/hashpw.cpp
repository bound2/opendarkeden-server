//////////////////////////////////////////////////////////////////////////////
// hashpw: prints the argon2id hash of a password for the Player.Password
// column, or checks a password against a stored value.
//
//   hashpw                    read the password from stdin, print its hash
//   hashpw --verify <stored>  read the password from stdin; exit 0 on a
//                             match, 1 on a mismatch
//
// The password is read from stdin (first line) rather than taken as an
// argument so it stays out of shell history and process listings.
//////////////////////////////////////////////////////////////////////////////

#include <exception>
#include <iostream>
#include <string>

#include <string_view>

#include "PasswordHash.h"

namespace {

int usage() {
    std::cerr << "usage: hashpw            (password on stdin; prints the hash)\n"
                 "       hashpw --verify <stored-value>\n";
    return 2;
}

} // namespace

int main(int argc, char** argv) {
    std::string password;
    if (!std::getline(std::cin, password)) {
        std::cerr << "hashpw: no password on stdin\n";
        return 2;
    }
    if (!password.empty() && password.back() == '\r')
        password.pop_back();
    if (password.empty()) {
        std::cerr << "hashpw: refusing an empty password\n";
        return 2;
    }

    try {
        if (argc == 1) {
            std::cout << de::password::hash(password) << '\n';
            return 0;
        }

        if (argc == 3 && std::string_view(argv[1]) == "--verify") {
            switch (de::password::verify(argv[2], password)) {
            case de::password::Verify::Accepted:
                std::cout << "match\n";
                return 0;
            case de::password::Verify::AcceptedRehash:
                std::cout << "match (legacy or outdated encoding; the loginserver rehashes it on login)\n";
                return 0;
            case de::password::Verify::Rejected:
                std::cout << "mismatch\n";
                return 1;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "hashpw: " << e.what() << '\n';
        return 3;
    }

    return usage();
}
