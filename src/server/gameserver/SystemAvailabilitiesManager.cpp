#include "SystemAvailabilitiesManager.h"

#include "Assert.h"
#include "repository/SystemAvailabilityRepository.h"

void SystemAvailabilitiesManager::load() {
    __BEGIN_TRY

#if defined(__CHINA_SERVER__) || defined(__THAILAND_SERVER__)
    vector<SystemAvailabilityRow> rows = defaultSystemAvailabilityRepository().loadAll();

    {
        for (size_t r = 0; r < rows.size(); r++) {
            int ID = rows[r].systemKind;
            bool Avail = rows[r].available != 0;

            if (ID == OpenDegreeID) {
                m_ZoneOpenDegree = rows[r].available;
                continue;
            }

            if (ID == SkillLimitID) {
                m_SkillLevelLimit = rows[r].available;
                continue;
            }

            if (ID == ItemLevelLimitID) {
                m_ItemLevelLimit = rows[r].available;
                continue;
            }

            if (ID >= (int)SYSTEM_MAX) {
                cout << "SystemAvailabilitiesManager::load() : Invalid System Kind!" << ID << endl;
                Assert(false);
            }

            SystemKind kind = (SystemKind)ID;
            setAvailable(kind, Avail);
        }
    }

#else
    for (int i = 0; i < SYSTEM_MAX; ++i)
        setAvailable((SystemKind)i, true);

#endif

    if (m_pAvailabilitiesPacket == NULL)
        m_pAvailabilitiesPacket = new GCSystemAvailabilities();
    m_pAvailabilitiesPacket->setFlag((DWORD)m_SystemFlags.to_ulong());
    m_pAvailabilitiesPacket->setOpenDegree(m_ZoneOpenDegree);
    m_pAvailabilitiesPacket->setSkillLimit(m_SkillLevelLimit);
    m_bEdited = false;

    __END_CATCH
}
