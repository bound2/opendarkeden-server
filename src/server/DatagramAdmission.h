//////////////////////////////////////////////////////////////////////////////
// Filename    : DatagramAdmission.h
// Description : Which sender a datagram is admitted from, by the link its
//               packet rides (docs/protocol/datagram-sources.md). A
//               datagram socket binds every address and authenticates
//               nothing, so each datagram loop asks this gate before it
//               reads a datagram: a client's port check from anywhere on
//               the gameserver, the loginserver's packets only from the
//               configured loginserver, a gameserver's only from the
//               gameservers the catalogue lists, and a link the server does
//               not receive from no one. A refused datagram is counted,
//               reported and dropped unread. The check is of the source
//               address the socket reported, which a sender on the network
//               path can forge; it is not authentication.
//////////////////////////////////////////////////////////////////////////////

#ifndef DARKEDEN_DATAGRAM_ADMISSION_H
#define DARKEDEN_DATAGRAM_ADMISSION_H

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "Packet.h"

class Datagram;
class GameServerInfoManager;
class Properties;

namespace de {

// The link a datagram's packet rides, which says who sends it.
enum class DatagramLink : unsigned char { None, Client, LoginToGame, GameToLogin, GameToGame };

// The link of every id Datagram::isDatagram admits; None for every other id.
DatagramLink datagramLinkOf(PacketID_t packetID) noexcept;

// What one socket admits: the links its server receives, and the senders
// each server link is admitted from, as the dotted-quad addresses
// Datagram::getHost answers. A link the server does not receive is refused
// from everyone, so a packet it has no handler for is never parsed.
struct DatagramPeers {
    bool client = false;      // CGPortCheck, from anywhere
    bool loginToGame = false; // the LG packets, from loginServers
    bool gameToLogin = false; // the GL packets and GMServerInfo, from gameServers
    bool gameToGame = false;  // the GG packets, from gameServers
    std::vector<std::string> loginServers;
    std::vector<std::string> gameServers;
};

// Whether a datagram carrying `packetID` from `host` is admitted.
bool admitsDatagram(PacketID_t packetID, const std::string& host, const DatagramPeers& peers) noexcept;

// The addresses a configuration's DatagramPeerIPs names, comma-separated,
// trimmed, sorted and distinct, for a deployment whose servers' datagrams
// arrive from addresses neither the configuration nor the catalogue names
// (a NAT or a proxy between the servers, a multi-homed host). Every entry
// must be an IPv4 dotted quad; Error names the first that is not, so a
// typo stops the server at startup instead of refusing a peer silently.
std::vector<std::string> configuredDatagramPeers(const Properties& config);

// A gameserver's socket: the port check from anywhere, the LG packets from
// the LoginServerIP its configuration names, the GG packets from the
// gameservers the catalogue lists, and DatagramPeerIPs for both server
// links. A configuration without LoginServerIP names no loginserver, so
// every loginserver datagram is refused.
DatagramPeers gameServerDatagramPeers(const Properties& config, const GameServerInfoManager& servers);

// A loginserver's socket: the GL packets and GMServerInfo from the
// gameservers the catalogue lists and DatagramPeerIPs; no client link and
// no GG packets, which the loginserver has no handler for.
DatagramPeers loginServerDatagramPeers(const Properties& config, const GameServerInfoManager& servers);

// The gate a datagram loop runs each received datagram through before
// reading it. The peers are asked for once, on the first datagram of a
// link the server receives, and kept: the configuration and the catalogue
// are loaded before the workers start and never reloaded. A refusal is
// reported with the packet id (PACKET_MAX for a datagram too short to
// carry one) and the sender, both outcomes are counted, and nothing the
// peers or the report throw escapes: a datagram the gate cannot judge is
// refused.
class DatagramGate {
public:
    using Peers = std::function<DatagramPeers()>;
    using Report = std::function<void(PacketID_t packetID, const std::string& host)>;

    DatagramGate(Peers peers, Report report);

    // True when the datagram may be read: its first two bytes name a
    // packet of a link its sender may use. The datagram's read position
    // is left where it was.
    bool admits(Datagram& datagram) noexcept;

    std::uint64_t admitted() const noexcept {
        return m_Admitted.load();
    }
    std::uint64_t refused() const noexcept {
        return m_Refused.load();
    }

private:
    const DatagramPeers& peers();

    Peers m_Peers;
    Report m_Report;
    std::mutex m_PeersMutex;
    std::optional<DatagramPeers> m_Cached;
    std::atomic<std::uint64_t> m_Admitted{0};
    std::atomic<std::uint64_t> m_Refused{0};
};

} // namespace de

#endif // DARKEDEN_DATAGRAM_ADMISSION_H
