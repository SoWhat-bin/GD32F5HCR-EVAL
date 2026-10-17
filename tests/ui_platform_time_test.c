#include "ui_platform.h"

#include <assert.h>

int main(void)
{
    ui_platform_time_t valid = {2026, 10, 5, 12, 34, 56};
    ui_platform_time_t invalid = valid;

    assert(ui_platform_time_valid(&valid));
    assert(!ui_platform_time_valid(NULL));

    invalid.month = 0;
    assert(!ui_platform_time_valid(&invalid));
    invalid = valid;
    invalid.day = 32;
    assert(!ui_platform_time_valid(&invalid));
    invalid = valid;
    invalid.hour = 24;
    assert(!ui_platform_time_valid(&invalid));
    invalid = valid;
    invalid.minute = 60;
    assert(!ui_platform_time_valid(&invalid));
    invalid = valid;
    invalid.second = 60;
    assert(!ui_platform_time_valid(&invalid));

    return 0;
}
