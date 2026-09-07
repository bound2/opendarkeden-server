// MySQL-backed integration tier for the Exchange repository (gameserver)
// and ServerCore's PayPlay and ServerInfo repositories: the real MySQL
// impls against the throwaway MySQL 5.7 loaded with initdb/, on the
// connections mysql_repository_test.cpp's main() wires for this binary.
// The dist connection PayPlay and two ServerInfo reads ask for, and the
// "USERINFO" name the Exchange point statements ask for, both resolve to
// the same server and DARKEDEN schema as the seeding below.
//
// Seeded names carry the "it-sc" prefix; numeric keys use 9170 and up.

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "DB.h"
#include "GCExchangeList.h"
#include "repository/ExchangeRepository.h"
#include "repository/PayPlayRepository.h"
#include "repository/ServerInfoRepository.h"

namespace {

void execSQL(const std::string& sql) {
    Statement* pStmt = NULL;
    BEGIN_DB {
        pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
        pStmt->executeQueryString(sql);
        SAFE_DELETE(pStmt);
    }
    END_DB(pStmt)
}

std::string queryScalar(const std::string& sql) {
    std::string value;
    Statement* pStmt = NULL;
    BEGIN_DB {
        pStmt = g_pDatabaseManager->getConnection("DARKEDEN")->createStatement();
        Result* pResult = pStmt->executeQueryString(sql);
        if (pResult->next())
            value = pResult->getString(1);
        SAFE_DELETE(pStmt);
    }
    END_DB(pStmt)
    return value;
}

std::string q(const std::string& s) {
    return "'" + s + "'";
}

std::string i64(int64_t v) {
    return std::to_string((long long)v);
}

class ExchangeMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        clean();
    }
    virtual void TearDown() {
        clean();
    }
    static void clean() {
        execSQL("DELETE FROM ExchangeOrder WHERE BuyerAccount LIKE 'it-sc%'");
        execSQL("DELETE FROM ExchangeListing WHERE SellerAccount LIKE 'it-sc%'");
    }

    // A listing with every column set to a distinct value; objectID keeps
    // the UNIQUE (ItemClass, ItemID, ObjectID) apart between rows.
    static ExchangeListing make(int objectID, const std::string& sellerAccount = "it-sc-seller",
                                const std::string& sellerPlayer = "it-sc-sp") {
        ExchangeListing l;
        l.listingID = 0;
        l.serverID = 9170;
        l.sellerAccount = sellerAccount;
        l.sellerPlayer = sellerPlayer;
        l.sellerRace = 1;
        l.itemClass = 2;
        l.itemType = 3;
        l.itemID = 917000000000LL;
        l.objectID = objectID;
        l.pricePoint = 500;
        l.currency = 0;
        l.status = LISTING_STATUS_ACTIVE;
        l.taxRate = 8;
        l.taxAmount = 40;
        l.createdAt = "ignored";
        l.expireAt = "2030-01-02 03:04:05";
        l.updatedAt = "ignored";
        l.version = 4;
        l.itemName = "it-sc item";
        l.enchantLevel = 5;
        l.grade = 6;
        l.durability = 7;
        l.silver = 8;
        l.optionType1 = 9;
        l.optionType2 = 10;
        l.optionType3 = 11;
        l.optionValue1 = 12;
        l.optionValue2 = 13;
        l.optionValue3 = 14;
        l.stackCount = 15;
        return l;
    }

    static std::string field(const char* column, int64_t listingID) {
        return queryScalar(std::string("SELECT ") + column +
                           " FROM ExchangeListing WHERE ListingID = " + i64(listingID));
    }
};

