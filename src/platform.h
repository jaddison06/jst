#pragma once

#include <stdbool.h>

# if defined(_WIN32)
#   define JST_WIN
# elif defined(__MACH__)
#   define JST_MAC
#   define JST_UNIX
# elif defined(__linux__)
#   define JST_LINUX
#   define JST_UNIX
# endif