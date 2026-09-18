////////////////////////////////////////////////////////////////////////////////
// Filename    : DynamicZoneFactoryManager.cpp
// Written By  :
// Description :
////////////////////////////////////////////////////////////////////////////////

#include "DynamicZoneFactoryManager.h"

#include "Assert.h"
#include "DynamicZoneAlterOfBlood.h"
#include "DynamicZoneGateOfAlter.h"
#include "DynamicZoneInfo.h"
#include "DynamicZoneOustersMirrorOfAbyss.h"
#include "DynamicZoneSlayerMirrorOfAbyss.h"
#include "DynamicZoneVampireMirrorOfAbyss.h"
#include "StringStream.h"

////////////////////////////////////////////////////////////////////////////////
// constructor
////////////////////////////////////////////////////////////////////////////////
DynamicZoneFactoryManager::DynamicZoneFactoryManager() : m_Factories(NULL), m_Size(DYNAMIC_ZONE_MAX) {
    Assert(m_Size > 0);

    // Create the factory array.
    m_Factories = new DynamicZoneFactory*[m_Size];

    // Initialize the factory pointers to NULL.
    for (int i = 0; i < m_Size; i++)
        m_Factories[i] = NULL;
}


////////////////////////////////////////////////////////////////////////////////
// destructor
////////////////////////////////////////////////////////////////////////////////
DynamicZoneFactoryManager::~DynamicZoneFactoryManager() {
    Assert(m_Factories != NULL);

    // Delete each factory.
    for (int i = 0; i < m_Size; i++) {
        if (m_Factories[i] != NULL) {
            delete m_Factories[i];
            m_Factories[i] = NULL;
        }
    }

    // Delete the factory array.
    delete[] m_Factories;
    m_Factories = NULL;
}


////////////////////////////////////////////////////////////////////////////////
// Add every defined factory here.
////////////////////////////////////////////////////////////////////////////////
void DynamicZoneFactoryManager::init() {
    addFactory(new DynamicZoneGateOfAlterFactory());
    addFactory(new DynamicZoneAlterOfBloodFactory());
    addFactory(new DynamicZoneSlayerMirrorOfAbyssFactory());
    addFactory(new DynamicZoneVampireMirrorOfAbyssFactory());
    addFactory(new DynamicZoneOustersMirrorOfAbyssFactory());
}


////////////////////////////////////////////////////////////////////////////////
// add dynamiczone factory to factories array
////////////////////////////////////////////////////////////////////////////////
void DynamicZoneFactoryManager::addFactory(DynamicZoneFactory* pFactory) {
    Assert(pFactory != NULL);

    if (m_Factories[pFactory->getDynamicZoneType()] != NULL) {
        StringStream msg;
        msg << "duplicate DynamicZone factories, " << pFactory->getDynamicZoneName();
        cout << msg.toString() << endl;
        Assert(false);
    }

    // Register the factory.
    m_Factories[pFactory->getDynamicZoneType()] = pFactory;
}


////////////////////////////////////////////////////////////////////////////////
// create dynamiczone object with dynamiczone type
////////////////////////////////////////////////////////////////////////////////
DynamicZone* DynamicZoneFactoryManager::createDynamicZone(int dynamicZoneType) const {
    if (dynamicZoneType >= m_Size || m_Factories[dynamicZoneType] == NULL) {
        StringStream msg;
        msg << "dynamiczone factory [" << dynamicZoneType << "] not exist.";
        cout << msg.toString() << endl;
        Assert(false);
    }

    return m_Factories[dynamicZoneType]->createDynamicZone();
}


////////////////////////////////////////////////////////////////////////////////
// get dynamiczone name with dynamiczone type
////////////////////////////////////////////////////////////////////////////////
string DynamicZoneFactoryManager::getDynamicZoneName(int dynamicZoneType) const {
    // Guard against a segmentation fault caused by an out-of-range type.
    // Such a call is a programming error.
    if (dynamicZoneType >= m_Size || m_Factories[dynamicZoneType] == NULL) {
        StringStream msg;
        msg << "invalid dynamiczone type (" << dynamicZoneType << ")";
        cout << msg.toString() << endl;
        Assert(false);
    }

    return m_Factories[dynamicZoneType]->getDynamicZoneName();
}


////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
int DynamicZoneFactoryManager::getDynamicZoneType(const string& dynamicZoneName) const {
    for (int i = 0; i < m_Size; i++) {
        if (m_Factories[i] != NULL) {
            if (m_Factories[i]->getDynamicZoneName() == dynamicZoneName) {
                return i;
            }
        }
    }

    string msg = "no such dynamiczone type : " + dynamicZoneName;
    cout << msg << endl;
    Assert(false);

    return DYNAMIC_ZONE_MAX;
}


////////////////////////////////////////////////////////////////////////////////
// get debug string
////////////////////////////////////////////////////////////////////////////////
string DynamicZoneFactoryManager::toString() const {
    StringStream msg;

    msg << "DynamicZoneFactoryManager(\n";

    for (int i = 0; i < m_Size; i++) {
        msg << "DynamicZoneFactories[" << i << "] == ";
        msg << (m_Factories[i] == NULL ? "NULL" : m_Factories[i]->getDynamicZoneName());
        msg << "\n";
    }

    msg << ")";

    return msg.toString();
}