TEST_F(ExchangeMySQL, ListingRoundTripAndStatusTransitions) {
    ExchangeRepository& repo = defaultExchangeRepository();

    int64_t id = repo.createListing(make(1));
    ASSERT_GT(id, 0);
    int64_t other = repo.createListing(make(2));
    ASSERT_GT(other, id);

    ExchangeListing* p = repo.getListing(id);
    ASSERT_TRUE(p != NULL);
    EXPECT_EQ(id, p->listingID);
    EXPECT_EQ(9170, p->serverID);
    EXPECT_EQ("it-sc-seller", p->sellerAccount);
    EXPECT_EQ("it-sc-sp", p->sellerPlayer);
    EXPECT_EQ(1, p->sellerRace);
    EXPECT_EQ(2, p->itemClass);
    EXPECT_EQ(3, p->itemType);
    EXPECT_EQ(917000000000LL, p->itemID);
    EXPECT_EQ(1, p->objectID);
    EXPECT_EQ(500, p->pricePoint);
    EXPECT_EQ(0, p->currency);
    EXPECT_EQ(LISTING_STATUS_ACTIVE, p->status);
    EXPECT_EQ("", p->buyerAccount);
    EXPECT_EQ("", p->buyerPlayer);
    EXPECT_EQ(8, p->taxRate);
    EXPECT_EQ(40, p->taxAmount);
    // CreatedAt and UpdatedAt are the process's clock, not the caller's
    // text; ExpireAt is the caller's.
    EXPECT_EQ(19u, p->createdAt.size());
    EXPECT_EQ("2030-01-02 03:04:05", p->expireAt);
    EXPECT_EQ("", p->soldAt);
    EXPECT_EQ("", p->cancelledAt);
    EXPECT_EQ(19u, p->updatedAt.size());
    EXPECT_EQ(4, p->version);
    EXPECT_EQ("it-sc item", p->itemName);
    EXPECT_EQ(5, p->enchantLevel);
    EXPECT_EQ(6, p->grade);
    EXPECT_EQ(7, p->durability);
    EXPECT_EQ(8, p->silver);
    EXPECT_EQ(9, p->optionType1);
    EXPECT_EQ(10, p->optionType2);
    EXPECT_EQ(11, p->optionType3);
    EXPECT_EQ(12, p->optionValue1);
    EXPECT_EQ(13, p->optionValue2);
    EXPECT_EQ(14, p->optionValue3);
    EXPECT_EQ(15, p->stackCount);
    delete p;

    EXPECT_TRUE(repo.getListing(id + 1000000) == NULL);

    // Sold: buyer columns and SoldAt; only an ACTIVE row.
    EXPECT_TRUE(repo.markListingSold(id, "it-sc-buyer", "it-sc-bp"));
    EXPECT_EQ("1", field("Status", id));
    EXPECT_EQ("it-sc-buyer", field("BuyerAccount", id));
    EXPECT_EQ("it-sc-bp", field("BuyerPlayer", id));
    EXPECT_EQ("19", queryScalar("SELECT LENGTH(SoldAt) FROM ExchangeListing WHERE ListingID = " + i64(id)));
    // The other row is untouched, and a sold row does not cancel or
    // expire; the methods still answer true.
    EXPECT_EQ("0", field("Status", other));
    EXPECT_TRUE(repo.cancelListing(id));
    EXPECT_EQ("1", field("Status", id));
    EXPECT_TRUE(repo.expireListing(id));
    EXPECT_EQ("1", field("Status", id));

    EXPECT_TRUE(repo.cancelListing(other));
    EXPECT_EQ("2", field("Status", other));
    EXPECT_EQ("19", queryScalar("SELECT LENGTH(CancelledAt) FROM ExchangeListing WHERE ListingID = " + i64(other)));
}

