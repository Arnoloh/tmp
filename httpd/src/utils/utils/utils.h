#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <time.h>

#include "my_atoi_base.h"
#include "utils/string/string.h"

#define BUFFER_SIZE 4096

void put_time(struct string *anws, int endofline);
struct string *UrlDecoding(struct string *url);

enum TYPE_ERROR
{
    FB = 403,
    NF = 404,
    MNA = 405,
    BR = 400,
    VNS = 505,
    OK = 200,

};

#define S_GET "GET"
#define S_HEAD "HEAD"

enum Method
{
    GET,
    HEAD,
    UNSUPPORTED = 127,
};

struct header
{
    enum Method method;
    int good_version;
    struct string *target;
    struct string *host;
    int content_length;
};

#endif /* UTILS_H */
