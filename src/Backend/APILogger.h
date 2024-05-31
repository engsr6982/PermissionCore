#include "hv/HttpContext.h"
#include "hv/HttpMessage.h"
#include <filesystem>
#include <fstream>
#include <string>


using string = std::string;
namespace fs = std::filesystem;

namespace pmc::backend {

class APILogger {
private:
    string        mCurrTime;
    std::ofstream mOpenedFile;

    void checkAndUpdateCurrTime();

    fs::path getAndCreateLogDir();

    void openFile();

public:
    void checkFile();

    void writeLine(const string& line);

    static APILogger& getInstance();

    static void log(HttpRequest* req);
    static void log(const HttpContextPtr& ctx);
};

} // namespace pmc::backend
