#include "tack/version.h"
#include "test.h"

void test_version_string(void)
{
    ASSERT_STREQ("version is 0.6.0", "0.6.0", tack_version_string());
    ASSERT_STREQ("name is Tack", "Tack", tack_name());
    ASSERT_EQ_INT("major", 0, TACK_VERSION_MAJOR);
    ASSERT_EQ_INT("minor", 6, TACK_VERSION_MINOR);
    ASSERT_EQ_INT("patch", 0, TACK_VERSION_PATCH);
}