TEST_F(ExchangeMySQL, ListingQueriesFilterByServerStatusSellerAndExpiry) {
    ExchangeRepository& repo = defaultExchangeRepository();

    int64_t a = repo.createListing(make(11));
    int64_t b = repo.createListing(make(12));
    int64_t c = repo.createListing(make(13, "it-sc-other", "it-sc-op"));
    ExchangeListing expired = make(14);
    expired.expireAt = "2001-01-01 00:00:00";
    int64_t d = repo.createListing(expired);
    repo.cancelListing(b);

    // Server 9170's ACTIVE listings: a, c, d. The page of size 2 holds
    // two of them, the second page the third.
    EXPECT_EQ(2u, repo.getListings(9170, LISTING_STATUS_ACTIVE, 1, 2).size());
    EXPECT_EQ(1u, repo.getListings(9170, LISTING_STATUS_ACTIVE, 2, 2).size());
    EXPECT_EQ(0u, repo.getListings(9170, LISTING_STATUS_ACTIVE, 3, 2).size());
    EXPECT_EQ(1u, repo.getListings(9170, LISTING_STATUS_CANCELLED, 1, 10).size());
    EXPECT_EQ(0u, repo.getListings(9171, LISTING_STATUS_ACTIVE, 1, 10).size());

    std::vector<ExchangeListing> mine = repo.getSellerListings("it-sc-seller", LISTING_STATUS_ACTIVE);
    EXPECT_EQ(2u, mine.size());
    for (size_t i = 0; i < mine.size(); i++) {
        EXPECT_TRUE(mine[i].listingID == a || mine[i].listingID == d);
        EXPECT_EQ("it-sc-seller", mine[i].sellerAccount);
    }
    EXPECT_EQ(1u, repo.getSellerListings("it-sc-other", LISTING_STATUS_ACTIVE).size());
    EXPECT_EQ(1u, repo.getSellerListings("it-sc-seller", LISTING_STATUS_CANCELLED).size());

    // Only d is ACTIVE with an ExpireAt in the past.
    std::vector<ExchangeListing> old = repo.getExpiredListings();
    bool sawD = false;
    for (size_t i = 0; i < old.size(); i++) {
        EXPECT_TRUE(old[i].listingID != a && old[i].listingID != b && old[i].listingID != c);
        if (old[i].listingID == d)
            sawD = true;
    }
    EXPECT_TRUE(sawD);
    EXPECT_TRUE(repo.expireListing(d));
    EXPECT_EQ("3", field("Status", d));
}

TEST_F(ExchangeMySQL, OrdersRoundTripAndJoinToTheSeller) {
    ExchangeRepository& repo = defaultExchangeRepository();

    int64_t listing = repo.createListing(make(21));
    int64_t otherListing = repo.createListing(make(22, "it-sc-other", "it-sc-op"));

    ExchangeOrder o;
    o.orderID = 0;
    o.listingID = listing;
    o.serverID = 9170;
    o.buyerAccount = "it-sc-buyer";
    o.buyerPlayer = "it-sc-bp";
    o.pricePoint = 500;
    o.taxAmount = 40;
    o.status = ORDER_STATUS_PAID;
    int64_t orderID = repo.createOrder(o);
    ASSERT_GT(orderID, 0);

    ExchangeOrder o2 = o;
    o2.listingID = otherListing;
    o2.buyerPlayer = "it-sc-bq";
    int64_t orderID2 = repo.createOrder(o2);
    ASSERT_GT(orderID2, orderID);

    std::vector<ExchangeOrder> mine = repo.getBuyerOrders("it-sc-bp", ORDER_STATUS_PAID);
    ASSERT_EQ(1u, mine.size());
    EXPECT_EQ(orderID, mine[0].orderID);
    EXPECT_EQ(listing, mine[0].listingID);
    EXPECT_EQ(9170, mine[0].serverID);
    EXPECT_EQ("it-sc-buyer", mine[0].buyerAccount);
    EXPECT_EQ("it-sc-bp", mine[0].buyerPlayer);
    EXPECT_EQ(500, mine[0].pricePoint);
    EXPECT_EQ(40, mine[0].taxAmount);
    EXPECT_EQ(ORDER_STATUS_PAID, mine[0].status);
    EXPECT_EQ(19u, mine[0].createdAt.size());
    EXPECT_EQ("", mine[0].deliveredAt);
    EXPECT_EQ("", mine[0].cancelledAt);

    // The seller's orders come through the listing's SellerPlayer.
    std::vector<ExchangeOrder> sold = repo.getSellerOrders("it-sc-sp", ORDER_STATUS_PAID);
    ASSERT_EQ(1u, sold.size());
    EXPECT_EQ(orderID, sold[0].orderID);
    EXPECT_EQ(1u, repo.getSellerOrders("it-sc-op", ORDER_STATUS_PAID).size());
    EXPECT_EQ(0u, repo.getSellerOrders("it-sc-nobody", ORDER_STATUS_PAID).size());

    EXPECT_TRUE(repo.markOrderDelivered(orderID));
    EXPECT_EQ("1", queryScalar("SELECT Status FROM ExchangeOrder WHERE OrderID = " + i64(orderID)));
    EXPECT_EQ("19", queryScalar("SELECT LENGTH(DeliveredAt) FROM ExchangeOrder WHERE OrderID = " + i64(orderID)));
    EXPECT_EQ("0", queryScalar("SELECT Status FROM ExchangeOrder WHERE OrderID = " + i64(orderID2)));
    EXPECT_EQ(0u, repo.getBuyerOrders("it-sc-bp", ORDER_STATUS_PAID).size());
    EXPECT_EQ(1u, repo.getBuyerOrders("it-sc-bp", ORDER_STATUS_DELIVERED).size());

    // A second order on one listing hits the UNIQUE ListingID.
    EXPECT_ANY_THROW(repo.createOrder(o));
}

