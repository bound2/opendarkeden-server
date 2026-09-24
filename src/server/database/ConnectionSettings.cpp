//////////////////////////////////////////////////////////////////////////////
// Filename    : ConnectionSettings.cpp
// Description : The configuration keys one database connection is built from.
//////////////////////////////////////////////////////////////////////////////

#include "ConnectionSettings.h"

#include "Properties.h"

namespace de {

ConnectionSettings connectionSettings(const Properties& config, const std::string& prefix) {
    ConnectionSettings settings;

    settings.host = config.getProperty(prefix + "_HOST");
    settings.db = config.getProperty(prefix + "_DB");
    settings.user = config.getProperty(prefix + "_USER");
    settings.password = config.getProperty(prefix + "_PASSWORD");

    const std::string portKey = prefix + "_PORT";
    if (config.hasKey(portKey))
        settings.port = (uint)config.getPropertyInt(portKey);

    return settings;
}

} // namespace de
