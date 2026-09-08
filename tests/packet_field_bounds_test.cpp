//////////////////////////////////////////////////////////////////////
//
// Filename    : packet_field_bounds_test.cpp
// Description : Range checks on the enum-valued fields of the CL packets
//               the login server reads off an unauthenticated socket.
//
//               A byte on the wire is whatever the peer sent. Storing one
//               straight into an enum member leaves an object holding a
//               value outside its enumeration, which is undefined to load
//               — the Debug toolchain traps on it — and the string tables
//               keyed by these enums (Slot2String, HairStyle2String,
//               PCType2String) have exactly one entry per enumerator, so a
//               value past the end indexes off the table. read() therefore
//               refuses the byte before the member is assigned, and these
//               cases pin that: one out-of-range byte per field is
//               rejected, and the image with every field at its highest
//               valid value still reads back.
//
//               Each image is produced by the packet's own write() at
//               encrypt code 0 and then patched at the field's offset,
//               so only the byte under test differs from a packet the
//               real client would send. The offsets follow write()'s
//               layout: a one-byte name length, the name, then the
//               fields. Nothing here writes a new byte sequence, so the
//               golden fixtures and tests/wire-layout.txt are untouched.
//
//////////////////////////////////////////////////////////////////////

#include <cstddef>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "CLCreatePC.h"
#include "CLSelectPC.h"
#include "Exception.h"
#include "TestStreams.h"

using wiretest::Loopback;
using wiretest::writeBody;

