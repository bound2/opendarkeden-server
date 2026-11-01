//////////////////////////////////////////////////////////////////////////////
// Filename    : DatagramAdmission.h
// Description : Which sender a datagram is admitted from, by the link its
//               packet rides (docs/protocol/datagram-sources.md). A
//               datagram socket binds every address and authenticates
//               nothing, so each datagram loop asks this gate before it
//               reads a datagram: a client's port check from anywhere, a
//               loginserver's packets only from the configured loginserver,
//               a gameserver's only from the gameservers the catalogue
//               lists. A refused datagram is counted, reported and dropped
//               unread.
//////////////////////////////////////////////////////////////////////////////

#ifndef DARKEDEN_DATAGRAM_ADMISSION_H
#define DARKEDEN_DATAGRAM_ADMISSION_H

#include <atomic>
#include <cstdint>
#include <functional>
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

// The senders a socket admits each server link from, as the dotted-quad
// addresses Datagram::getHost answers.
struct DatagramPeers {
    std::vector<std::string> loginServers; // LoginToGame
    std::vector<std::string> gameServers;  // GameToLogin and GameToGame
};

// Whether a datagram carrying `packetID` from `host` is admitted.
bool admitsDatagram(PacketID_t packetID, const std::string& host, const DatagramPeers& peers) noexcept;

// The addresses a configuration's DatagramPeerIPs names, comma-separated and
// trimmed, for a deployment whose servers' datagrams arrive from addresses
// neither the configuration nor the catalogue names (a multi-homed host, a
// NAT). They are admitted for every server link, never for a client's.
std::vector<std::string> configuredDatagramPeers(const Properties& config);

// A gameserver's peers: the LoginServerIP its configuration names, the
// gameservers the catalogue lists, and DatagramPeerIPs for both. A
// configuration without LoginServerIP names no loginserver, so every
// loginserver datagram is refused.
DatagramPeers gameServerDatagramPeers(const Properties& config, const GameServerInfoManager& servers);

// A loginserver's peers: the gameservers the catalogue lists, and
// DatagramPeerIPs for both links.
DatagramPeers loginServerDatagramPeers(const Properties& config, const GameServerInfoManager& servers);

// The gate a datagram loop runs each received datagram through before
// reading it. The peers are asked for on each datagram, so a reloaded
// catalogue is seen at once; a refusal is reported with the packet id (or
// PACKET_MAX for a datagram too short to carry one) and the sender, and
// both outcomes are counted.
class DatagramGate {
public:
    using Peers = std::function<DatagramPeers()>;
    using Report = std::function<void(PacketID_t packetID, const std::string& host)>;

    DatagramGate(Peers peers, Report report);

    // True when the datagram may be read: its first two bytes name a
    // packet of a link its sender may use.
    bool admits(Datagram& datagram);

    std::uint64_t admitted() const noexcept {
        return m_Admitted.load();
    }
    std::uint64_t refused() const noexcept {
        return m_Refused.load();
    }

private:
    Peers m_Peers;
    Report m_Report;
    std::atomic<std::uint64_t> m_Admitted{0};
    std::atomic<std::uint64_t> m_Refused{0};
};

} // namespace de

#endif // DARKEDEN_DATAGRAM_ADMISSION_H
