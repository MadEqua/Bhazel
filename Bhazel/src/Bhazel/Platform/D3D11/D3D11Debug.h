#pragma once

#include <dxgidebug.h>


#ifndef BZ_DIST

namespace BZ {

    class DXGIDebug {
        BZ_GENERATE_SINGLETON(DXGIDebug)
    public:
        void printMessages();
    private:
        uint64 next = 0u;
        IDXGIInfoQueue *dxgiInfoQueue;
    };
}

#define BZ_ASSERT_HRES(call) { \
    HRESULT hr; \
    if(FAILED(hr = (call))) \
        BZ_ASSERT_ALWAYS_CORE("HRESULT is not OK! Error: 0x{0:08x}.", static_cast<uint32>(hr)) \
}

#define BZ_LOG_HRES(call) { \
    HRESULT hr; \
    if(FAILED(hr = (call))) \
        BZ_LOG_CORE_ERROR("HRESULT is not OK. Error: 0x{0:08x}. File: {1}. Line: {2}.", static_cast<uint32>(hr), __FILE__, __LINE__); \
}

#define BZ_LOG_DXGI(call) (call); BZ::DXGIDebug::get().printMessages()

#define BZ_ASSERT_HRES_DXGI(call) { \
    HRESULT hr = (call); \
    BZ::DXGIDebug::get().printMessages(); \
    if(FAILED(hr)) BZ_ASSERT_ALWAYS_CORE("HRESULT is not OK! Error: 0x{0:08x}.", static_cast<uint32>(hr)) \
}

#define BZ_LOG_HRES_DXGI(call) { \
    HRESULT hr = (call); \
    BZ::DXGIDebug::get().printMessages(); \
    if(FAILED(hr)) BZ_LOG_CORE_ERROR("HRESULT is not OK! Error: 0x{0:08}. File: {1}. Line: {2}.", static_cast<uint32>(hr), __FILE__, __LINE__); \
}

#else
#define BZ_ASSERT_HRES(call) call
#define BZ_LOG_HRES(call) call

#define BZ_LOG_DXGI(call) call

#define BZ_ASSERT_HRES_DXGI(call) call
#define BZ_LOG_HRES_DXGI(call) call
#endif