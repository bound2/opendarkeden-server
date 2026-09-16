//////////////////////////////////////////////////////////////////////////////
// Filename    : CommandRouter.h
// Description : The table a GM chat command is looked up in: its name, the
//               permission level it demands and the body that answers it.
//               Knows nothing of zones, creatures or managers - it carries
//               the two pointers a command body needs and matches names.
//////////////////////////////////////////////////////////////////////////////

#ifndef __GM_COMMAND_ROUTER_H__
#define __GM_COMMAND_ROUTER_H__

#include <string>
#include <vector>

#include <string_view>

class Creature;
class GamePlayer;

namespace de::gm {

// What a command demands of whoever typed it, ordered the way Competence
// (src/Core/types/CreatureTypes.h) is: GOD is the strongest level and
// Everyone the weakest.
//
//   God      - the creature's competence is GOD.
//   DM       - GOD or DM.
//   Helper   - anything that is not a PLAYER, so GOD, DM or HELPER.
//   Everyone - no gate at all.
enum class Permission : unsigned char { God = 0, DM = 1, Helper = 2, Everyone = 3 };

// May a caller of this level run a command that declares that one?
inline bool admits(Permission caller, Permission required) {
    return static_cast<unsigned char>(caller) <= static_cast<unsigned char>(required);
}

// What a command body is handed. msg is the whole chat message and i the
// offset of the '*' that introduces the command, so a body reads its
// arguments out of msg from i the way it always has. caller is the level the
// creature speaks with; it travels with the message so a command that
// dispatches a message of its own does not have to work it out again.
struct CommandContext {
    Creature* pCreature;
    GamePlayer* pPlayer;
    const std::string& msg;
    int i;
    Permission caller;
};

using CommandHandler = void (*)(const CommandContext&);

struct Command {
    // The text that follows the '*'.
    std::string name;
    // How many characters of the message are compared against it. Normally
    // the length of the name, which makes the match a prefix match: "*savex"
    // runs *save. A command whose name is longer than the length it is
    // compared over can never be reached.
    std::string::size_type matchLength;
    Permission permission;
    CommandHandler handler;
};

// Commands are answered in registration order: the first whose name matches
// and whose gate the caller clears runs, and nothing else does. Where one
// name is a prefix of another, the one registered first wins for both.
class CommandRouter {
public:
    // Compare over the whole name.
    void add(std::string_view name, Permission permission, CommandHandler handler);
    // Compare over matchLength characters only.
    void add(std::string_view name, std::string::size_type matchLength, Permission permission, CommandHandler handler);

    // Runs the command the message names, if the caller may run it. Answers
    // false when no command ran - an unknown name, or one every matching
    // command's gate refused.
    bool dispatch(const CommandContext& context) const;

    const std::vector<Command>& commands() const {
        return m_Commands;
    }

private:
    std::vector<Command> m_Commands;
};

} // namespace de::gm

#endif // __GM_COMMAND_ROUTER_H__
