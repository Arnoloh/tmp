#include "my_atoi_base.h"

int get_index(const char *str, char c)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == c)
            return i;
    }
    return -1;
}
int power(int n, int power)
{
    int a = n;
    if (power == 0)
        return 1;
    while (power > 1)
    {
        a *= n;
        power--;
    }
    return a;
}
int my_atoi_base(const char *str, const char *base)
{
    if (str == NULL || *str == '\0')
        return 0;
    int i = 0;
    int len_base = strlen(base);
    int len_str = strlen(str);
    int neg = 2;
    int number = 0;
    if (str[i] == '-')
    {
        neg = -1;
        i++;
    }
    while (str[i] != '\0')
    {
        if (str[i] == ' ' && number == 0 && neg == 2)
        {
            i++;
            continue;
        }
        if (str[i] == '+' && number == 0 && neg == 2)
        {
            neg = 1;
            i++;
            continue;
        }
        if (str[i] == '-' && neg == 2)
        {
            neg = -1;
            i++;
            continue;
        }
        int po = get_index(base, str[i]);
        if (po == -1 || ((str[i] == '+' || str[i] == '-') && neg != 2))
            return 0;
        int p = power(len_base, len_str - i - 1);
        number += po * p;
        i++;
    }
    return neg == -1 ? -number : number;
}
