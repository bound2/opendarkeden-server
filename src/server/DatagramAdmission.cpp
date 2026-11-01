//////////////////////////////////////////////////////////////////////////////
// Filename    : DatagramAdmission.cpp
// Description : see DatagramAdmission.h.
//////////////////////////////////////////////////////////////////////////////

#include "DatagramAdmission.h"

#include <algorithm>
#include <cstring>
#include <utility>

#include "Datagram.h"
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
    std::sort(addresses.begin(), addresses.end());
    addresses.erase(std::unique(addresses.begin(), addresses.end()), addresses.end());
    return addresses;
}

} // namespace

bool admitsDatagram(PacketID_t packetID, const std::string& host, const DatagramPeers& peers) noexcept {
    switch (datagramLinkOf(packetID)) {
    case DatagramLink::Client:
        return true;
    case DatagramLink::LoginToGame:
        return listed(peers.loginServers, host);
    case DatagramLink::GameToLogin:
    case DatagramLink::GameToGame:
        return listed(peers.gameServers, host);
    case DatagramLink::None:
        break;
    }
    return false;
}

namespace {

void sortUnique(std::vector<std::string>& addresses) {
    std::sort(addresses.begin(), addresses.end());
    addresses.erase(std::unique(addresses.begin(), addresses.end()), addresses.end());
}

// Every address the configuration names for both server links, added to the
// peers found elsewhere.
void addConfiguredPeers(DatagramPeers& peers, const Properties& config) {
    const std::vector<std::string> configured = configuredDatagramPeers(config);
    peers.loginServers.insert(peers.loginServers.end(), configured.begin(), configured.end());
    peers.gameServers.insert(peers.gameServers.end(), configured.begin(), configured.end());
    sortUnique(peers.loginServers);
    sortUnique(peers.gameServers);
}

} // namespace

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
        if (first < last)
            addresses.push_back(list.substr(first, last - first));
        start = end + 1;
    }
    sortUnique(addresses);
    return addresses;
}

DatagramPeers gameServerDatagramPeers(const Properties& config, const GameServerInfoManager& servers) {
    DatagramPeers peers;
    if (config.hasKey("LoginServerIP"))
        peers.loginServers.push_back(config.getProperty("LoginServerIP"));
    peers.gameServers = catalogueAddresses(servers);
    addConfiguredPeers(peers, config);
    return peers;
}

DatagramPeers loginServerDatagramPeers(const Properties& config, const GameServerInfoManager& servers) {
    DatagramPeers peers;
    peers.gameServers = catalogueAddresses(servers);
    addConfiguredPeers(peers, config);
    return peers;
}

DatagramGate::DatagramGate(Peers peers, Report report) : m_Peers(std::move(peers)), m_Report(std::move(report)) {}

bool DatagramGate::admits(Datagram& datagram) {
    PacketID_t packetID = Packet::PACKET_MAX;
    bool admitted = false;
    if (datagram.getLength() >= szPacketID) {
        std::memcpy(&packetID, datagram.getData(), szPacketID);
        admitted = admitsDatagram(packetID, datagram.getHost(), m_Peers());
    }

    if (admitted) {
        ++m_Admitted;
        return true;
    }
    ++m_Refused;
    if (m_Report)
        m_Report(packetID, datagram.getHost());
    return false;
}

} // namespace de
