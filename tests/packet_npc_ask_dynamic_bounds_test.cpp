//////////////////////////////////////////////////////////////////////
//
// Filename    : packet_npc_ask_dynamic_bounds_test.cpp
// Description : GCNPCAskDynamic's budget is what its write() can emit.
//
//               The subject and each choice travel behind a WORD length,
//               so the wire alone would let one run to 65,535 bytes, and
//               the client admits the packet only up to the factory's
//               maximum, which budgets 1,024 bytes for the subject and
//               for each of 15 choices. A packet at that budget writes
//               exactly the budget; a body one byte past it is refused by
//               write(); a subject past its term still writes when the
//               choices leave the body room, as the client admits it.
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

// A packet whose subject is `subjectSize` bytes and whose fifteen choices
// are each at the budget's term.
GCNPCAskDynamic withSubjectOf(size_t subjectSize) {
    GCNPCAskDynamic packet;
    packet.setObjectID(7);
    packet.setScriptID(3);
    packet.setSubject(std::string(subjectSize, 's'));
    for (uint i = 0; i < GCNPCAskDynamic::kMaxCount; i++)
        packet.addContent(std::string(GCNPCAskDynamic::kMaxStringSize, 'c'));
    return packet;
}

} // namespace

TEST(GCNPCAskDynamicBounds, aPacketAtTheBudgetWritesExactlyTheBudget) {
    const GCNPCAskDynamic packet = withSubjectOf(GCNPCAskDynamic::kMaxStringSize);
    const std::vector<unsigned char> body = wiretest::writeBody(packet, kPlainCode);
    EXPECT_EQ(body.size(), (size_t)GCNPCAskDynamicFactory::kMaxSize);
    EXPECT_EQ((size_t)packet.getPacketSize(), body.size());
}

TEST(GCNPCAskDynamicBounds, aBodyOneBytePastTheBudgetIsRefused) {
    const GCNPCAskDynamic packet = withSubjectOf(GCNPCAskDynamic::kMaxStringSize + 1);
    EXPECT_EQ((size_t)packet.getPacketSize(), (size_t)GCNPCAskDynamicFactory::kMaxSize + 1);
    EXPECT_THROW(wiretest::writeBody(packet, kPlainCode), InvalidProtocolException);
}

TEST(GCNPCAskDynamicBounds, aSubjectPastItsTermWritesWhileTheBodyFits) {
    GCNPCAskDynamic packet;
    packet.setObjectID(7);
    packet.setScriptID(3);
    packet.setSubject(std::string(2 * GCNPCAskDynamic::kMaxStringSize, 's'));
    packet.addContent("yes");
    const std::vector<unsigned char> body = wiretest::writeBody(packet, kPlainCode);
    EXPECT_EQ(body.size(), (size_t)packet.getPacketSize());
    EXPECT_LT(body.size(), (size_t)GCNPCAskDynamicFactory::kMaxSize);
}

TEST(GCNPCAskDynamicBounds, aSixteenthChoiceIsRefused) {
    GCNPCAskDynamic packet = withSubjectOf(1);
    EXPECT_EQ((int)packet.getContentsCount(), (int)GCNPCAskDynamic::kMaxCount);
    EXPECT_THROW(packet.addContent("one more"), InvalidProtocolException);
    EXPECT_EQ((int)packet.getContentsCount(), (int)GCNPCAskDynamic::kMaxCount);
}
