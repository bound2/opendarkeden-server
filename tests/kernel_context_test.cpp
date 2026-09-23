//----------------------------------------------------------------------
// kernel_context_test.cpp
//
// Pins src/Core/KernelContext.h: the registry that carries the
// process-wide managers whose classes live in src/Core -- the
// configuration, the packet factory table and the packet validator --
// instead of the g_p* globals each server used to assign.
//
// The managers are never defined here. KernelContext.h forward-declares
// them and its accessors only bind a reference to the registered object,
// so a context can be built over stand-in pointers with none of the
// servers linked. That is also what lets the registry be a kernel file
// while the factory table and the validator are compiled per server,
// outside the kernel. The suite links de-kernel alone, which already
// carries the context's translation unit and the Assert helper.
//
// A failing Assert appends to assertion_failed.log in the working
// directory, so the ctest entry runs this from the build tree.
//----------------------------------------------------------------------

#include <gtest/gtest.h>

#include "Exception.h"
#include "KernelContext.h"

namespace {

// Distinct addresses standing in for the managers. Nothing dereferences
// them: the context stores a pointer and hands back a reference to it.
char g_managerStorage[16];

template <class T> T* standIn(int slot) {
    return reinterpret_cast<T*>(&g_managerStorage[slot]);
}

} // namespace

TEST(KernelContextTest, RegisteredManagersAreReadBack) {
    de::KernelContext context;

    Properties* pConfig = standIn<Properties>(0);
    PacketFactoryManager* pPacketFactoryManager = standIn<PacketFactoryManager>(1);
    PacketValidator* pPacketValidator = standIn<PacketValidator>(2);

    context.setConfig(pConfig);
    context.setPacketFactoryManager(pPacketFactoryManager);
    context.setPacketValidator(pPacketValidator);

    EXPECT_EQ(&context.config(), pConfig);
    EXPECT_EQ(&context.packetFactories(), pPacketFactoryManager);
    EXPECT_EQ(&context.packetValidator(), pPacketValidator);
}

TEST(KernelContextTest, ReregisteringReplacesTheManager) {
    de::KernelContext context;

    context.setConfig(standIn<Properties>(0));
    context.setConfig(standIn<Properties>(1));

    EXPECT_EQ(&context.config(), standIn<Properties>(1));
}

TEST(KernelContextTest, UnregisteredManagerAsserts) {
    de::KernelContext context;

    // Reading a manager the startup code has not registered yet is a
    // startup-order bug, so every accessor asserts rather than returning
    // something the caller could test.
    EXPECT_THROW(context.config(), AssertionError);
    EXPECT_THROW(context.packetFactories(), AssertionError);
    EXPECT_THROW(context.packetValidator(), AssertionError);
}

TEST(KernelContextTest, RegisteringOneManagerLeavesTheOthersAsserting) {
    de::KernelContext context;

    context.setPacketValidator(standIn<PacketValidator>(2));

    EXPECT_EQ(&context.packetValidator(), standIn<PacketValidator>(2));
    EXPECT_THROW(context.config(), AssertionError);
}

TEST(KernelContextTest, ProcessWideContextIsOneInstance) {
    EXPECT_EQ(&de::kernelContext(), &de::kernelContext());
}
