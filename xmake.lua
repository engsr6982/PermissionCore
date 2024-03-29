add_rules("mode.debug", "mode.release")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")

-- add_requires("levilamina x.x.x") for a specific version
-- add_requires("levilamina develop") to use develop version
-- please note that you should add bdslibrary yourself if using dev version
add_requires("levilamina 0.9.4")

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

target("PermissionCore") -- Change this to your plugin name.
    add_cxflags("/EHa", "/utf-8")
    add_defines(
        "NOMINMAX", 
        "UNICODE",
        "PERMISSION_CORE_API_EXPORT" -- export dll api
    )
    add_files("src/**.cpp")
    add_includedirs("src", "include")
    add_packages("levilamina")
    add_shflags("/DELAYLOAD:bedrock_server.dll") -- To use symbols provided by SymbolProvider.
    set_exceptions("none") -- To avoid conflicts with /EHa.
    set_kind("shared")
    set_languages("c++20")
    set_symbols("debug")

    after_build(function (target)
        local plugin_packer = import("scripts.after_build")

        local plugin_define = {
            pluginName = target:name(),
            pluginFile = path.filename(target:targetfile()),
        }
        
        plugin_packer.pack_plugin(target,plugin_define)
    end)
