// The gate each datagram loop runs a received datagram through before
// reading it (DatagramAdmission.h): a client's port check is admitted from
// anywhere, a loginserver's packets only from the configured loginserver,
// a gameserver's only from the gameservers the catalogue lists, and
// nothing else from anyone. The gate peeks the id, so a refused datagram's
// body is never parsed.

#include <cstring>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "Datagram.h"
#include "DatagramAdmission.h"
#include "GameServerInfoManager.h"
#include "Properties.h"
#include "repository/ServerInfoRepository.h"

namespace {

class CatalogueRepository : public ServerInfoRepository {
public:
    bool loadMaxServerGroupID(int& value) override {
        value = maxGroup;
        return true;
    }
    bool loadMaxWorldID(int& value) override {
        value = maxWorld;
        return true;
    }
    std::vector<ServerInfoRow> loadServers() override {
        return rows;
    }
    std::vector<ServerInfoNonPKRow> loadNonPKServers() override {
        return {};
    }
    std::vector<ServerInfoCastleStatRow> loadCastleStats() override {
        return {};
    }
    std::vector<ServerInfoWorldRow> loadWorlds() override {
        std::abort();
    }

    int maxWorld = 7;
    int maxGroup = 9;
    std::vector<ServerInfoRow> rows = {{1, "First", "192.0.2.9", 9991, 9992, 7, 9, SERVER_FREE},
                                       {2, "Second", "192.0.2.10", 9993, 9994, 7, 9, SERVER_FREE},
                                       {1, "Other world", "192.0.2.11", 9995, 9996, 3, 1, SERVER_FREE}};
};

de::DatagramPeers peers(std::vector<std::string> loginServers, std::vector<std::string> gameServers) {
    de::DatagramPeers result;
    result.loginServers = std::move(loginServers);
    result.gameServers = std::move(gameServers);
    return result;
}

// A received datagram as the socket hands it to the loop: the raw bytes
// and the sender's address. Filled in place, since a Datagram owns its
// buffer and is not copied.
void receive(Datagram& datagram, const std::vector<char>& bytes, const std::string& host) {
    std::vector<char> copy = bytes;
    datagram.setData(copy.data(), static_cast<uint>(copy.size()));
    datagram.setHost(host);
    datagram.setPort(9997);
}

std::vector<char> headerOf(PacketID_t packetID) {
    std::vector<char> bytes(szPacketID + szPacketSize, 0);
    std::memcpy(bytes.data(), &packetID, szPacketID);
    return bytes;
}

} // namespace

TEST(DatagramLink, EveryDatagramIdHasALinkAndNoOtherIdDoes) {
    Datagram datagram;
    for (PacketID_t id = 0; id < Packet::PACKET_MAX; ++id) {
        const bool isDatagram = datagram.isDatagram(id);
        const bool hasLink = de::datagramLinkOf(id) != de::DatagramLink::None;
        EXPECT_EQ(isDatagram, hasLink) << "packet id " << id;
    }
    EXPECT_EQ(de::datagramLinkOf(Packet::PACKET_CG_PORT_CHECK), de::DatagramLink::Client);
    EXPECT_EQ(de::datagramLinkOf(Packet::PACKET_LG_KICK_CHARACTER), de::DatagramLink::LoginToGame);
    EXPECT_EQ(de::datagramLinkOf(Packet::PACKET_GL_KICK_VERIFY), de::DatagramLink::GameToLogin);
    EXPECT_EQ(de::datagramLinkOf(Packet::PACKET_GM_SERVER_INFO), de::DatagramLink::GameToLogin);
    EXPECT_EQ(de::datagramLinkOf(Packet::PACKET_GG_COMMAND), de::DatagramLink::GameToGame);
    EXPECT_EQ(de::datagramLinkOf(Packet::PACKET_MAX), de::DatagramLink::None);
}

TEST(DatagramAdmission, AClientsPortCheckIsAdmittedFromAnywhere) {
    EXPECT_TRUE(de::admitsDatagram(Packet::PACKET_CG_PORT_CHECK, "198.51.100.7", peers({}, {})));
    EXPECT_TRUE(de::admitsDatagram(Packet::PACKET_CG_PORT_CHECK, "192.0.2.1", peers({"192.0.2.1"}, {})));
}

TEST(DatagramAdmission, ALoginserversPacketIsAdmittedOnlyFromTheConfiguredLoginserver) {
    const auto configured = peers({"192.0.2.1"}, {"192.0.2.9"});
    for (const PacketID_t id :
         {Packet::PACKET_LG_KICK_CHARACTER, Packet::PACKET_LG_INCOMING_CONNECTION,
          Packet::PACKET_LG_INCOMING_CONNECTION_OK, Packet::PACKET_LG_INCOMING_CONNECTION_ERROR}) {
        EXPECT_TRUE(de::admitsDatagram(id, "192.0.2.1", configured)) << "packet id " << id;
        EXPECT_FALSE(de::admitsDatagram(id, "192.0.2.2", configured)) << "packet id " << id;
        EXPECT_FALSE(de::admitsDatagram(id, "192.0.2.9", configured)) << "a gameserver, packet id " << id;
        EXPECT_FALSE(de::admitsDatagram(id, "192.0.2.1", peers({}, {"192.0.2.1"})))
            << "no loginserver named, packet id " << id;
    }
}

