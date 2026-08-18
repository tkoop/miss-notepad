#include "tack/version.h"
#include "test.h"

void test_version_string(void)
{
    ASSERT_STREQ("version is 1.0.0", "1.0.0", tack_version_string());
    ASSERT_STREQ("name is Tack", "Tack", tack_name());
    ASSERT_EQ_INT("major", 1, TACK_VERSION_MAJOR);
    ASSERT_EQ_INT("minor", 0, TACK_VERSION_MINOR);
    ASSERT_EQ_INT("patch", 0, TACK_VERSION_PATCH);
}
