#pragma once

#ifdef WIN32
#include "endian_win.h"
#elif defined __GNUC__
#include "endian_gnu.h"
#endif

