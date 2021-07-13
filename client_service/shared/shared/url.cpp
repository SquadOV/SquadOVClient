#include "shared/url.h"
#include "shared/errors/error.h"

#include <curl/curl.h>

namespace shared::url {

std::string urlEncode(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (curl) {
        char* output = curl_easy_escape(curl, url.data(), url.size());
        if(output) {
            std::string ret(output);
            curl_free(output);
            return ret;
        } else {
            THROW_ERROR("Failed to URL escape.");
        }
    } else {
        THROW_ERROR("CURL not init.");
    }
    return "";
}

}