TEST(DatagramAdmission, AGameserversPacketIsAdmittedOnlyFromAListedGameserver) {
    const auto configured = peers({"192.0.2.1"}, {"192.0.2.9", "192.0.2.10"});
    for (const PacketID_t id :
         {Packet::PACKET_GL_INCOMING_CONNECTION, Packet::PACKET_GL_INCOMING_CONNECTION_OK,
          Packet::PACKET_GL_INCOMING_CONNECTION_ERROR, Packet::PACKET_GL_KICK_VERIFY, Packet::PACKET_GM_SERVER_INFO,
          Packet::PACKET_GG_COMMAND, Packet::PACKET_GG_GUILD_CHAT, Packet::PACKET_GG_SERVER_CHAT}) {
        EXPECT_TRUE(de::admitsDatagram(id, "192.0.2.9", configured)) << "packet id " << id;
        EXPECT_TRUE(de::admitsDatagram(id, "192.0.2.10", configured)) << "packet id " << id;
        EXPECT_FALSE(de::admitsDatagram(id, "192.0.2.1", configured)) << "the loginserver, packet id " << id;
        EXPECT_FALSE(de::admitsDatagram(id, "198.51.100.7", configured)) << "packet id " << id;
        EXPECT_FALSE(de::admitsDatagram(id, "192.0.2.9", peers({"192.0.2.9"}, {})))
            << "no gameserver listed, packet id " << id;
    }
}

TEST(DatagramAdmission, AnIdThatIsNoDatagramIsRefusedFromEveryone) {
    const auto configured = peers({"192.0.2.1"}, {"192.0.2.9"});
    EXPECT_FALSE(de::admitsDatagram(Packet::PACKET_CG_SAY, "192.0.2.1", configured));
    EXPECT_FALSE(de::admitsDatagram(Packet::PACKET_CG_SAY, "192.0.2.9", configured));
    EXPECT_FALSE(de::admitsDatagram(Packet::PACKET_MAX, "192.0.2.9", configured));
}

TEST(DatagramPeers, TheGameserversPeersAreTheConfiguredLoginserverAndTheCatalogue) {
    Properties config;
    config.setProperty("LoginServerIP", "192.0.2.1");
    CatalogueRepository repository;
    GameServerInfoManager servers;
    servers.load(repository);

    const de::DatagramPeers peers = de::gameServerDatagramPeers(config, servers);
    EXPECT_EQ(peers.loginServers, (std::vector<std::string>{"192.0.2.1"}));
    EXPECT_EQ(peers.gameServers, (std::vector<std::string>{"192.0.2.10", "192.0.2.11", "192.0.2.9"}));
}

TEST(DatagramPeers, AConfigurationWithoutALoginserverNamesNone) {
    Properties config;
    GameServerInfoManager servers;

    const de::DatagramPeers peers = de::gameServerDatagramPeers(config, servers);
    EXPECT_TRUE(peers.loginServers.empty());
    EXPECT_TRUE(peers.gameServers.empty());
}

TEST(DatagramPeers, TheLoginserversPeersAreTheCataloguesGameservers) {
    Properties config;
    CatalogueRepository repository;
    repository.rows.push_back({3, "Same address", "192.0.2.9", 9997, 9998, 7, 9, SERVER_FREE});
    GameServerInfoManager servers;
    servers.load(repository);

    const de::DatagramPeers peers = de::loginServerDatagramPeers(config, servers);
    EXPECT_TRUE(peers.loginServers.empty());
    EXPECT_EQ(peers.gameServers, (std::vector<std::string>{"192.0.2.10", "192.0.2.11", "192.0.2.9"}));
}

TEST(DatagramPeers, TheConfiguredAddressesAreTrimmedSortedAndDeduplicated) {
    Properties config;
    config.setProperty("DatagramPeerIPs", " 10.0.0.6 ,10.0.0.5,, 10.0.0.6\t,");
    EXPECT_EQ(de::configuredDatagramPeers(config), (std::vector<std::string>{"10.0.0.5", "10.0.0.6"}));

    Properties none;
    EXPECT_TRUE(de::configuredDatagramPeers(none).empty());
    none.setProperty("DatagramPeerIPs", " , ");
    EXPECT_TRUE(de::configuredDatagramPeers(none).empty());
}

