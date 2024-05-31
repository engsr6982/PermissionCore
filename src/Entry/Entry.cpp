#include "Entry.h"

#include <memory>

#include "ll/api/i18n/I18n.h"
#include "ll/api/plugin/NativePlugin.h"
#include "ll/api/plugin/RegisterHelper.h"


// my
#include "Backend/API.h"
#include "Command/command.h"
#include "Config/Config.h"
#include "DB/db.h"


namespace pmc {

static std::unique_ptr<entry> instance;

entry& entry::getInstance() { return *instance; }

bool entry::load() {
    getSelf().getLogger().info("Loading...");

    ll::i18n::load(mSelf.getLangDir());
    pmc::db::getInstance().loadLevelDB();

    pmc::config::loadConfig();

    return true;
}

bool entry::enable() {
    getSelf().getLogger().info("Enabling...");

    pmc::command::registerCommand();

    pmc::backend::startAPIServerThread();

    return true;
}

bool entry::disable() {
    getSelf().getLogger().info("Disabling...");
    return true;
}

} // namespace pmc

LL_REGISTER_PLUGIN(pmc::entry, pmc::instance);
