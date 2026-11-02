// The gate each datagram loop runs a received datagram through before
// reading it (DatagramAdmission.h): a client's port check is admitted from
// anywhere on the gameserver, the loginserver's packets only from the
// configured loginserver, a gameserver's only from the gameservers the
// catalogue lists, a link the server does not receive from no one, and
// nothing else from anyone. The gate peeks the id, so a refused datagram's
// body is never parsed and an admitted one reads as it would have.

#include <cstdlib>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "CGPortCheck.h"
#include "Datagram.h"
#include "DatagramAdmission.h"
#include "DatagramPacket.h"
#include "Exception.h"
#include "GameServerInfoManager.h"
#include "KernelContext.h"
#include "PacketFactoryManager.h"
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

// A gameserver's socket with the given peers: every link it receives.
de::DatagramPeers gameserverPeers(std::vector<std::string> loginServers, std::vector<std::string> gameServers) {
    de::DatagramPeers result;
    result.client = true;
    result.loginToGame = true;
    result.gameToGame = true;
    result.loginServers = std::move(loginServers);
    result.gameServers = std::move(gameServers);
    return result;
}

// A loginserver's socket with the given peers: the GL link alone.
de::DatagramPeers loginserverPeers(std::vector<std::string> gameServers) {
    de::DatagramPeers result;
    result.gameToLogin = true;
    result.gameServers = std::move(gameServers);
    return result;
}

const std::vector<PacketID_t> kLoginToGame = {Packet::PACKET_LG_KICK_CHARACTER, Packet::PACKET_LG_INCOMING_CONNECTION,
                                              Packet::PACKET_LG_INCOMING_CONNECTION_OK,
                                              Packet::PACKET_LG_INCOMING_CONNECTION_ERROR};
const std::vector<PacketID_t> kGameToLogin = {
    Packet::PACKET_GL_INCOMING_CONNECTION, Packet::PACKET_GL_INCOMING_CONNECTION_OK,
    Packet::PACKET_GL_INCOMING_CONNECTION_ERROR, Packet::PACKET_GL_KICK_VERIFY, Packet::PACKET_GM_SERVER_INFO};
const std::vector<PacketID_t> kGameToGame = {Packet::PACKET_GG_COMMAND, Packet::PACKET_GG_GUILD_CHAT,
                                             Packet::PACKET_GG_SERVER_CHAT};

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

// The receive path builds packets through the factory table registered
// on the kernel context, which no other test in this binary installs.
void installPortCheckFactory() {
    static PacketFactoryManager* pPacketFactoryManager = NULL;
    if (pPacketFactoryManager == NULL) {
        pPacketFactoryManager = new PacketFactoryManager();
        pPacketFactoryManager->addFactory(new CGPortCheckFactory());
        de::kernelContext().setPacketFactoryManager(pPacketFactoryManager);
    }
}

} // namespace

TEST(DatagramLink, EveryDatagramIdHasALinkAndNoOtherIdDoes) {
    Datagram datagram;
    // A wire id is a u16, so every value it can hold is asked, PACKET_MAX
    // and above included.
    for (unsigned int id = 0; id <= 0xFFFF; ++id) {
        const PacketID_t packetID = static_cast<PacketID_t>(id);
        const bool isDatagram = datagram.isDatagram(packetID);
        const bool hasLink = de::datagramLinkOf(packetID) != de::DatagramLink::None;
        EXPECT_EQ(isDatagram, hasLink) << "packet id " << id;
    }
    EXPECT_EQ(de::datagramLinkOf(Packet::PACKET_CG_PORT_CHECK), de::DatagramLink::Client);
    for (const PacketID_t id : kLoginToGame)
        EXPECT_EQ(de::datagramLinkOf(id), de::DatagramLink::LoginToGame) << "packet id " << id;
    for (const PacketID_t id : kGameToLogin)
        EXPECT_EQ(de::datagramLinkOf(id), de::DatagramLink::GameToLogin) << "packet id " << id;
    for (const PacketID_t id : kGameToGame)
        EXPECT_EQ(de::datagramLinkOf(id), de::DatagramLink::GameToGame) << "packet id " << id;
}

TEST(DatagramAdmission, AClientsPortCheckIsAdmittedFromAnywhereOnTheGameserverOnly) {
    EXPECT_TRUE(de::admitsDatagram(Packet::PACKET_CG_PORT_CHECK, "198.51.100.7", gameserverPeers({}, {})));
    EXPECT_TRUE(de::admitsDatagram(Packet::PACKET_CG_PORT_CHECK, "192.0.2.1", gameserverPeers({"192.0.2.1"}, {})));
    EXPECT_FALSE(de::admitsDatagram(Packet::PACKET_CG_PORT_CHECK, "198.51.100.7", loginserverPeers({})));
    EXPECT_FALSE(de::admitsDatagram(Packet::PACKET_CG_PORT_CHECK, "192.0.2.9", loginserverPeers({"192.0.2.9"})));
}

