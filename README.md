# PermissionCore 权限组核心

一个适用于`Levilamina`的非官方权限组前置, 旨在为`Levilamina`原生插件提供一个权限组轮子

## 功能

本插件对外提供的 API，主要分为以下三大块

- 权限核心  
  每个插件创建一个自己的核心，提供权限核心 API

- 权限管理  
  每个插件创建的核心注册到管理进行全局管理，开发者可以从这里获取已注册的权限核心

- 权限注册  
  插件需要把权限注册到这个 API 中，这可以为将来的 Command、GUI 提供查询支持

## 安装使用

### 用户

使用 Lip 一键安装

```bash
lip install github.com/engsr6982/PermissionCore
```

安装插件后启动服务器，控制台输入`? permc`查看命令列表

### 开发者

> 以下是按照传统的 submodule 形式使用 PermissionCore  
> 如果需要 xmake 来管理 PermissionCore，请参考测试仓库的[xmake.lua](https://github.com/engsr6982/PermissionCoreTest/blob/main/xmake.lua)

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

打开`xmake.lua`，在`target`后面加上

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
