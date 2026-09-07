#ifndef __SYSTEM_AVAILABILITY_REPOSITORY_H__
#define __SYSTEM_AVAILABILITY_REPOSITORY_H__

#include <vector>

#include "Types.h"

// SystemAvailabilities: the per-system on/off flags the server reads at
// boot, and the six rows the egg-dummy-DB shutdown path deletes on its
// way out.
//
// Neither caller reaches this on the shipped gameserver.
// SystemAvailabilitiesManager::load calls it only under
// "#if defined(__CHINA_SERVER__) || defined(__THAILAND_SERVER__)" and
// otherwise marks every system available; EventShutdown's deletes sit
// behind the "#else" of
// "#if !defined(__THAILAND_SERVER__) && !defined(__CHINA_SERVER__)". The
// gameserver build defines neither macro. (ItemUtil.cpp #defines
// __THAILAND_SERVER__ itself before including
// SystemAvailabilitiesManager.h, which gives that one TU a manager with
// an extra member and a different layout from every other TU's; neither
// caller lives there.)

// One row of the boot-time read, which is a "SELECT *": the positional
// getInt(1)/getInt(2) depend on SystemKind and Available being the first
// two columns (Description is third and unread). A column added before
// them would silently reassign every flag.
struct SystemAvailabilityRow {
    int systemKind;
    int available;
};

class SystemAvailabilityRepository {
public:
    virtual ~SystemAvailabilityRepository() {}

    // SystemAvailabilitiesManager::load.
    virtual std::vector<SystemAvailabilityRow> loadAll() = 0;

    // EventShutdown's teardown: DELETE the row of one system kind. The
    // statement quotes the value ("'%d'"); SystemKind is int(11), so it
    // still compares as a number.
    virtual void deleteSystemKind(int systemKind) = 0;
};

// The process-wide MySQL-backed instance, wired in
// MySQLSystemAvailabilityRepository.cpp.
SystemAvailabilityRepository& defaultSystemAvailabilityRepository();

#endif
