#define BOOST_TEST_MODULE Base64 Encode Test
#include <boost/test/included/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include "shared/base64/encode.h"
#include <vector>
#include <utility>

namespace bdata = boost::unit_test::data;
BOOST_DATA_TEST_CASE(
    encode_test,
    bdata::make(
    std::vector<std::tuple<std::string,std::string>>{
        {"bob the builder", "Ym9iIHRoZSBidWlsZGVy"},
        {"123ald;kfja-234alfs;dkjf", "MTIzYWxkO2tmamEtMjM0YWxmcztka2pm"}
    }),
    data, ref
)
{
    BOOST_CHECK_EQUAL(shared::base64::encode(data), ref);
}