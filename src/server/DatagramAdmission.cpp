//////////////////////////////////////////////////////////////////////////////
// Filename    : DatagramAdmission.cpp
// Description : see DatagramAdmission.h.
//////////////////////////////////////////////////////////////////////////////

#include "DatagramAdmission.h"

#include <algorithm>
#include <cstring>
#include <exception>
#include <utility>

#include <arpa/inet.h>

#include "Datagram.h"
#include "Exception.h"
#include "GameServerInfo.h"
#include "GameServerInfoManager.h"
#include "Properties.h"

namespace de {

DatagramLink datagramLinkOf(PacketID_t packetID) noexcept {
    switch (packetID) {
    case Packet::PACKET_CG_PORT_CHECK:
        return DatagramLink::Client;
    case Packet::PACKET_LG_KICK_CHARACTER:
    case Packet::PACKET_LG_INCOMING_CONNECTION:
    case Packet::PACKET_LG_INCOMING_CONNECTION_ERROR:
    case Packet::PACKET_LG_INCOMING_CONNECTION_OK:
        return DatagramLink::LoginToGame;
    case Packet::PACKET_GL_INCOMING_CONNECTION:
    case Packet::PACKET_GL_INCOMING_CONNECTION_ERROR:
    case Packet::PACKET_GL_INCOMING_CONNECTION_OK:
    case Packet::PACKET_GL_KICK_VERIFY:
    case Packet::PACKET_GM_SERVER_INFO:
        return DatagramLink::GameToLogin;
    case Packet::PACKET_GG_COMMAND:
    case Packet::PACKET_GG_GUILD_CHAT:
    case Packet::PACKET_GG_SERVER_CHAT:
        return DatagramLink::GameToGame;
    default:
        return DatagramLink::None;
    }
}

namespace {

bool listed(const std::vector<std::string>& hosts, const std::string& host) noexcept {
    for (const std::string& listedHost : hosts) {
        if (listedHost == host)
            return true;
    }
    return false;
}

void sortUnique(std::vector<std::string>& addresses) {
    std::sort(addresses.begin(), addresses.end());
    addresses.erase(std::unique(addresses.begin(), addresses.end()), addresses.end());
}

// Every distinct address the catalogue lists, ascending; none before a load.
std::vector<std::string> catalogueAddresses(const GameServerInfoManager& servers) {
    std::vector<std::string> addresses;
    HashMapGameServerInfo** view = servers.getGameServerInfos();
    if (view == nullptr)
        return addresses;

    const int worlds = servers.getMaxWorldID();
    const int groups = servers.getMaxServerGroupID();
    for (int worldID = 0; worldID < worlds; ++worldID) {
        HashMapGameServerInfo* tables = view[worldID];
        if (tables == nullptr)
            continue;
        for (int groupID = 0; groupID < groups; ++groupID) {
            for (const auto& entry : tables[groupID]) {
                if (entry.second != nullptr)
                    addresses.push_back(entry.second->getIP());
            }
        }
    }
    sortUnique(addresses);
    return addresses;
}

bool isDottedQuad(const std::string& address) noexcept {
    in_addr parsed;
    return inet_pton(AF_INET, address.c_str(), &parsed) == 1;
}

void join(std::vector<std::string>& into, const std::vector<std::string>& addresses) {
    into.insert(into.end(), addresses.begin(), addresses.end());
    sortUnique(into);
}

} // namespace

bool admitsDatagram(PacketID_t packetID, const std::string& host, const DatagramPeers& peers) noexcept {
    switch (datagramLinkOf(packetID)) {
    case DatagramLink::Client:
        return peers.client;
    case DatagramLink::LoginToGame:
        return peers.loginToGame && listed(peers.loginServers, host);
    case DatagramLink::GameToLogin:
        return peers.gameToLogin && listed(peers.gameServers, host);
    case DatagramLink::GameToGame:
        return peers.gameToGame && listed(peers.gameServers, host);
    case DatagramLink::None:
        break;
    }
    return false;
}

std::vector<std::string> configuredDatagramPeers(const Properties& config) {
    std::vector<std::string> addresses;
    if (!config.hasKey("DatagramPeerIPs"))
        return addresses;

    const std::string list = config.getProperty("DatagramPeerIPs");
    std::string::size_type start = 0;
    while (start <= list.size()) {
        std::string::size_type end = list.find(',', start);
        if (end == std::string::npos)
            end = list.size();
        std::string::size_type first = start;
        std::string::size_type last = end;
        while (first < last && (list[first] == ' ' || list[first] == '\t'))
            ++first;
        while (last > first && (list[last - 1] == ' ' || list[last - 1] == '\t'))
            --last;
        if (first < last) {
            const std::string address = list.substr(first, last - first);
            if (!isDottedQuad(address))
                throw Error("DatagramPeerIPs: not an IPv4 address: " + address);
            addresses.push_back(address);
        }
        start = end + 1;
    }
    sortUnique(addresses);
    return addresses;
}

DatagramPeers gameServerDatagramPeers(const Properties& config, const GameServerInfoManager& servers) {
    DatagramPeers peers;
    peers.client = true;
    peers.loginToGame = true;
    peers.gameToGame = true;
    if (config.hasKey("LoginServerIP"))
        peers.loginServers.push_back(config.getProperty("LoginServerIP"));
    peers.gameServers = catalogueAddresses(servers);
    const std::vector<std::string> configured = configuredDatagramPeers(config);
    join(peers.loginServers, configured);
    join(peers.gameServers, configured);
    return peers;
}

DatagramPeers loginServerDatagramPeers(const Properties& config, const GameServerInfoManager& servers) {
    DatagramPeers peers;
    peers.gameToLogin = true;
    peers.gameServers = catalogueAddresses(servers);
    join(peers.gameServers, configuredDatagramPeers(config));
    return peers;
}

DatagramGate::DatagramGate(Peers peers, Report report) : m_Peers(std::move(peers)), m_Report(std::move(report)) {}

const DatagramPeers& DatagramGate::peers() {
    std::lock_guard<std::mutex> lock(m_PeersMutex);
    if (!m_Cached.has_value())
        m_Cached = m_Peers();
    return *m_Cached;
}

bool DatagramGate::admits(Datagram& datagram) noexcept {
    PacketID_t packetID = Packet::PACKET_MAX;
    bool admitted = false;
    try {
        if (datagram.getLength() >= szPacketID) {
            std::memcpy(&packetID, datagram.getData(), szPacketID);
            if (datagramLinkOf(packetID) != DatagramLink::None)
                admitted = admitsDatagram(packetID, datagram.getHost(), peers());
        }
    } catch (...) {
        // The peers could not be built or the sender not named: refused.
        admitted = false;
    }

    if (admitted) {
        ++m_Admitted;
        return true;
    }
    ++m_Refused;
    if (m_Report) {
        try {
            m_Report(packetID, datagram.getHost());
        } catch (...) {
            // The datagram is dropped whether or not the line is written.
        }
    }
    return false;
}

} // namespace de
