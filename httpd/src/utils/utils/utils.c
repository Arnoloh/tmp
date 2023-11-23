#include "utils.h"

void put_time(struct string *anws, int endofline)
{
    time_t temp = time(NULL);
    struct tm *time = gmtime(&temp);
    char date[BUFFER_SIZE];
    if (endofline)
        strftime(date, sizeof(date), "Date: %a, %d %b %Y %X %Z\r\n", time);
    else
        strftime(date, sizeof(date), "%a, %d %b %Y %X %Z", time);
    string_concat_str(anws, date, strlen(date));
}

struct string *UrlDecoding(struct string *url)
{
    struct string *result = string_create("", 0);
    for (size_t i = 0; i < url->size; i++)
    {
        if (url->data[i] == '%')
        {
            char h[] = { url->data[i + 1], url->data[i + 2], '\0' };
            int dec = my_atoi_base(h, "0123456789ABCDEF");
            char hexa[] = { dec };
            string_concat_str(result, hexa, 1);
            i += 2;
        }
        else if (url->data[i] == '?')
            break;
        else
        {
            char hexa[] = { url->data[i] };
            string_concat_str(result, hexa, 1);
        }
    }
    string_destroy(url);
    return result;
}
