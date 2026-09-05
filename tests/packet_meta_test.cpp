//////////////////////////////////////////////////////////////////////
//
// Filename    : packet_meta_test.cpp
// Description : Pins the compile-time packet metadata (PacketMeta.h).
//
// Three things are checked, and the first two fail the BUILD:
//   1. every factory in the kernel satisfies de::PacketFactoryType, and
//      the whole kernel folds into one FactoryList without a duplicate
//      or out-of-range id -- the same fact wire_layout_test proves at
//      run time, now proved while compiling;
//   2. validateRegistry rejects each kind of bad table, so the assert in
//      FactoryList is known to fire, not just known to compile;
//   3. at run time, each factory's constexpr members agree with what its
//      virtual getters and its packet report. The dispatcher now
//      registers handlers by Factory::kPacketID, so the factory/packet id
//      agreement is what keeps dispatch pointed at the same handlers.
//
//////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <string_view>
#include <type_traits>

#include "Packet.h"
#include "PacketFactory.h"
#include "PacketMeta.h"

#define ALL_PACKET_FACTORIES_INCLUDES
#include "AllPacketFactories.inc"
#undef ALL_PACKET_FACTORIES_INCLUDES

namespace {

using de::PacketFactoryType;
using de::packet::Concat;
using de::packet::FactoryList;
using de::packet::Meta;
using de::packet::RegistryError;
using de::packet::validateRegistry;

// --- 1. the kernel as one table ------------------------------------------

using KernelFactories = FactoryList<
#define ALL_PACKET_FACTORIES_TYPES
#include "AllPacketFactories.inc"
#undef ALL_PACKET_FACTORIES_TYPES
    >;

static_assert(KernelFactories::kCount > 400, "the generated factory list is unexpectedly short");
static_assert(KernelFactories::kVerdict.error == RegistryError::None,
              "two kernel factories claim one packet id, or an id is >= PACKET_MAX");

// Concept membership: a real factory is in, and the ways of being out are
// each rejected. The out-of-contract types below mirror what a factory
// looks like before this branch's edit -- id only in a virtual body.
static_assert(PacketFactoryType<CGAttackFactory>);
static_assert(PacketFactoryType<CGUseMessageItemFromInventoryFactory>, "a derived factory carries its own facts");

struct NotAFactory {
    [[maybe_unused]] static constexpr PacketID_t kPacketID = 1;
    [[maybe_unused]] static constexpr PacketSize_t kMaxSize = 1;
    [[maybe_unused]] static constexpr std::string_view kName = "NotAFactory";
};
static_assert(!PacketFactoryType<NotAFactory>, "must derive from PacketFactory");

// Not constexpr, so a value initialised from it is never a constant expression.
PacketID_t idDecidedAtRunTime() {
    return 1;
}

struct RuntimeIdFactory : PacketFactory {
    static inline const PacketID_t kPacketID = idDecidedAtRunTime();
    static constexpr PacketSize_t kMaxSize = 1;
    static constexpr std::string_view kName = "RuntimeIdFactory";
    Packet* createPacket() override {
        return nullptr;
    }
    std::string getPacketName() const override {
        return std::string(kName);
    }
    PacketID_t getPacketID() const override {
        return kPacketID;
    }
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};
static_assert(!PacketFactoryType<RuntimeIdFactory>, "the id must be usable at compile time");

struct UnnamedFactory : RuntimeIdFactory {
    static constexpr PacketID_t kPacketID = 1;
    static constexpr std::string_view kName = "";
};
static_assert(!PacketFactoryType<UnnamedFactory>, "the name must be non-empty");

// --- 2. the rules, on hand-built tables ---------------------------------

constexpr Meta a{1, 4, "A"};
constexpr Meta b{2, 4, "B"};
constexpr Meta aAgain{1, 8, "A2"};
constexpr Meta tooHigh{static_cast<PacketID_t>(Packet::PACKET_MAX), 4, "High"};
constexpr Meta unnamed{3, 4, ""};

static_assert(validateRegistry(std::array<Meta, 0>{}).error == RegistryError::None);
static_assert(validateRegistry(std::array{a, b}).error == RegistryError::None);
static_assert(validateRegistry(std::array{a, b, aAgain}).error == RegistryError::DuplicateId);
static_assert(validateRegistry(std::array{a, b, aAgain}).id == 1, "the verdict names the clashing id");
static_assert(validateRegistry(std::array{a, tooHigh}).error == RegistryError::IdOutOfRange);
static_assert(validateRegistry(std::array{a, tooHigh}).id == Packet::PACKET_MAX);
static_assert(validateRegistry(std::array{a, unnamed}).error == RegistryError::EmptyName);

// Concat validates the joined table: two individually valid lists that
// share an id are one invalid list. (Only the verdict is inspected here;
// naming the list's kMeta would trip its static_assert, which is the
// production behaviour and not something a test can observe.)
using Left = FactoryList<CGAttackFactory>;
using Right = FactoryList<CGMoveFactory>;
static_assert(Concat<Left, Right>::kCount == 2);
static_assert(Concat<Left, Right>::kMeta[0].id == CGAttackFactory::kPacketID);
static_assert(Concat<Left, Right>::kMeta[1].id == CGMoveFactory::kPacketID);
static_assert(validateRegistry(std::array{de::packet::metaOf<CGAttackFactory>(), de::packet::metaOf<CGAttackFactory>()})
                  .error == RegistryError::DuplicateId);

// --- 3. constexpr members agree with the virtual getters ----------------

struct Disagreement {
    std::string factory;
    std::string what;
};

template <PacketFactoryType F> void checkOne(std::vector<Disagreement>& out) {
    F factory;
    const std::string name(F::kName);
    if (factory.getPacketID() != F::kPacketID)
        out.push_back({name, "getPacketID() != kPacketID"});
    if (factory.getPacketMaxSize() != F::kMaxSize)
        out.push_back({name, "getPacketMaxSize() != kMaxSize"});
    if (factory.getPacketName() != name)
        out.push_back({name, "getPacketName() != kName"});
    std::unique_ptr<Packet> packet(factory.createPacket());
    if (!packet)
        out.push_back({name, "createPacket() returned null"});
    else if (packet->getPacketID() != F::kPacketID)
        out.push_back({name, "packet->getPacketID() != Factory::kPacketID (dispatch would miss)"});
}

TEST(PacketMeta, ConstexprMembersAgreeWithVirtuals) {
    std::vector<Disagreement> disagreements;
    KernelFactories::forEach([&]<typename F>(std::type_identity<F>) { checkOne<F>(disagreements); });
    for (const Disagreement& d : disagreements)
        ADD_FAILURE() << d.factory << ": " << d.what;
    EXPECT_TRUE(disagreements.empty());
}

TEST(PacketMeta, ForEachVisitsInPackOrder) {
    std::vector<std::string> names;
    Concat<Left, Right>::forEach([&]<typename F>(std::type_identity<F>) { names.emplace_back(F::kName); });
    ASSERT_EQ(names.size(), 2u);
    EXPECT_EQ(names[0], "CGAttack");
    EXPECT_EQ(names[1], "CGMove");
}

// The generated .inc has two sections built from one list: the REGISTER
// section (heap factories, what wire_layout_test inventories) and the TYPES
// section (this file's KernelFactories). They are only useful together if
// they name the same factories, so compare the ids the run-time section
// produces with the ids in the compile-time table.
std::vector<PacketFactory*> makeRegisteredFactories() {
    std::vector<PacketFactory*> factories;
#define ALL_PACKET_FACTORIES_REGISTER
#include "AllPacketFactories.inc"
#undef ALL_PACKET_FACTORIES_REGISTER
    return factories;
}

TEST(PacketMeta, GeneratedSectionsNameTheSameFactories) {
    std::vector<PacketFactory*> registered = makeRegisteredFactories();
    std::vector<PacketID_t> runtimeIds;
    for (PacketFactory* factory : registered)
        runtimeIds.push_back(factory->getPacketID());
    std::vector<PacketID_t> tableIds;
    for (const Meta& meta : KernelFactories::kMeta)
        tableIds.push_back(meta.id);
    std::sort(runtimeIds.begin(), runtimeIds.end());
    std::sort(tableIds.begin(), tableIds.end());
    EXPECT_EQ(runtimeIds, tableIds);
    EXPECT_EQ(registered.size(), KernelFactories::kCount);
    for (PacketFactory* factory : registered)
        delete factory;
}

} // namespace
