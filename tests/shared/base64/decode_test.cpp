#define BOOST_TEST_MODULE Base64 Decode Test
#include <boost/test/included/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "shared/base64/decode.h"
#include <vector>
#include <utility>

namespace bdata = boost::unit_test::data;
BOOST_DATA_TEST_CASE(
    decode_test,
    bdata::make(
    std::vector<std::tuple<std::string,std::string>>{
        {"Ym9iIHRoZSBidWlsZGVy", "bob the builder"},
        {"MTIzYWxkO2tmamEtMjM0YWxmcztka2pm", "123ald;kfja-234alfs;dkjf"}
    }),
    data, ref
)
{
    BOOST_CHECK_EQUAL(shared::base64::decode(data), ref);
}