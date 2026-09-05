#include "missnotepad/version.h"
#include "test.h"

void test_version_string(void)
{
    ASSERT_STREQ("version is 1.0.0", "1.0.0", missnotepad_version_string());
    ASSERT_STREQ("name is MissNotepad", "MissNotepad", missnotepad_name());
    ASSERT_EQ_INT("major", 1, MISSNOTEPAD_VERSION_MAJOR);
    ASSERT_EQ_INT("minor", 0, MISSNOTEPAD_VERSION_MINOR);
    ASSERT_EQ_INT("patch", 0, MISSNOTEPAD_VERSION_PATCH);
}
