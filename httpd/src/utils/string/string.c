#define _POSIX_C_SOURCE 200809L

#include "string.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct string *string_create(const char *str, size_t size)
{
    struct string *string = calloc(1, sizeof(struct string));
    string->data = calloc(size + 1, sizeof(char));

    for (size_t i = 0; i < size; i++)
        string->data[i] = str[i];

    string->size = size;
    string->data[size] = '\0';
    return string;
}
int string_compare_n_str(const struct string *str1, const char *str2, size_t n)
{
    if (strlen(str2) != n)
        return 1;
    for (size_t i = 0; i < n; i++)
    {
        char c1 = tolower(str1->data[i]);
        char c2 = tolower(str2[i]);

        if (c1 == c2)
            continue;
        if (c1 > c2)
            return 1;
        if (c1 < c2)
            return -1;
    }
    return 0;
}

int string_strstr(const struct string *str1, const char *str2)
{
    size_t len = strlen(str2);
    if (str1->size < len)
        return 0;

    for (size_t i = 0; i < str1->size; i++)
    {
        size_t j = 0;
        char c1 = tolower(str1->data[i]);
        char c2 = tolower(str2[j]);
        if (c1 == c2)
        {
            for (; j < strlen(str2) && i < str1->size; j++)
            {
                c1 = tolower(str1->data[i]);
                c2 = tolower(str2[j]);

                if (c1 == c2)
                {
                    i++;
                    continue;
                }
                break;
            }
            if (j == strlen(str2))
                return i;
        }
    }
    return 0;
}

int string_compare_n_start_str(const struct string *str1, const char *str2,
                               size_t n, size_t start)
{
    if (strlen(str2) != n)
        return 1;
    for (size_t i = start; i < n + start; i++)
    {
        char c1 = tolower(str1->data[i]);
        char c2 = tolower(str2[i - start]);

        if (c1 == c2)
            continue;
        if (c1 > c2)
            return 1;
        if (c1 < c2)
            return -1;
    }
    return 0;
}

void print_string(struct string *str)
{
    for (size_t i = 0; i < str->size; i++)
        putchar(str->data[i]);
    fflush(stdout);
}

int string_strcspn(struct string *str, const char *pattern, size_t start)
{
    size_t len = strlen(pattern);
    for (size_t i = start; i < str->size; i++)
    {
        for (size_t j = 0; j < len; j++)
            if (str->data[i] == pattern[j])
                return i - start;
    }
    return -1;
}

struct string *string_concat_str(struct string *str, const char *to_concat,
                                 size_t size)
{
    if (str->data == NULL && to_concat == NULL)
        return str;

    str->data = realloc(str->data, str->size + size + 1);
    for (size_t i = 0; i < size; i++)
        str->data[i + str->size] = to_concat[i];

    str->size += size;
    str->data[str->size] = '\0';
    return str;
}

void string_destroy(struct string *str)
{
    if (str == NULL)
        return;
    free(str->data);
    free(str);
}