TEST(DatagramPeers, TheConfiguredAddressesJoinBothServerLinksOnEitherServer) {
    Properties config;
    config.setProperty("LoginServerIP", "192.0.2.1");
    config.setProperty("DatagramPeerIPs", "10.0.0.5");
    CatalogueRepository repository;
    GameServerInfoManager servers;
    servers.load(repository);

    const de::DatagramPeers onTheGameserver = de::gameServerDatagramPeers(config, servers);
    EXPECT_EQ(onTheGameserver.loginServers, (std::vector<std::string>{"10.0.0.5", "192.0.2.1"}));
    EXPECT_EQ(onTheGameserver.gameServers,
              (std::vector<std::string>{"10.0.0.5", "192.0.2.10", "192.0.2.11", "192.0.2.9"}));

    const de::DatagramPeers onTheLoginserver = de::loginServerDatagramPeers(config, servers);
    EXPECT_EQ(onTheLoginserver.loginServers, (std::vector<std::string>{"10.0.0.5"}));
    EXPECT_EQ(onTheLoginserver.gameServers,
              (std::vector<std::string>{"10.0.0.5", "192.0.2.10", "192.0.2.11", "192.0.2.9"}));

    // A client's port check needs no address; a server link admits the
    // configured address like any peer's.
    EXPECT_TRUE(de::admitsDatagram(Packet::PACKET_GG_COMMAND, "10.0.0.5", onTheGameserver));
    EXPECT_TRUE(de::admitsDatagram(Packet::PACKET_LG_KICK_CHARACTER, "10.0.0.5", onTheGameserver));
    EXPECT_FALSE(de::admitsDatagram(Packet::PACKET_GG_COMMAND, "10.0.0.7", onTheGameserver));
}

TEST(DatagramGate, AnAdmittedDatagramIsCountedAndARefusedOneReportedUnread) {
    std::vector<std::pair<PacketID_t, std::string>> reports;
    de::DatagramGate gate([] { return peers({"192.0.2.1"}, {"192.0.2.9"}); },
                          [&](PacketID_t id, const std::string& host) { reports.emplace_back(id, host); });

    Datagram fromTheLoginserver;
    receive(fromTheLoginserver, headerOf(Packet::PACKET_LG_KICK_CHARACTER), "192.0.2.1");
    EXPECT_TRUE(gate.admits(fromTheLoginserver));

    Datagram fromElsewhere;
    receive(fromElsewhere, headerOf(Packet::PACKET_LG_KICK_CHARACTER), "198.51.100.7");
    EXPECT_FALSE(gate.admits(fromElsewhere));

    Datagram aCommandFromTheLoginserver;
    receive(aCommandFromTheLoginserver, headerOf(Packet::PACKET_GG_COMMAND), "192.0.2.1");
    EXPECT_FALSE(gate.admits(aCommandFromTheLoginserver));

    Datagram aPortCheck;
    receive(aPortCheck, headerOf(Packet::PACKET_CG_PORT_CHECK), "198.51.100.7");
    EXPECT_TRUE(gate.admits(aPortCheck));

    EXPECT_EQ(gate.admitted(), 2u);
    EXPECT_EQ(gate.refused(), 2u);
    ASSERT_EQ(reports.size(), 2u);
    EXPECT_EQ(reports[0].first, Packet::PACKET_LG_KICK_CHARACTER);
    EXPECT_EQ(reports[0].second, "198.51.100.7");
    EXPECT_EQ(reports[1].first, Packet::PACKET_GG_COMMAND);
    EXPECT_EQ(reports[1].second, "192.0.2.1");
}

// The socket hands the loop a datagram only for a positive receive, so the
// shortest one it can see is one byte, too short for an id.
TEST(DatagramGate, ADatagramTooShortForAnIdIsRefusedAsNoPacket) {
    std::vector<std::pair<PacketID_t, std::string>> reports;
    de::DatagramGate gate([] { return peers({"192.0.2.1"}, {"192.0.2.9"}); },
                          [&](PacketID_t id, const std::string& host) { reports.emplace_back(id, host); });

    Datagram oneByte;
    receive(oneByte, std::vector<char>{1}, "192.0.2.1");
    EXPECT_FALSE(gate.admits(oneByte));

    EXPECT_EQ(gate.admitted(), 0u);
    EXPECT_EQ(gate.refused(), 1u);
    ASSERT_EQ(reports.size(), 1u);
    EXPECT_EQ(reports[0].first, Packet::PACKET_MAX);
    EXPECT_EQ(reports[0].second, "192.0.2.1");
}

TEST(DatagramGate, ThePeersAreAskedForOnEveryDatagram) {
    std::vector<std::string> loginServers;
    de::DatagramGate gate([&] { return peers(loginServers, {}); }, [](PacketID_t, const std::string&) {});

    Datagram datagram;
    receive(datagram, headerOf(Packet::PACKET_LG_KICK_CHARACTER), "192.0.2.1");
    EXPECT_FALSE(gate.admits(datagram));
    loginServers.push_back("192.0.2.1");
    EXPECT_TRUE(gate.admits(datagram));
}
