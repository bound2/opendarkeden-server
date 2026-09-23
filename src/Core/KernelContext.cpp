//////////////////////////////////////////////////////////////////////////////
// Filename    : KernelContext.cpp
// Description : Accessors for the managers registered on a KernelContext.
//
//               The manager types stay incomplete here: an accessor only
//               binds a reference to the registered object, so nothing in
//               this file needs a manager's definition. That is what lets the
//               kernel carry the registry while the packet factory table and
//               the validator are compiled per server, outside it.
//////////////////////////////////////////////////////////////////////////////

#include "KernelContext.h"

#include "Assert.h"

namespace de {

Properties& KernelContext::config() const {
    Assert(m_pConfig != nullptr);
    return *m_pConfig;
}

PacketFactoryManager& KernelContext::packetFactories() const {
    Assert(m_pPacketFactoryManager != nullptr);
    return *m_pPacketFactoryManager;
}

PacketValidator& KernelContext::packetValidator() const {
    Assert(m_pPacketValidator != nullptr);
    return *m_pPacketValidator;
}

KernelContext& kernelContext() {
    static KernelContext context;
    return context;
}

} // namespace de
