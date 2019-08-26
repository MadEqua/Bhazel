#pragma once

#include <dxgidebug.h>


namespace BZ {

    class DXGIDebug
    {
        BZ_GENERATE_SINGLETON(DXGIDebug)
    public:
        void printMessages();
    private:
        uint64 next = 0u;
        IDXGIInfoQueue *dxgiInfoQueue;
    };
}

#ifndef BZ_DIST

#define BZ_ASSERT_HRES(hrcall) { \
    HRESULT hr; \
    if(FAILED(hr = (hrcall))) \
        BZ_ASSERT_ALWAYS_CORE("HRESULT is not OK! Error: 0x{0:08x}", static_cast<uint32>(hr)) \
}

#define BZ_LOG_HRES(hrcall) { \
    HRESULT hr; \
    if(FAILED(hr = (hrcall))) \
        BZ_LOG_CORE_ERROR("HRESULT is not OK! Error: 0x{0:08x}. File: {1}. Line: {2}.", static_cast<uint32>(hr), __FILE__, __LINE__); \
}

#define BZ_ASSERT_HRES_DXGI(hrcall) { \
    HRESULT hr; \
    if(FAILED(hr = (hrcall))) { \
        BZ::DXGIDebug::getInstance().printMessages(); \
        BZ_ASSERT_ALWAYS_CORE("HRESULT is not OK! Error: 0x{0:08x}", static_cast<uint32>(hr)) \
    } \
}

#define BZ_LOG_HRES_DXGI(hrcall) { \
    HRESULT hr; \
    if(FAILED(hr = (hrcall))) { \
        BZ::DXGIDebug::getInstance().printMessages(); \
        BZ_LOG_CORE_ERROR("HRESULT is not OK! Error: 0x{0:08}. File: {1}. Line: {2}.", static_cast<uint32>(hr), __FILE__, __LINE__); \
    } \
}

#else
#define ASSERT_HRES(hrcall)
#define LOG_HRES(hrcall)

#define ASSERT_HRES_DXGI(hrcall)
#define LOG_HRES_DXGI(hrcall)
#endif