#pragma once

#include <iostream>
#include <memory>
#include <algorithm>
#include <functional>

#include <sstream>
#include <string>

#ifdef BZ_PLATFORM_WINDOWS
    #include <Windows.h>
    #include <Windowsx.h>
#endif

#include "Bhazel/Core/Types.h"
#include "Bhazel/Core/Log.h"
#include "Bhazel/Core/Core.h"