TEST_F(ExchangeMySQL, TextIsEscapedAndARelistedObjectIsRefused) {
    ExchangeRepository& repo = defaultExchangeRepository();

    ExchangeListing quoted = make(31, "it-sc-o'brien", "it-sc-d\\r");
    quoted.itemName = "it-sc it's \\ \"name\"";
    int64_t id = repo.createListing(quoted);
    ASSERT_GT(id, 0);

    ExchangeListing* p = repo.getListing(id);
    ASSERT_TRUE(p != NULL);
    EXPECT_EQ("it-sc-o'brien", p->sellerAccount);
    EXPECT_EQ("it-sc-d\\r", p->sellerPlayer);
    EXPECT_EQ("it-sc it's \\ \"name\"", p->itemName);
    delete p;
    EXPECT_EQ(1u, repo.getSellerListings("it-sc-o'brien", LISTING_STATUS_ACTIVE).size());

    // The same (ItemClass, ItemID, ObjectID) again, even after the first
    // row was cancelled: ER_DUP_ENTRY crossing as END_DB's const char*.
    repo.cancelListing(id);
    EXPECT_ANY_THROW(repo.createListing(make(31)));
}

TEST_F(ExchangeMySQL, PointTablesAreNotReachableOnTheConnectionTheyAskFor) {
    // AccountPoint and PointLedger live in USERINFO, but the "USERINFO"
    // name resolves to the DARKEDEN connection, so every point statement
    // is a SQL error. This pins that each throws, not the message.
    ExchangeRepository& repo = defaultExchangeRepository();
    int balanceAfter = 0;
    EXPECT_ANY_THROW(repo.getPointBalance("it-sc-buyer"));
    EXPECT_ANY_THROW(repo.hasIdempotencyKey("it-sc-key"));
    EXPECT_ANY_THROW(repo.adjustPoints("it-sc-buyer", 10, balanceAfter, POINT_REASON_ADJUST, 0, 0, ""));
}

TEST_F(ExchangeMySQL, TheTransactionPairHoldsOnTheOneConnection) {
    ExchangeRepository& repo = defaultExchangeRepository();

    EXPECT_TRUE(repo.beginTransaction());
    int64_t rolledBack = repo.createListing(make(41));
    ASSERT_GT(rolledBack, 0);
    EXPECT_TRUE(repo.rollback());
    EXPECT_TRUE(repo.getListing(rolledBack) == NULL);

    EXPECT_TRUE(repo.beginTransaction());
    int64_t kept = repo.createListing(make(42));
    ASSERT_GT(kept, 0);
    EXPECT_TRUE(repo.commit());
    ExchangeListing* p = repo.getListing(kept);
    EXPECT_TRUE(p != NULL);
    delete p;
}

class PayPlayMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        clean();
    }
    virtual void TearDown() {
        clean();
    }
    static void clean() {
        execSQL("DELETE FROM PCRoomInfo WHERE ID BETWEEN 9170 AND 9179");
        execSQL("DELETE FROM PCRoomIPInfo WHERE ID BETWEEN 9170 AND 9179");
        execSQL("DELETE FROM PCRoomUserInfo WHERE ID BETWEEN 9170 AND 9179");
        execSQL("DELETE FROM PCRoomPayList WHERE PCRoomID BETWEEN 9170 AND 9179");
        execSQL("DELETE FROM Player WHERE PlayerID LIKE 'it-sc%'");
    }

    static void seedRoom(int id, const std::string& ip, int payType, int hours) {
        execSQL("INSERT INTO PCRoomInfo (ID, Account, PayType, PayStartDate, PayPlayDate, PayPlayHours, PayPlayFlag, "
                "UserLimit, UserMax) VALUES (" +
                std::to_string(id) + ", 'it-sc-room" + std::to_string(id) + "', " + std::to_string(payType) +
                ", '2026-01-01 00:00:00', '2030-01-01 00:00:00', " + std::to_string(hours) + ", 3, 20, 30)");
        execSQL("INSERT INTO PCRoomIPInfo (ID, IP) VALUES (" + std::to_string(id) + ", " + q(ip) + ")");
    }
};

TEST_F(PayPlayMySQL, TheRoomOfAClientIPInBothProjections) {
    PayPlayRepository& repo = defaultPayPlayRepository();
    seedRoom(9170, "91.70.0.1", 2, 11);
    seedRoom(9171, "91.70.0.2", 1, 12);

    PayPlayPCRoomRow room;
    ASSERT_TRUE(repo.loadPCRoomByIP("91.70.0.1", room));
    EXPECT_EQ(9170, room.id);
    EXPECT_EQ(2, room.payType);
    EXPECT_EQ("2026-01-01 00:00:00", room.payStartDate);
    EXPECT_EQ("2030-01-01 00:00:00", room.payPlayDate);
    EXPECT_EQ(11, room.payPlayHours);
    EXPECT_EQ(3, room.payPlayFlag);
    EXPECT_EQ(20, room.userLimit);
    EXPECT_EQ(30, room.userMax);

    PayPlayPCRoomPeriodRow period;
    ASSERT_TRUE(repo.loadPCRoomPeriodByIP("91.70.0.2", period));
    EXPECT_EQ(9171, period.id);
    EXPECT_EQ(1, period.payType);
    EXPECT_EQ("2026-01-01 00:00:00", period.payStartDate);
    EXPECT_EQ("2030-01-01 00:00:00", period.payPlayDate);
    EXPECT_EQ(12, period.payPlayHours);

    room.id = -1;
    EXPECT_FALSE(repo.loadPCRoomByIP("91.70.0.3", room));
    EXPECT_FALSE(repo.loadPCRoomPeriodByIP("91.70.0.3", period));
    EXPECT_EQ(-1, room.id);

    // The room's hours go down and the new value comes back; a missing
    // room answers false.
    int remaining = -1;
    ASSERT_TRUE(repo.decreasePCRoomPayPlayHours(4, 9170, remaining));
    EXPECT_EQ(7, remaining);
    EXPECT_EQ("12", queryScalar("SELECT PayPlayHours FROM PCRoomInfo WHERE ID = 9171"));
    remaining = -1;
    EXPECT_FALSE(repo.decreasePCRoomPayPlayHours(1, 9179, remaining));
    EXPECT_EQ(-1, remaining);
}

TEST_F(PayPlayMySQL, OccupantsAndMonthlyMinutes) {
    PayPlayRepository& repo = defaultPayPlayRepository();

    EXPECT_EQ(0, repo.loadPCRoomUserCount(9170));
    repo.insertPCRoomUser(9170, "it-sc-a");
    repo.insertPCRoomUser(9170, "it-sc-b");
    repo.insertPCRoomUser(9171, "it-sc-c");
    // (ID, PlayerID) is the primary key: INSERT IGNORE drops a repeat.
    repo.insertPCRoomUser(9170, "it-sc-a");
    EXPECT_EQ(2, repo.loadPCRoomUserCount(9170));
    EXPECT_EQ(1, repo.loadPCRoomUserCount(9171));

    repo.deletePCRoomUser("it-sc-a");
    EXPECT_EQ(1, repo.loadPCRoomUserCount(9170));
    EXPECT_EQ(1, repo.loadPCRoomUserCount(9171));

    EXPECT_FALSE(repo.hasPCRoomPayMonth(9170, 2026, 9));
    repo.insertPCRoomPayMonth(9170, 2026, 9, 15);
    EXPECT_TRUE(repo.hasPCRoomPayMonth(9170, 2026, 9));
    EXPECT_FALSE(repo.hasPCRoomPayMonth(9170, 2026, 10));
    EXPECT_FALSE(repo.hasPCRoomPayMonth(9171, 2026, 9));
    repo.addPCRoomPayMinutes(5, 9170, 2026, 9);
    EXPECT_EQ("20", queryScalar("SELECT PayPlayMinute FROM PCRoomPayList WHERE PCRoomID = 9170 AND Year = 2026 "
                                "AND Month = 9"));
}

