#include "Config.h"

#include "Entry/Entry.h"
#include "ll/api/Config.h"
#include <filesystem>


namespace pmc::config {

mConfig cfg;

void loadConfig() {
    auto& mSelf  = pmc::entry::getInstance().getSelf();
    auto& logger = mSelf.getLogger();
    auto  path   = mSelf.getConfigDir() / "Config.json";

    if (!std::filesystem::exists(path)) {
        ll::config::saveConfig(cfg, path);
        logger.warn("Configuration file does not exist, a default configuration file has been generated");
    }

    ll::config::loadConfig(cfg, path);
}

} // namespace pmc::config