#include "bzpch.h"

#include "D3D11Debug.h"
#include <D3D11SDKLayers.h>

#ifndef BZ_DIST

namespace BZ {

    DXGIDebug::DXGIDebug() :
        dxgiInfoQueue (nullptr) {

        // define function signature of DXGIGetDebugInterface
        typedef HRESULT(WINAPI* DXGIGetDebugInterface)(REFIID, void **);

        // load the dll that contains the function DXGIGetDebugInterface
        const auto hModDxgiDebug = LoadLibraryEx(L"dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        if(!hModDxgiDebug) {
            BZ_LOG_ERROR("Failed to load dxgidebug.dll. DXGIDebug will not function.");
            return;
        }

        // get address of DXGIGetDebugInterface in dll
        const auto dxgiGetDebugInterface = reinterpret_cast<DXGIGetDebugInterface>(reinterpret_cast<void*>(GetProcAddress(hModDxgiDebug, "DXGIGetDebugInterface")));
        if(!dxgiGetDebugInterface) {
            BZ_LOG_ERROR("Failed to load DXGIGetDebugInterface in dxgidebug.dll. DXGIDebug will not function.");
            return;
        }

        BZ_LOG_HRES(dxgiGetDebugInterface(__uuidof(IDXGIInfoQueue), reinterpret_cast<void**>(&dxgiInfoQueue)));
    }

    DXGIDebug::~DXGIDebug() {}

    void DXGIDebug::printMessages() {
        if(!dxgiInfoQueue) return;

        const auto end = dxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
        for(auto i = next; i < end; i++) {
            SIZE_T messageLength;

            BZ_LOG_HRES(dxgiInfoQueue->GetMessage(DXGI_DEBUG_ALL, i, nullptr, &messageLength));
            
            auto bytes = std::make_unique<byte[]>(messageLength);
            auto message = reinterpret_cast<DXGI_INFO_QUEUE_MESSAGE*>(bytes.get());
            
            BZ_LOG_HRES(dxgiInfoQueue->GetMessage(DXGI_DEBUG_ALL, i, message, &messageLength));
            
            const char *category = "Unknown";
            switch(message->Category) {
            case DXGI_INFO_QUEUE_MESSAGE_CATEGORY_MISCELLANEOUS:
                category = "Miscellaneous"; break;
            case DXGI_INFO_QUEUE_MESSAGE_CATEGORY_UNKNOWN:
                category = "Unknown"; break;
            case DXGI_INFO_QUEUE_MESSAGE_CATEGORY_INITIALIZATION:
                category = "Initialization"; break;
            case DXGI_INFO_QUEUE_MESSAGE_CATEGORY_CLEANUP:
                category = "Cleanup"; break;
            case DXGI_INFO_QUEUE_MESSAGE_CATEGORY_COMPILATION:
                category = "Compilation"; break;
            case DXGI_INFO_QUEUE_MESSAGE_CATEGORY_STATE_CREATION:
                category = "State Creation"; break;
            case DXGI_INFO_QUEUE_MESSAGE_CATEGORY_STATE_SETTING:
                category = "State Setting"; break;
            case DXGI_INFO_QUEUE_MESSAGE_CATEGORY_STATE_GETTING:
                category = "State Getting"; break;
            case DXGI_INFO_QUEUE_MESSAGE_CATEGORY_RESOURCE_MANIPULATION:
                category = "Resource Manipulation"; break;
            case DXGI_INFO_QUEUE_MESSAGE_CATEGORY_EXECUTION:
                category = "Execution"; break;
            case DXGI_INFO_QUEUE_MESSAGE_CATEGORY_SHADER:
                category = "Shader"; break;
            }

            const char *producer = "Unknown";
            if(IsEqualGUID(message->Producer, DXGI_DEBUG_ALL)) {
                producer = "All";
            }
            else if(IsEqualGUID(message->Producer, DXGI_DEBUG_DX)) {
                producer = "DX";
            }
            else if(IsEqualGUID(message->Producer, DXGI_DEBUG_DXGI)) {
                producer = "DXGI";
            }
            else if(IsEqualGUID(message->Producer, DXGI_DEBUG_APP)) {
                producer = "App";
            }
            else if(IsEqualGUID(message->Producer, DXGI_DEBUG_D3D11)) {
                producer = "D3D11";
            }

            switch(message->Severity) {
            case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION:
                BZ_LOG_CORE_ERROR("D3D11 Debug - Id: {0}. Producer: {1}. Category: {2}. Severity: Corruption. Description: {3}", message->ID, producer, category, message->pDescription);
                break;
            case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR:
                BZ_LOG_CORE_ERROR("D3D11 Debug - Id: {0}. Producer: {1}. Category: {2}. Severity: Error. Description: {3}", message->ID, producer, category, message->pDescription);
                break;
            case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING:
                BZ_LOG_CORE_WARN("D3D11 Debug - Id: {0}. Producer: {1}. Category: {2}. Severity: Warning. Description: {3}", message->ID, producer, category, message->pDescription);
                break;
            case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_INFO:
                BZ_LOG_CORE_INFO("D3D11 Debug - Id: {0}. Producer: {1}. Category: {2}. Severity: Info. Description: {3}", message->ID, producer, category, message->pDescription);
                break;
            case DXGI_INFO_QUEUE_MESSAGE_SEVERITY_MESSAGE:
                BZ_LOG_CORE_INFO("D3D11 Debug - Id: {0}. Producer: {1}. Category: {2}. Severity: Message. Description: {3}", message->ID, producer, category, message->pDescription);
                break;
            default:
                BZ_LOG_CORE_INFO("D3D11 Debug - Id: {0}. Producer: {1}. Category: {2}. Severity: Unknown. Description: {3}", message->ID, producer, category, message->pDescription);
            }
        }

        // set the index (next) so that the next call to printMessages()
        // will only get errors generated after this call
        next = dxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
    }
}
#endif