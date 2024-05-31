#include "API.h"
#include "fmt/compile.h"
#include "hv/json.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>

using string = std::string;
using ll::i18n_literals::operator""_tr;
using json   = nlohmann::json;
namespace fs = std::filesystem;


// 全局定义 HttpService 实例, 保证其生命周期与程序一致
hv::HttpService router;
hv::HttpServer  svr;


namespace pmc::backend {


void startAPIServerThread() {
    auto& cfg    = pmc::config::cfg.Network;
    auto& logger = pmc::entry::getInstance().getSelf().getLogger();
    if (!cfg.Enable) return;
    if (cfg.AllowCORS) router.AllowCORS(); // 允许跨域请求

    // 注册路由
    router.GET("/api/validate", [cfg](HttpRequest* req, HttpResponse* res) { CheckToken_RS(req, res, cfg); });

    // Query => plugin/perm/group
    router.GET("/api/query", [cfg](const HttpContextPtr& ctx) {
        CheckToken_CTX(ctx, cfg);
        auto&  pm     = pmc::PermissionManager::getInstance();
        string type   = ctx->param("type");
        string plugin = ctx->param("plugin");

        hv::Json res;
        int      code    = 200;
        string   message = "OK";

        // 查询指定类型数据
        if (type == "plugin") {
            res["data"] = pm.getAllKeys();
        } else if (type == "perm") {
            auto pem = pmc::PermissionRegister::getInstance();
            if (pem.hasPlugin(plugin)) {
                nlohmann::json j = nlohmann::json::array();
                for (auto& p : pem.getPermissions(plugin)) {
                    nlohmann::json j2;
                    j2["name"]  = p.name;
                    j2["value"] = p.value;
                    j.push_back(j2);
                }
                res["data"] = j;
            } else {
                code    = 404;
                message = "Not Found";
            }
        } else if (type == "group") {
            if (pm.hasRegisterPermissionCore(plugin)) {
                nlohmann::json j = nlohmann::json::array();
                // 遍历vector
                for (auto g : pm.getPermissionCore(plugin)->getAllGroups()) {
                    j.push_back(g.toJson());
                }
                res["data"] = j;
            } else {
                code    = 404;
                message = "Not Found";
            }
        } else {
            code    = 400;
            message = "Bad Request";
        }
        // 设置成功状态码和消息
        res["type"]    = type;
        res["plugin"]  = plugin;
        res["status"]  = code;
        res["message"] = message;
        // 发送响应
        return ctx->sendJson(res);
    });


    // 启动 HTTP 服务器
    svr.port    = cfg.Port;
    svr.service = &router;
    svr.setThreadNum(4); // 分配4个线程处理请求
    svr.start();
    logger.info("Start api server on 127.0.0.1:{0}."_tr(cfg.Port));
    if (cfg.Token == "default_token") {
        logger.warn("Please don't use default token in production environment."_tr());
    }
}

} // namespace pmc::backend