TEST(DatagramAdmission, ALoginserversPacketIsAdmittedOnlyFromTheConfiguredLoginserver) {
    const auto gameserver = gameserverPeers({"192.0.2.1"}, {"192.0.2.9", "192.0.2.10", "192.0.2.11"});
    for (const PacketID_t id : kLoginToGame) {
        EXPECT_TRUE(de::admitsDatagram(id, "192.0.2.1", gameserver)) << "packet id " << id;
        EXPECT_FALSE(de::admitsDatagram(id, "192.0.2.2", gameserver)) << "packet id " << id;
        EXPECT_FALSE(de::admitsDatagram(id, "192.0.2.9", gameserver)) << "a gameserver, packet id " << id;
        // Neither a longer address nor a shorter one is the listed one.
        EXPECT_FALSE(de::admitsDatagram(id, "192.0.2.10", gameserver)) << "packet id " << id;
        EXPECT_FALSE(de::admitsDatagram(id, "192.0.2.", gameserver)) << "packet id " << id;
        EXPECT_FALSE(de::admitsDatagram(id, "192.0.2.1", gameserverPeers({}, {"192.0.2.1"})))
            << "no loginserver named, packet id " << id;
        // The loginserver's own socket never receives this link.
        EXPECT_FALSE(de::admitsDatagram(id, "192.0.2.1", loginserverPeers({"192.0.2.1"}))) << "packet id " << id;
    }
}

TEST(DatagramAdmission, AGameserversPacketToTheLoginserverIsAdmittedOnlyFromAListedGameserver) {
    const auto loginserver = loginserverPeers({"192.0.2.1", "192.0.2.9", "192.0.2.10"});
    for (const PacketID_t id : kGameToLogin) {
        EXPECT_TRUE(de::admitsDatagram(id, "192.0.2.9", loginserver)) << "packet id " << id;
        EXPECT_TRUE(de::admitsDatagram(id, "192.0.2.10", loginserver)) << "packet id " << id;
        EXPECT_FALSE(de::admitsDatagram(id, "192.0.2.100", loginserver)) << "packet id " << id;
        EXPECT_FALSE(de::admitsDatagram(id, "192.0.2", loginserver)) << "packet id " << id;
        EXPECT_FALSE(de::admitsDatagram(id, "198.51.100.7", loginserver)) << "packet id " << id;
        EXPECT_FALSE(de::admitsDatagram(id, "192.0.2.9", loginserverPeers({})))
            << "no gameserver listed, packet id " << id;
        // A gameserver's own socket never receives this link.
        EXPECT_FALSE(de::admitsDatagram(id, "192.0.2.9", gameserverPeers({"192.0.2.1"}, {"192.0.2.9"})))
            << "packet id " << id;
    }
}

TEST(DatagramAdmission, AGameserversPacketToAnotherGameserverIsAdmittedOnlyFromAListedGameserver) {
    const auto gameserver = gameserverPeers({"192.0.2.1"}, {"192.0.2.9", "192.0.2.10"});
    for (const PacketID_t id : kGameToGame) {
        EXPECT_TRUE(de::admitsDatagram(id, "192.0.2.9", gameserver)) << "packet id " << id;
        EXPECT_TRUE(de::admitsDatagram(id, "192.0.2.10", gameserver)) << "packet id " << id;
        EXPECT_FALSE(de::admitsDatagram(id, "192.0.2.1", gameserver)) << "the loginserver, packet id " << id;
        EXPECT_FALSE(de::admitsDatagram(id, "192.0.2.100", gameserver)) << "packet id " << id;
        EXPECT_FALSE(de::admitsDatagram(id, "198.51.100.7", gameserver)) << "packet id " << id;
        // The loginserver's socket never receives this link.
        EXPECT_FALSE(de::admitsDatagram(id, "192.0.2.9", loginserverPeers({"192.0.2.9"}))) << "packet id " << id;
    }
}

TEST(DatagramAdmission, AnIdThatIsNoDatagramIsRefusedFromEveryone) {
    const auto gameserver = gameserverPeers({"192.0.2.1"}, {"192.0.2.9"});
    EXPECT_FALSE(de::admitsDatagram(Packet::PACKET_CG_SAY, "192.0.2.1", gameserver));
    EXPECT_FALSE(de::admitsDatagram(Packet::PACKET_CG_SAY, "192.0.2.9", gameserver));
    EXPECT_FALSE(de::admitsDatagram(Packet::PACKET_MAX, "192.0.2.9", gameserver));
    EXPECT_FALSE(de::admitsDatagram(static_cast<PacketID_t>(0xFFFF), "192.0.2.9", gameserver));
}

