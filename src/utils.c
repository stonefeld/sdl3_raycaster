#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

void print_error(const unsigned int line, const char *file, const char *error) {
    fprintf(stderr, "[%s:%d] Error: %s\n", file, line, error);
    exit(EXIT_FAILURE);
}