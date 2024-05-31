#include "API.h"
#include "fmt/compile.h"
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

    // 注册路由
    router.GET("/api/validate", [cfg](HttpRequest* req, HttpResponse* res) { CheckToken_RS(req, res, cfg); });

    // Query => plugin/perm
    router.GET("/api/query/{type}", [cfg](const HttpContextPtr& ctx) {
        CheckToken_CTX(ctx, cfg);
        string   type = ctx->param("type");
        hv::Json res;
        res["type"] = type;

        // 查询指定类型数据
        if (type == "plugin") {
            res["data"] = pmc::PermissionManager::getInstance().getAllKeys();
        } else if (type == "perm") {

        } else {
            res["status"]  = 400;
            res["message"] = "Bad Request";
        }
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
        logger.warn("Token is not set, please set it in config.toml."_tr());
    }
}

} // namespace pmc::backend