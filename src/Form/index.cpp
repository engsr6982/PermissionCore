#include "index.h"
#include "mc/world/actor/player/Player.h"

namespace pmc::form {

void index(Player& player) { player.sendMessage("a"); }


} // namespace pmc::form