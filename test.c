#include <stdio.h>
#define SV_IMPLEMENTATION
#include "./sv.h"

int main() {
    String_View s = sv_from_cstr("   Hello, World!   ");
    sv_trim(s);
    String_View trimmed = sv_chop_by_delim(&s, ',');
    printf("|" SV_Fmt "|\n", SV_Arg(trimmed));
    printf("|" SV_Fmt "|\n", SV_Arg(s));
    return 0;
}
