unsigned int kstrlen(const char *const s)
{
    unsigned int result = 0;

    while (s[result]) {
        ++result;
    }

    return result;
}