TEST(DatagramPeers, TheGameserversSocketTakesItsLinksFromTheConfigurationAndTheCatalogue) {
    Properties config;
    config.setProperty("LoginServerIP", "192.0.2.1");
    CatalogueRepository repository;
    GameServerInfoManager servers;
    servers.load(repository);

    const de::DatagramPeers peers = de::gameServerDatagramPeers(config, servers);
    EXPECT_TRUE(peers.client);
    EXPECT_TRUE(peers.loginToGame);
    EXPECT_TRUE(peers.gameToGame);
    EXPECT_FALSE(peers.gameToLogin);
    EXPECT_EQ(peers.loginServers, (std::vector<std::string>{"192.0.2.1"}));
    EXPECT_EQ(peers.gameServers, (std::vector<std::string>{"192.0.2.10", "192.0.2.11", "192.0.2.9"}));
}

TEST(DatagramPeers, AConfigurationWithoutALoginserverNamesNone) {
    Properties config;
    GameServerInfoManager servers;

    const de::DatagramPeers peers = de::gameServerDatagramPeers(config, servers);
    EXPECT_TRUE(peers.loginServers.empty());
    EXPECT_TRUE(peers.gameServers.empty());
    for (const PacketID_t id : kLoginToGame)
        EXPECT_FALSE(de::admitsDatagram(id, "192.0.2.1", peers)) << "packet id " << id;
}

TEST(DatagramPeers, TheLoginserversSocketTakesTheCataloguesGameserversAlone) {
    Properties config;
    CatalogueRepository repository;
    repository.rows.push_back({3, "Same address", "192.0.2.9", 9997, 9998, 7, 9, SERVER_FREE});
    GameServerInfoManager servers;
    servers.load(repository);

    const de::DatagramPeers peers = de::loginServerDatagramPeers(config, servers);
    EXPECT_FALSE(peers.client);
    EXPECT_FALSE(peers.loginToGame);
    EXPECT_TRUE(peers.gameToLogin);
    EXPECT_FALSE(peers.gameToGame);
    EXPECT_TRUE(peers.loginServers.empty());
    EXPECT_EQ(peers.gameServers, (std::vector<std::string>{"192.0.2.10", "192.0.2.11", "192.0.2.9"}));
}

