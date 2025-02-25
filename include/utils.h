#ifndef UTILS_H
#define UTILS_H

#define PRINT_ERROR(error) print_error(__LINE__, __FILE__, error)
#define WRAP_SDL_ERROR(error) \
    if (error < 0)            \
    PRINT_ERROR(SDL_GetError())

void print_error(const unsigned int line, const char *file, const char *error);

#endif