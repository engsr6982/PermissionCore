#pragma once
#include "APILogger.h"
#include "Config/Config.h"
#include "Date.h"
#include "Entry/Entry.h"
#include "PermissionCore/Group.h"
#include "PermissionCore/PermissionCore.h"
#include "PermissionCore/PermissionManager.h"
#include "PermissionCore/PermissionRegister.h"
#include "hv/HttpContext.h"
#include "hv/HttpMessage.h"
#include "hv/HttpServer.h"
#include "hv/HttpService.h"
#include "hv/http_content.h"
#include "hv/httpdef.h"
#include "ll/api/Logger.h"
#include "nlohmann/json.hpp"
#include "string"
#include <filesystem>
#include <functional>
#include <ll/api/i18n/I18n.h>
#include <memory>
#include <stdexcept>
#include <thread>


namespace pmc::backend {

void startAPIServerThread();

#define CheckToken_RS(req, res, cfg)                                                                                   \
    APILogger::log(req);                                                                                               \
    if (req->GetHeader("Authorization") != "Bearer " + cfg.Token) {                                                    \
        res->json["status"]  = 401;                                                                                    \
        res->json["message"] = "Unauthorized";                                                                         \
        return 401;                                                                                                    \
    }                                                                                                                  \
    res->json["status"]  = 200;                                                                                        \
    res->json["message"] = "OK";                                                                                       \
    return 200;

#define CheckToken_CTX(ctx, cfg)                                                                                       \
    APILogger::log(ctx);                                                                                               \
    string _token = ctx->header("Authorization");                                                                      \
    if (_token != "Bearer " + cfg.Token) {                                                                             \
        ctx->setStatus(http_status::HTTP_STATUS_UNAUTHORIZED);                                                         \
        hv::Json _tres;                                                                                                \
        _tres["status"]  = 401;                                                                                        \
        _tres["message"] = "Unauthorized";                                                                             \
        return ctx->sendJson(_tres);                                                                                   \
    }

} // namespace pmc::backend