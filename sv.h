#ifndef SV_H_
#define SV_H_

#include <ctype.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

#ifndef SVDEF
#define SVDEF
#endif /* ifndef SVDEF */

typedef struct {
    const char *data;
    size_t count;
} String_View;

#define SV(cstr_lit) sv_from_parts(cstr_lit, sizeof(cstr_lit) - 1)
#define SV_STATIC(cstr_lit) \
    { \
        sizeof(cstr_lit) - 1, \
        (cstr_lit) \
    } 

#define SV_NULL sv_from_parts(NULL, 0)

#define SV_Fmt "%.*s"
#define SV_Arg(s) (int)(s).count, (s).data

SVDEF char* sv_to_cstr(String_View sv);
SVDEF String_View sv_from_parts(const char *data, size_t count);
SVDEF String_View sv_from_cstr(const char *cstr);
SVDEF String_View sv_trim_left(String_View sv);
SVDEF String_View sv_trim_right(String_View sv);
SVDEF String_View sv_trim(String_View sv);
SVDEF String_View sv_chop_left(String_View *sv, size_t n);
SVDEF String_View sv_chop_right(String_View *sv, size_t n);
SVDEF bool sv_try_chop_by_delim(String_View *sv, char delim, String_View *chunk);
SVDEF String_View sv_chop_by_delim(String_View *sv, char delim);
SVDEF bool sv_eq(String_View a, String_View b);

#endif // SV_H_

#ifdef SV_IMPLEMENTATION


SVDEF char* sv_to_cstr(String_View sv) {
    if (!sv.data && sv.count > 0)
        return NULL;
    char *cstr = malloc(sv.count + 1);
    if (!cstr) return NULL;
    if (sv.count > 0)
        memcpy(cstr, sv.data, sv.count);

    cstr[sv.count] = '\0';
    return cstr;
}

SVDEF String_View sv_from_parts(const char *data, size_t count) {
    return (String_View) {
        .data = data,
        .count = count
    };
}

SVDEF String_View sv_from_cstr(const char *cstr) {
    return sv_from_parts(cstr, strlen(cstr));
}

SVDEF String_View sv_chop_left(String_View *sv, size_t n) {
    if (n > sv->count)
        n = sv->count;
    String_View result = sv_from_parts(sv->data, n);
    sv->data += n;
    sv->count -= n;
    return result;
}

SVDEF String_View sv_chop_right(String_View *sv, size_t n) {
    if (n > sv->count)
        n = sv->count;
    String_View result = sv_from_parts(sv->data + sv->count - n, n);
    sv->count -= n;
    return result;
}

SVDEF String_View sv_trim_left(String_View sv) {
    size_t i = 0;
    while (i < sv.count && isspace(sv.data[i])) {
        i++;
    }
    return sv_from_parts(sv.data + i, sv.count - i);
}

SVDEF String_View sv_trim_right(String_View sv) {
    size_t i = 0;
    while (i < sv.count && isspace(sv.data[sv.count -1 - i])) {
        i++;
    }
    return sv_from_parts(sv.data, sv.count - i);
}

SVDEF String_View sv_trim(String_View sv) {
    return sv_trim_right(sv_trim_left(sv));
}

SVDEF bool sv_try_chop_by_delim(String_View *sv, char delim, String_View *chunk) {
    size_t i = 0;
    while (i < sv->count && sv->data[i] != delim)
        i++;
    String_View result = sv_from_parts(sv->data, i);
    if (i < sv->count) {
        sv->data += i + 1;
        sv->count -= i + 1;
        if (chunk) 
            *chunk = result;
        return true;
    }
    return false;
}

SVDEF String_View sv_chop_by_delim(String_View *sv, char delim) {
    size_t i = 0;
    while (i < sv->count && sv->data[i] != delim)
        i++;
    String_View result = sv_from_parts(sv->data, i);
    if (i < sv->count) {
        sv->data += i + 1;
        sv->count -= i + 1;
    } else {
        sv->data += i;
        sv->count -= i;
    }
    return result;
}

SVDEF String_View sv_chop_by_type(String_View *sv, int (*istype)(int c)) {
    size_t i = 0;
    while (i < sv->count && istype(sv->data[i]))
        i++;
    String_View result = sv_from_parts(sv->data, i);
    if (i < sv->count) {
        sv->data += i + 1;
        sv->count -= i + 1;
    } else {
        sv->data += i;
        sv->count -= i;
    }
    return result;
}

SVDEF bool sv_eq(String_View a, String_View b) {
    if (a.count != b.count) 
        return false;
    else 
        return memcmp(a.data, b.data, a.count) == 0;
}
#endif // SV_IMPLEMENTATION

