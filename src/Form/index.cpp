#include "index.h"
#include "PermissionCore/PermissionCore.h"
#include "PermissionCore/PermissionManager.h"
#include "PermissionCore/PermissionRegister.h"
#include "ll/api/form/CustomForm.h"
#include "ll/api/form/FormBase.h"
#include "ll/api/form/ModalForm.h"
#include "ll/api/form/SimpleForm.h"
#include "mc/world/actor/player/Player.h"
#include <sstream>


using namespace ll::form;
#define TITLE "§aPermissionCore§r & §e管理面板"

namespace pmc::form {

// utils
void sendMsg(Player& p, string const& msg) {
    std::stringstream ss;
    ss << "§a[PermissionCore] §r" << msg;
    p.sendMessage(ss.str());
}

// func
void _continueForm(Player& player) {
    ModalForm fm;
    fm.setTitle(TITLE);
    fm.setContent("§a§l• 操作完成\n是否继续?"_tr());
    fm.setUpperButton("§a§l继续"_tr());
    fm.setLowerButton("§c§l取消"_tr());
    fm.sendTo(player, [](Player& p, ModalFormResult const& val, FormCancelReason) {
        if (!val) {
            sendMsg(p, "表单已放弃"_tr());
            return;
        }
        if ((bool)val.value()) index(p);
        else sendMsg(p, "表单已取消"_tr());
    });
}


void index(Player& player) { _selectPlugin(player); }

void _selectPlugin(Player& player) {
    SimpleForm fm;
    fm.setTitle(TITLE);
    fm.setContent("§a§l• 选择目标插件"_tr());

    auto plugins = PermissionManager::getInstance().getAllPluginNames();
    for (auto const& plugin : plugins) {
        fm.appendButton(plugin, [plugin](Player& p) { _pluginOperationPanel(p, plugin); });
    }

    fm.sendTo(player);
}


void _pluginOperationPanel(Player& player, const string targetPluginName) {
    SimpleForm fm;
    fm.setTitle(TITLE);
    fm.setContent("§a§l• {0} 插件操作面板"_tr(targetPluginName));

    fm.appendButton("返回上一页"_tr(), "", "path", [](Player& p) { index(p); });

    fm.appendButton("组管理"_tr(), [targetPluginName](Player& p) { _selectGroup(p, targetPluginName); });
    fm.appendButton("权限列表"_tr(), [targetPluginName](Player& p) { _showPluginPermissions(p, targetPluginName); });
    fm.appendButton("搜索"_tr(), [targetPluginName](Player& p) {
        // TODO: search permission
    });

    fm.sendTo(player);
}


void _showPluginPermissions(Player& player, const string targetPluginName) {
    CustomForm fm;
    fm.setTitle(TITLE);
    fm.appendLabel("§a§l• {0} 插件权限列表"_tr(targetPluginName));

    auto permissions = PermissionRegister::getInstance().getPermissions(targetPluginName);
    for (auto const& pm : permissions) {
        fm.appendLabel("[权限名]: {0}\n[权限值]: {1}"_tr(pm.name, pm.value));
    }

    fm.appendLabel("§a§l• 共计 {0} 个权限"_tr(permissions.size()));
    fm.sendTo(player, [](Player& p, CustomFormResult const&, FormCancelReason) { _continueForm(p); });
}


void _selectGroup(Player& player, const string targetPluginName) {
    SimpleForm fm;
    fm.setTitle(TITLE);

    fm.appendButton("返回上一页"_tr(), "", "path", [targetPluginName](Player& p) {
        _pluginOperationPanel(p, targetPluginName);
    });
    fm.appendButton("创建新组"_tr(), [targetPluginName](Player& p) {
        // TODO: create new group
    });

    auto groups = PermissionManager::getInstance().getPermissionCore(targetPluginName)->getAllGroups();
    for (auto const& group : groups) {
        string targetGroupName = group.groupName;
        fm.appendButton(targetGroupName, [targetPluginName, targetGroupName](Player& p) {
            _showGroupInfoAndEditPanel(p, targetPluginName, targetGroupName);
        });
    }

    fm.setContent("§a§l• {0} 插件组管理, 共计 {1} 个组"_tr(targetPluginName, groups.size()));
    fm.sendTo(player);
}


void _showGroupInfoAndEditPanel(Player& player, const string targetPluginName, const string targetGroupName) {
    SimpleForm fm;
    fm.setTitle(TITLE);

    auto group = *PermissionManager::getInstance().getPermissionCore(targetPluginName)->getGroup(targetGroupName);
    fm.setContent("§a§l• 正在编辑 {0} 插件。§r\n[组名]: {1}\n[用户]: {2}\n[权限]: {3}\n[状态]: {4}\n[忽略]: {5}\n "_tr(
        targetPluginName,
        group.groupName,
        group.userList.size(),
        group.permissionList.size(),
        (group.status == group::GroupStatus::Open ? "启用"_tr() : "禁用"_tr()),
        (group.ignoreListType == group::IgnoreListType::None
             ? "无"_tr()
             : (group.ignoreListType == group::IgnoreListType::UserList ? "用户"_tr() : "权限"_tr()))
    ));

    fm.appendButton("编辑用户列表"_tr(), [targetPluginName, targetGroupName](Player& p) {});
    fm.appendButton("编辑权限列表"_tr(), [targetPluginName, targetGroupName](Player& p) {});
    fm.appendButton("编辑组状态"_tr(), [targetPluginName, targetGroupName](Player& p) {});
    fm.appendButton("编辑忽略列表"_tr(), [targetPluginName, targetGroupName](Player& p) {});
    fm.appendButton("重命名此组"_tr(), [targetPluginName, targetGroupName](Player& p) {});
    fm.appendButton("删除此组"_tr(), [targetPluginName, targetGroupName](Player& p) {});

    fm.appendButton("返回上一页"_tr(), "", "path", [targetPluginName](Player& p) {
        _selectGroup(p, targetPluginName);
    });

    fm.sendTo(player);
}

} // namespace pmc::form