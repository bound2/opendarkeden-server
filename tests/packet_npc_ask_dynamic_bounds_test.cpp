//////////////////////////////////////////////////////////////////////
//
// Filename    : packet_npc_ask_dynamic_bounds_test.cpp
// Description : GCNPCAskDynamic's budget is what its write() can emit.
//
//               The subject and each choice travel behind a WORD length,
//               so the wire alone would let one run to 65,535 bytes, and
//               the client admits the packet only up to the factory's
//               maximum, which budgets 1,024 bytes per string and 15
//               choices. A packet at that budget writes exactly the
//               budget; a subject or a choice past it is refused by the
//               setter and by write(), so the budget holds.
//
//////////////////////////////////////////////////////////////////////

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Exception.h"
#include "GCNPCAskDynamic.h"
#include "Packet.h"
#include "support/TestStreams.h"

namespace {

const uchar kPlainCode = 0;

GCNPCAskDynamic atTheBudget() {
    GCNPCAskDynamic packet;
    packet.setObjectID(7);
    packet.setScriptID(3);
    packet.setSubject(std::string(GCNPCAskDynamic::kMaxStringSize, 's'));
    for (uint i = 0; i < GCNPCAskDynamic::kMaxCount; i++)
        packet.addContent(std::string(GCNPCAskDynamic::kMaxStringSize, 'c'));
    return packet;
}

} // namespace

TEST(GCNPCAskDynamicBounds, aPacketAtTheBudgetWritesExactlyTheBudget) {
    const GCNPCAskDynamic packet = atTheBudget();
    const std::vector<unsigned char> body = wiretest::writeBody(packet, kPlainCode);
    EXPECT_EQ(body.size(), (size_t)GCNPCAskDynamicFactory::kMaxSize);
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size());
}

TEST(GCNPCAskDynamicBounds, aSubjectPastTheBudgetIsRefused) {
    GCNPCAskDynamic packet;
    EXPECT_THROW(packet.setSubject(std::string(GCNPCAskDynamic::kMaxStringSize + 1, 's')), InvalidProtocolException);
    EXPECT_NO_THROW(packet.setSubject(std::string(GCNPCAskDynamic::kMaxStringSize, 's')));
}

TEST(GCNPCAskDynamicBounds, aChoicePastTheBudgetIsRefused) {
    GCNPCAskDynamic packet;
    packet.setSubject("ask");
    EXPECT_THROW(packet.addContent(std::string(GCNPCAskDynamic::kMaxStringSize + 1, 'c')), InvalidProtocolException);
    EXPECT_EQ(packet.getContentsCount(), 0);
    EXPECT_NO_THROW(packet.addContent(std::string(GCNPCAskDynamic::kMaxStringSize, 'c')));
    EXPECT_EQ(packet.getContentsCount(), 1);
}