TEST(DatagramPeers, ACatalogueRowNamedByHostnameIsListedAsWrittenAndMatchesNoAddress) {
    Properties config;
    CatalogueRepository repository;
    repository.rows = {{1, "By name", "gameserver.example", 9991, 9992, 7, 9, SERVER_FREE}};
    GameServerInfoManager servers;
    servers.load(repository);

    const de::DatagramPeers peers = de::loginServerDatagramPeers(config, servers);
    EXPECT_EQ(peers.gameServers, (std::vector<std::string>{"gameserver.example"}));
    EXPECT_FALSE(de::admitsDatagram(Packet::PACKET_GL_KICK_VERIFY, "192.0.2.9", peers));
    EXPECT_FALSE(de::admitsDatagram(Packet::PACKET_GL_KICK_VERIFY, "198.51.100.7", peers));
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

TEST(DatagramPeers, AConfiguredAddressThatIsNoDottedQuadIsAnError) {
    for (const char* bad : {"loginserver.example", "10.0.0", "10.0.0.256", "10.0.0.5:9997", "::1"}) {
        Properties config;
        config.setProperty("DatagramPeerIPs", std::string("10.0.0.5, ") + bad);
        EXPECT_THROW(de::configuredDatagramPeers(config), Error) << bad;
        GameServerInfoManager servers;
        EXPECT_THROW(de::gameServerDatagramPeers(config, servers), Error) << bad;
        EXPECT_THROW(de::loginServerDatagramPeers(config, servers), Error) << bad;
    }
}

TEST(DatagramPeers, TheConfiguredAddressesJoinTheServerLinksEachSocketReceives) {
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
    EXPECT_TRUE(de::admitsDatagram(Packet::PACKET_LG_KICK_CHARACTER, "10.0.0.5", onTheGameserver));
    EXPECT_TRUE(de::admitsDatagram(Packet::PACKET_GG_COMMAND, "10.0.0.5", onTheGameserver));
    EXPECT_FALSE(de::admitsDatagram(Packet::PACKET_GG_COMMAND, "10.0.0.7", onTheGameserver));

    const de::DatagramPeers onTheLoginserver = de::loginServerDatagramPeers(config, servers);
    EXPECT_TRUE(onTheLoginserver.loginServers.empty());
    EXPECT_EQ(onTheLoginserver.gameServers,
              (std::vector<std::string>{"10.0.0.5", "192.0.2.10", "192.0.2.11", "192.0.2.9"}));
    EXPECT_TRUE(de::admitsDatagram(Packet::PACKET_GL_KICK_VERIFY, "10.0.0.5", onTheLoginserver));
    EXPECT_FALSE(de::admitsDatagram(Packet::PACKET_GG_COMMAND, "10.0.0.5", onTheLoginserver));
}

TEST(DatagramGate, AnAdmittedDatagramIsCountedAndARefusedOneReported) {
    std::vector<std::pair<PacketID_t, std::string>> reports;
    de::DatagramGate gate([] { return gameserverPeers({"192.0.2.1"}, {"192.0.2.9"}); },
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

    Datagram noDatagramId;
    receive(noDatagramId, headerOf(Packet::PACKET_CG_SAY), "192.0.2.1");
    EXPECT_FALSE(gate.admits(noDatagramId));

    EXPECT_EQ(gate.admitted(), 2u);
    EXPECT_EQ(gate.refused(), 3u);
    ASSERT_EQ(reports.size(), 3u);
    EXPECT_EQ(reports[0].first, Packet::PACKET_LG_KICK_CHARACTER);
    EXPECT_EQ(reports[0].second, "198.51.100.7");
    EXPECT_EQ(reports[1].first, Packet::PACKET_GG_COMMAND);
    EXPECT_EQ(reports[1].second, "192.0.2.1");
    EXPECT_EQ(reports[2].first, Packet::PACKET_CG_SAY);
    EXPECT_EQ(reports[2].second, "192.0.2.1");
}

// The socket hands the loop a datagram only for a positive receive, so the
// shortest one it can see is one byte, too short for an id.
TEST(DatagramGate, ADatagramTooShortForAnIdIsRefusedAsNoPacket) {
    std::vector<std::pair<PacketID_t, std::string>> reports;
    de::DatagramGate gate([] { return gameserverPeers({"192.0.2.1"}, {"192.0.2.9"}); },
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

TEST(DatagramGate, AnAdmittedDatagramReadsAsItWouldHaveBeforeTheGate) {
    installPortCheckFactory();
    de::DatagramGate gate([] { return gameserverPeers({}, {}); }, [](PacketID_t, const std::string&) {});

    CGPortCheck sent;
    sent.setPCName("Rowan");
    Datagram datagram;
    datagram.write(&sent);
    datagram.setHost("198.51.100.7");
    datagram.setPort(40000);
    EXPECT_TRUE(gate.admits(datagram));

    DatagramPacket* pPacket = NULL;
    datagram.read(pPacket);
    ASSERT_TRUE(pPacket != NULL);
    EXPECT_EQ(pPacket->getPacketID(), Packet::PACKET_CG_PORT_CHECK);
    EXPECT_EQ(static_cast<CGPortCheck*>(pPacket)->getPCName(), "Rowan");
    EXPECT_EQ(pPacket->getHost(), "198.51.100.7");
    EXPECT_EQ(pPacket->getPort(), 40000u);
    delete pPacket;
}

TEST(DatagramGate, ThePeersAreAskedForOnceOnTheFirstServerLinkDatagram) {
    int asked = 0;
    de::DatagramGate gate(
        [&] {
            ++asked;
            return gameserverPeers({"192.0.2.1"}, {});
        },
        [](PacketID_t, const std::string&) {});

    Datagram aPortCheck;
    receive(aPortCheck, headerOf(Packet::PACKET_CG_PORT_CHECK), "198.51.100.7");
    EXPECT_TRUE(gate.admits(aPortCheck));
    EXPECT_EQ(asked, 1);

    Datagram noDatagramId;
    receive(noDatagramId, headerOf(Packet::PACKET_CG_SAY), "192.0.2.1");
    EXPECT_FALSE(gate.admits(noDatagramId));
    EXPECT_EQ(asked, 1);

    Datagram first;
    receive(first, headerOf(Packet::PACKET_LG_KICK_CHARACTER), "192.0.2.1");
    EXPECT_TRUE(gate.admits(first));
    Datagram second;
    receive(second, headerOf(Packet::PACKET_LG_KICK_CHARACTER), "192.0.2.1");
    EXPECT_TRUE(gate.admits(second));
    EXPECT_EQ(asked, 1);
}

TEST(DatagramGate, ADatagramTheGateCannotJudgeIsRefused) {
    int reported = 0;
    de::DatagramGate gate([]() -> de::DatagramPeers { throw Error("no catalogue yet"); },
                          [&](PacketID_t, const std::string&) {
                              ++reported;
                              throw Error("no log either");
                          });

    Datagram fromTheLoginserver;
    receive(fromTheLoginserver, headerOf(Packet::PACKET_LG_KICK_CHARACTER), "192.0.2.1");
    EXPECT_FALSE(gate.admits(fromTheLoginserver));
    EXPECT_EQ(gate.admitted(), 0u);
    EXPECT_EQ(gate.refused(), 1u);
    EXPECT_EQ(reported, 1);
}
