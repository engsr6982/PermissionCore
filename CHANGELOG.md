# Changelog

所有对该项目的显著更改将记录在此文件中。

格式基于[Keep a Changelog](https://keepachangelog.com/en/1.0.0/)，
并且该项目遵循[Semantic Versioning](https://semver.org/spec/v2.0.0.html)。

## [Unreleased]

## [0.10.0] - 2024-6-20

### Changed

- 适配 LeviLamina 0.13.0

## [0.9.0] - 2024-5-15

### Changed

- 更改插件全局命名空间`perm` => `pmc`

## [0.8.0] - 2024-4-29

### Changed

- 适配 Levilamina 0.12.1

## [0.7.0] - 2024-4-21

### Added

- Add i18n support.

### Changed

- Update Plugin-template.
- Refactor Command.

## [0.6.2] - 2024-4-16

### Fixed

- Fixed tooth.json

## [0.6.1] - 2024-4-16

### Apadted

- Supported Levilamina 0.11.0

## [0.6.0] - 2024-4-11

### Added

- Add Command Support.

## [0.5.0] - 2024-4-2

### Changed

- try support Levilamina 0.10.5

## [0.4.1] - 2024-3-31

### Fixed

- fix: fixed removeUserToGroup、addUserToGroup、removePermissionToGroup

## [0.4.0] - 2024-3-30

### Changed

- feat: update group.h
- refactor: db.h & db.cpp
- refactor: permissioncore
- feat: add hasRegisterPermissionCore
- feat: add Form & Command
- chore: remove enablePublicGroups in constructor
- chore: creat Permission.h
- refactor: PermissionRegister

## [0.3.0] - 2024-03-27

### Changed

- 更改头文件结构
- 调整部分 API

### Fixed

- 修复一些问题

## [0.2.1] - 2024-03-27

### Removed

- 头文件“Register.h”移除`registerPerm`声明(修复编译)

## [0.2.0] - 2024-03-27

### Added

- 添加`PermissionManager`

### Changed

- 更改宏 "PERMISSION_CORE_API" 为 "PermExports"
- 更改 SDK 文件结构

## [0.1.0] - 2024-03-20

### Added

- 完成权限核心功能。
