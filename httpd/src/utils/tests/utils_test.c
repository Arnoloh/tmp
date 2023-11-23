#include <criterion/criterion.h>

#include "utils/string/string.h"

Test(utils, utils)
{}

TestSuite(String);

Test(String, test_full)
{
    struct string *string = string_create("Hello World!", 28);
    string_concat_str(string, " Mamama", 8);
    string_destroy(string);
}