TEST_F(PayPlayMySQL, TheAccountsPayPlayColumns) {
    PayPlayRepository& repo = defaultPayPlayRepository();
    execSQL("INSERT INTO Player (PlayerID, PayType, PayPlayDate, PayPlayHours, PayPlayFlag, FamilyPayPlayDate) "
            "VALUES ('it-sc-a', 1, '2030-01-02 03:04:05', 9, 5, '2031-01-02 03:04:05')");
    execSQL("INSERT INTO Player (PlayerID, PayType, PayPlayDate, PayPlayHours, PayPlayFlag, FamilyPayPlayDate) "
            "VALUES ('it-sc-k', 1, '2001-01-02 03:04:05', 9, 5, '2001-01-02 03:04:05')");
    execSQL("INSERT INTO Player (PlayerID, PayType, PayPlayDate) VALUES ('it-sc-f', 0, '2001-01-02 03:04:05')");

    PayPlayAccountRow row;
    ASSERT_TRUE(repo.loadAccountPayPlay("it-sc-a", row));
    EXPECT_EQ(1, row.payType);
    EXPECT_EQ("2030-01-02 03:04:05", row.payPlayDate);
    EXPECT_EQ(9, row.payPlayHours);
    EXPECT_EQ(5, row.payPlayFlag);
    EXPECT_EQ("2031-01-02 03:04:05", row.familyPayPlayDate);
    row.payType = -1;
    EXPECT_FALSE(repo.loadAccountPayPlay("it-sc-none", row));
    EXPECT_EQ(-1, row.payType);

    // Playing: free, or a period still running.
    int flag = -1;
    ASSERT_TRUE(repo.loadAccountPayPlaying("it-sc-a", flag));
    EXPECT_EQ(1, flag);
    ASSERT_TRUE(repo.loadAccountPayPlaying("it-sc-k", flag));
    EXPECT_EQ(0, flag);
    ASSERT_TRUE(repo.loadAccountPayPlaying("it-sc-f", flag));
    EXPECT_EQ(1, flag);
    flag = -1;
    EXPECT_FALSE(repo.loadAccountPayPlaying("it-sc-none", flag));
    EXPECT_EQ(-1, flag);

    repo.decreaseAccountPayPlayHours(4, "it-sc-a");
    EXPECT_EQ("5", queryScalar("SELECT PayPlayHours FROM Player WHERE PlayerID = 'it-sc-a'"));
    EXPECT_EQ("9", queryScalar("SELECT PayPlayHours FROM Player WHERE PlayerID = 'it-sc-k'"));

    repo.clearAccountPayPlay("it-sc-a");
    EXPECT_EQ("0", queryScalar("SELECT PayPlayHours FROM Player WHERE PlayerID = 'it-sc-a'"));
    EXPECT_EQ("2002-11-18 00:00:00", queryScalar("SELECT PayPlayDate FROM Player WHERE PlayerID = 'it-sc-a'"));
    EXPECT_EQ("2001-01-02 03:04:05", queryScalar("SELECT PayPlayDate FROM Player WHERE PlayerID = 'it-sc-k'"));
}

class ServerInfoMySQL : public ::testing::Test {
protected:
    virtual void SetUp() {
        clean();
    }
    virtual void TearDown() {
        clean();
    }
    static void clean() {
        execSQL("DELETE FROM GameServerInfo WHERE WorldID BETWEEN 9170 AND 9179");
        execSQL("DELETE FROM NonPKServerList WHERE WorldID = 91");
        execSQL("DELETE FROM CastleStatInfo WHERE WorldID BETWEEN 9170 AND 9179");
        execSQL("DELETE FROM WorldInfo WHERE ID BETWEEN 9170 AND 9179");
    }
};

