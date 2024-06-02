#include "ll/api/i18n/I18n.h"
#include "mc/world/actor/player/Player.h"


using ll::i18n_literals::operator""_tr;
using string = std::string;

namespace pmc::form {

void index(Player& player);

void _continueForm(Player& player);

void _selectPlugin(Player& player);

void _pluginOperationPanel(Player& player, const string targetPluginName);

void _showPluginPermissions(Player& player, const string targetPluginName);

void _selectGroup(Player& player, const string targetPluginName);

void _showGroupInfoAndEditPanel(Player& player, const string targetPluginName, const string targetGroupName);

} // namespace pmc::form