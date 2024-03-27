# PermissionCore 权限组核心

一个适用于`Levilamina`的非官方权限组前置

## 功能

- 管理组  
    可以理解为插件的全局管理员
- 用户组  
    细分插件权限给用户
- 公共组  
    开放权限给所有玩家
- 权限注册  
    动态注册权限（为数据库中的权限值提供翻译）
- GUI全局管理  
    todo...

## 实现

本插件照搬的自己的JS版本（GUI还没搬完）

采用`LevelDB`存储数据，对每个插件的数据进行存储

权限注册采用动态注册（也就是不会保存）

## How to use?

以下是按照传统的submodule形式使用PermissionCore  
如果需要xmake来管理PermissionCore，请参考测试仓库的[xmake.lua](https://github.com/engsr6982/PermissionCoreTest/blob/main/xmake.lua)

- Step1:

从`Release`下载`SDK`，解压到你的项目仓库根目录的`SDK_PermissonCore`下

```file
project:
├─SDK_PermissionCore
│    │  include_all.h
│    │
│    ├─Lib
│    │      PermissionCore.lib
│    │
│    └─PermissionCore
│        ├─PermissionCore
│        │      PermissionCore.h
│        │
│        └─Registers
│                Registers.h
│
├─src
...省略其他内容
```

- Step2:

打开`xmake.lua`

在`target`后面加上

```lua
target("Levilamina-Plugin-Template")
    add_includedirs(
        "SDK_PermissonCore", -- 让xmake识别到头文件
        "src"
    )
    add_links(
        "SDK_PermissonCore/Lib/PermissionCore" -- 为编译链接库
    )
    -- 省略其他内容
```