TEST_F(ServerInfoMySQL, ServersWithTheirMaximaAndFlags) {
    ServerInfoRepository& repo = defaultServerInfoRepository();

    size_t before = repo.loadServers().size();
    execSQL("INSERT INTO GameServerInfo (ServerID, Nickname, IP, TCPPort, UDPPort, WorldID, GroupID, Stat) "
            "VALUES (61701, 'it-sc game1', '10.0.0.1', 33064, 9997, 9170, 5, 1)");
    execSQL("INSERT INTO GameServerInfo (ServerID, Nickname, IP, TCPPort, UDPPort, WorldID, GroupID, Stat) "
            "VALUES (61702, 'it-sc game2', '10.0.0.2', 33065, 9998, 9171, 9, 0)");

    std::vector<ServerInfoRow> rows = repo.loadServers();
    EXPECT_EQ(before + 2, rows.size());
    int seen = 0;
    for (size_t i = 0; i < rows.size(); i++) {
        if (rows[i].serverID == 61701) {
            seen++;
            EXPECT_EQ("it-sc game1", rows[i].nickname);
            EXPECT_EQ("10.0.0.1", rows[i].ip);
            EXPECT_EQ(33064, rows[i].tcpPort);
            EXPECT_EQ(9997, rows[i].udpPort);
            EXPECT_EQ(9170, rows[i].worldID);
            EXPECT_EQ(5, rows[i].groupID);
            EXPECT_EQ(1, rows[i].stat);
        }
    }
    EXPECT_EQ(1, seen);

    // initdb/ seeds GroupID 1 and WorldID 0 only, so the seeded rows hold
    // both maxima.
    int maxGroupID = 0;
    ASSERT_TRUE(repo.loadMaxServerGroupID(maxGroupID));
    EXPECT_EQ(9, maxGroupID);
    int maxWorldID = 0;
    ASSERT_TRUE(repo.loadMaxWorldID(maxWorldID));
    EXPECT_EQ(9171, maxWorldID);

    size_t nonPKBefore = repo.loadNonPKServers().size();
    execSQL("INSERT INTO NonPKServerList (WorldID, ServerGroupID) VALUES (91, 7)");
    std::vector<ServerInfoNonPKRow> nonPK = repo.loadNonPKServers();
    EXPECT_EQ(nonPKBefore + 1, nonPK.size());
    seen = 0;
    for (size_t i = 0; i < nonPK.size(); i++) {
        if (nonPK[i].worldID == 91) {
            seen++;
            EXPECT_EQ(7, nonPK[i].serverGroupID);
        }
    }
    EXPECT_EQ(1, seen);

    size_t castleBefore = repo.loadCastleStats().size();
    execSQL("INSERT INTO CastleStatInfo (WorldID, ServerGroupID, FollowServerID) VALUES (9170, 5, 8)");
    std::vector<ServerInfoCastleStatRow> castles = repo.loadCastleStats();
    EXPECT_EQ(castleBefore + 1, castles.size());
    seen = 0;
    for (size_t i = 0; i < castles.size(); i++) {
        if (castles[i].worldID == 9170) {
            seen++;
            EXPECT_EQ(5, castles[i].serverGroupID);
            EXPECT_EQ(8, castles[i].followServerID);
        }
    }
    EXPECT_EQ(1, seen);
}

TEST_F(ServerInfoMySQL, Worlds) {
    ServerInfoRepository& repo = defaultServerInfoRepository();

    size_t before = repo.loadWorlds().size();
    execSQL("INSERT INTO WorldInfo (ID, Name, Stat) VALUES (9170, 'it-sc world', 2)");

    std::vector<ServerInfoWorldRow> rows = repo.loadWorlds();
    EXPECT_EQ(before + 1, rows.size());
    int seen = 0;
    for (size_t i = 0; i < rows.size(); i++) {
        if (rows[i].id == 9170) {
            seen++;
            EXPECT_EQ("it-sc world", rows[i].name);
            EXPECT_EQ(2, rows[i].stat);
        }
    }
    EXPECT_EQ(1, seen);
}

} // namespace