namespace {

// The unencrypted code. These two packets read their bodies with plain
// read() calls, so the encrypter never sees the bytes; code 0 keeps the
// image identical to what is patched below.
const uchar kPlainCode = 0;

// The name both images carry. Any length works; it only shifts the field
// offsets, which are computed from it.
const char* const kName = "TestName";

// Push a raw byte image through a real loopback connection and let the
// packet read it, exactly as a server reads a client's bytes. The image
// is written with the inherited plain write(), not writeEncrypt(), so it
// arrives byte for byte.
void readImage(Packet& packet, const std::vector<unsigned char>& bytes) {
    Loopback loopback;
    loopback.setCodes(kPlainCode);
    loopback.out().write(reinterpret_cast<const char*>(&bytes[0]), (uint)bytes.size());
    loopback.pump((uint)bytes.size());
    packet.read(loopback.in());
}

// A CLCreatePC every field of which is at its highest valid value:
// SLOT3, HAIR_STYLE3 and MALE. Its image is the base every patched image
// below is made from, so a rejection can only come from the patched byte.
CLCreatePC highestValidCreatePC() {
    CLCreatePC packet;
    packet.setName(kName);
    packet.setSlot(SLOT3);
    packet.setSex(MALE);
    packet.setHairStyle(HAIR_STYLE3);
    packet.setHairColor(7);
    packet.setSkinColor(8);
    packet.setShirtColor(9);
    packet.setShirtColor(10, SUB_COLOR);
    packet.setJeansColor(11);
    packet.setJeansColor(12, SUB_COLOR);
    packet.setSTR(13);
    packet.setDEX(14);
    packet.setINT(15);
    packet.setRace(RACE_SLAYER);
    return packet;
}

CLSelectPC highestValidSelectPC() {
    CLSelectPC packet;
    packet.setPCName(kName);
    packet.setPCType(PC_OUSTERS);
    return packet;
}

// write() lays CLCreatePC out as: name length, name, slot, flags, colors,
// attributes, race. So the slot is the byte after the name and the flags
// byte follows it.
const size_t kCreatePCSlotOffset = 1 + sizeof("TestName") - 1;
const size_t kCreatePCFlagsOffset = kCreatePCSlotOffset + 1;

// CLSelectPC is name length, name, pc type.
const size_t kSelectPCTypeOffset = 1 + sizeof("TestName") - 1;

std::vector<unsigned char> patched(const std::vector<unsigned char>& bytes, size_t offset, unsigned char value) {
    std::vector<unsigned char> copy(bytes);
    EXPECT_LT(offset, copy.size());
    copy[offset] = value;
    return copy;
}

//////////////////////////////////////////////////////////////////////
// CLCreatePC
//////////////////////////////////////////////////////////////////////

TEST(CLCreatePCFieldBounds, theImageUnderTestIsTheRealWireLayout) {
    const std::vector<unsigned char> bytes = writeBody(highestValidCreatePC(), kPlainCode);

    // The name length prefix, and the two patch offsets pointing at the
    // values the packet was built with. If write() ever moves a field,
    // this fails before the rejection cases mislead anyone.
    ASSERT_GT(bytes.size(), kCreatePCFlagsOffset);
    EXPECT_EQ(std::string(kName).size(), (size_t)bytes[0]);
    EXPECT_EQ((unsigned char)SLOT3, bytes[kCreatePCSlotOffset]);
    // Sex in bit 0, hair style in bits 1..2: MALE | (HAIR_STYLE3 << 1).
    EXPECT_EQ((unsigned char)(1 | (HAIR_STYLE3 << 1)), bytes[kCreatePCFlagsOffset]);
}

TEST(CLCreatePCFieldBounds, everyFieldAtItsHighestValidValueReadsBack) {
    const CLCreatePC src = highestValidCreatePC();
    CLCreatePC dst;
    readImage(dst, writeBody(src, kPlainCode));

    EXPECT_EQ(std::string(kName), dst.getName());
    EXPECT_EQ(SLOT3, dst.getSlot());
    EXPECT_EQ(MALE, dst.getSex());
    EXPECT_EQ(HAIR_STYLE3, dst.getHairStyle());
    EXPECT_EQ(7, dst.getHairColor());
    EXPECT_EQ(8, dst.getSkinColor());
    EXPECT_EQ(13, dst.getSTR());
    EXPECT_EQ(14, dst.getDEX());
    EXPECT_EQ(15, dst.getINT());
    EXPECT_EQ(RACE_SLAYER, dst.getRace());
}

TEST(CLCreatePCFieldBounds, aSlotByteOutsideTheThreeSlotsIsRefused) {
    // SLOT_MAX is the count, so it is the first byte that names no slot;
    // Slot2String stops one entry earlier.
    const unsigned char slots[] = {(unsigned char)SLOT_MAX, 4, 17, 255};
    const std::vector<unsigned char> base = writeBody(highestValidCreatePC(), kPlainCode);

    for (size_t i = 0; i < sizeof(slots) / sizeof(slots[0]); i++) {
        CLCreatePC dst;
        EXPECT_THROW(readImage(dst, patched(base, kCreatePCSlotOffset, slots[i])), InvalidProtocolException)
            << "slot " << (int)slots[i];
    }
}

TEST(CLCreatePCFieldBounds, everyRealSlotByteIsAccepted) {
    const Slot slots[] = {SLOT1, SLOT2, SLOT3};
    const std::vector<unsigned char> base = writeBody(highestValidCreatePC(), kPlainCode);

    for (size_t i = 0; i < sizeof(slots) / sizeof(slots[0]); i++) {
        CLCreatePC dst;
        readImage(dst, patched(base, kCreatePCSlotOffset, (unsigned char)slots[i]));
        EXPECT_EQ(slots[i], dst.getSlot());
    }
}

TEST(CLCreatePCFieldBounds, aHairStyleOutsideTheThreeStylesIsRefused) {
    // The two hair-style bits reach 3; HairStyle stops at HAIR_STYLE3 (2).
    // Both flag bytes below carry hair style 3, once with each sex.
    const unsigned char flags[] = {(unsigned char)(3 << 1), (unsigned char)(1 | (3 << 1))};
    const std::vector<unsigned char> base = writeBody(highestValidCreatePC(), kPlainCode);

    for (size_t i = 0; i < sizeof(flags) / sizeof(flags[0]); i++) {
        CLCreatePC dst;
        EXPECT_THROW(readImage(dst, patched(base, kCreatePCFlagsOffset, flags[i])), InvalidProtocolException)
            << "flags " << (int)flags[i];
    }
}

TEST(CLCreatePCFieldBounds, everyRealHairStyleAndSexIsAccepted) {
    const HairStyle styles[] = {HAIR_STYLE1, HAIR_STYLE2, HAIR_STYLE3};
    const Sex sexes[] = {FEMALE, MALE};
    const std::vector<unsigned char> base = writeBody(highestValidCreatePC(), kPlainCode);

    for (size_t s = 0; s < sizeof(styles) / sizeof(styles[0]); s++) {
        for (size_t x = 0; x < sizeof(sexes) / sizeof(sexes[0]); x++) {
            const unsigned char flags = (unsigned char)((sexes[x] == MALE ? 1 : 0) | (styles[s] << 1));
            CLCreatePC dst;
            readImage(dst, patched(base, kCreatePCFlagsOffset, flags));
            EXPECT_EQ(styles[s], dst.getHairStyle()) << "flags " << (int)flags;
            EXPECT_EQ(sexes[x], dst.getSex()) << "flags " << (int)flags;
        }
    }
}

TEST(CLCreatePCFieldBounds, bitsAboveTheFlagFieldAreIgnored) {
    // The flags land in a three-bit set, so bits 3..7 have never been part
    // of the packet. Reading stays indifferent to them: an image
    // whose hair-style and sex bits are valid is accepted whatever the
    // rest of the byte holds.
    const std::vector<unsigned char> base = writeBody(highestValidCreatePC(), kPlainCode);
    const unsigned char flags = (unsigned char)(0xF8 | 1 | (HAIR_STYLE1 << 1));

    CLCreatePC dst;
    readImage(dst, patched(base, kCreatePCFlagsOffset, flags));
    EXPECT_EQ(HAIR_STYLE1, dst.getHairStyle());
    EXPECT_EQ(MALE, dst.getSex());
}

//////////////////////////////////////////////////////////////////////
// CLSelectPC
//////////////////////////////////////////////////////////////////////

TEST(CLSelectPCFieldBounds, theImageUnderTestIsTheRealWireLayout) {
    const std::vector<unsigned char> bytes = writeBody(highestValidSelectPC(), kPlainCode);

    ASSERT_GT(bytes.size(), kSelectPCTypeOffset);
    EXPECT_EQ(std::string(kName).size(), (size_t)bytes[0]);
    EXPECT_EQ((unsigned char)PC_OUSTERS, bytes[kSelectPCTypeOffset]);
}

TEST(CLSelectPCFieldBounds, theHighestValidPCTypeReadsBack) {
    CLSelectPC dst;
    readImage(dst, writeBody(highestValidSelectPC(), kPlainCode));

    EXPECT_EQ(std::string(kName), dst.getPCName());
    EXPECT_EQ(PC_OUSTERS, dst.getPCType());
}

TEST(CLSelectPCFieldBounds, aPCTypeByteOutsideTheThreeTypesIsRefused) {
    // PC_OUSTERS is the last type, so 3 is the first byte that names none.
    const unsigned char types[] = {(unsigned char)(PC_OUSTERS + 1), 9, 255};
    const std::vector<unsigned char> base = writeBody(highestValidSelectPC(), kPlainCode);

    for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); i++) {
        CLSelectPC dst;
        EXPECT_THROW(readImage(dst, patched(base, kSelectPCTypeOffset, types[i])), InvalidProtocolException)
            << "pc type " << (int)types[i];
    }
}

TEST(CLSelectPCFieldBounds, everyRealPCTypeIsAccepted) {
    const PCType types[] = {PC_SLAYER, PC_VAMPIRE, PC_OUSTERS};
    const std::vector<unsigned char> base = writeBody(highestValidSelectPC(), kPlainCode);

    for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); i++) {
        CLSelectPC dst;
        readImage(dst, patched(base, kSelectPCTypeOffset, (unsigned char)types[i]));
        EXPECT_EQ(types[i], dst.getPCType());
    }
}

} // namespace
