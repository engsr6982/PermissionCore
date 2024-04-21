#include "Entry.h"

#include <memory>

#include "ll/api/plugin/NativePlugin.h"
#include "ll/api/plugin/RegisterHelper.h"

// my
#include "Command/command.h"
#include "DB/db.h"

namespace perm {

static std::unique_ptr<entry> instance;

entry& entry::getInstance() { return *instance; }

bool entry::load() {
    getSelf().getLogger().info("Loading...");

    ll::i18n::load(getSelf().getLangDir());
    perm::db::getInstance().loadLevelDB();

    return true;
}

bool entry::enable() {
    getSelf().getLogger().info("Enabling...");

    perm::command::registerCommand();
    return true;
}

bool entry::disable() {
    getSelf().getLogger().info("Disabling...");
    return false;
}

} // namespace perm

LL_REGISTER_PLUGIN(perm::entry, perm::instance);
