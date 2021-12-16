#define BOOST_TEST_MODULE VDF File Test
#include <boost/test/included/unit_test.hpp>
#include <filesystem>
#include <string>

#include "shared/valve/vdf_file.h"

namespace fs = std::filesystem;
BOOST_AUTO_TEST_CASE(test_vdf_load_from_file)
{
    const auto vdf = shared::valve::readVdfFile(fs::path("data/test_vdf_load_from_file.vdf"));
    BOOST_TEST(vdf.hasKey("libraryfolders"));

    const auto& libraryFolders = vdf["libraryfolders"];
    BOOST_TEST(libraryFolders.hasKey("contentstatsid"));
    BOOST_CHECK_EQUAL(libraryFolders["contentstatsid"].value(), "-6447241787288536465");

    BOOST_TEST(libraryFolders.hasKey("0"));
    {
        const auto& lib = libraryFolders["0"];
        BOOST_CHECK_EQUAL(lib["path"].value(), "C:\\Program Files (x86)\\Steam");
        BOOST_CHECK_EQUAL(lib["label"].value(), "");
        BOOST_CHECK_EQUAL(lib["contentid"].value(), "-6447241787288536465");
        BOOST_CHECK_EQUAL(lib["totalsize"].value(), "0");
        BOOST_CHECK_EQUAL(lib["update_clean_bytes_tally"].value(), "21677650284");
        BOOST_CHECK_EQUAL(lib["time_last_update_corruption"].value(), "0");
        BOOST_TEST(lib.hasKey("apps"));
        {
            const auto& apps = lib["apps"];
            BOOST_CHECK_EQUAL(apps["745"].value(), "158517065");
            BOOST_CHECK_EQUAL(apps["39210"].value(), "54893900");
        }
    }

    BOOST_TEST(libraryFolders.hasKey("1"));
    {
        const auto& lib = libraryFolders["1"];
        BOOST_CHECK_EQUAL(lib["path"].value(), "E:\\SteamLibrary");
        BOOST_CHECK_EQUAL(lib["label"].value(), "");
        BOOST_CHECK_EQUAL(lib["contentid"].value(), "7378487568877460732");
        BOOST_CHECK_EQUAL(lib["totalsize"].value(), "4000785100800");
        BOOST_CHECK_EQUAL(lib["update_clean_bytes_tally"].value(), "29433128373");
        BOOST_CHECK_EQUAL(lib["time_last_update_corruption"].value(), "0");
        BOOST_TEST(lib.hasKey("apps"));
        {
            const auto& apps = lib["apps"];
            BOOST_CHECK_EQUAL(apps["730"].value(), "0");
        }
    }
}