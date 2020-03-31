#include <bzpch.h>

#include <FileWatch.hpp>

#include "FileWatcher.h"
#include "Core/Application.h"
#include "Graphics/Shader.h"
#include "Graphics/PipelineState.h"

#include <codecvt>


namespace BZ {

    FileWatcher::~FileWatcher() {
        delete fileWatcher;
    }

    void FileWatcher::startWatching() {

        const auto &watchFolder = Application::getInstance().getAssetsPath();

        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring wideWatchFolder = converter.from_bytes(watchFolder);

        //Filter bursts of callbacks. Happens a lot on Windows.
        timer.start();

        //This lambda will be called on a worker thread from filewatch
        fileWatcher = new filewatch::FileWatch<std::wstring>(wideWatchFolder,
            [this](const std::wstring &path, const filewatch::Event change_type) {
                float timeSinceLast = timer.getCountedTime().asSeconds();
                if (timeSinceLast > TIME_TO_DEBOUNCE_CALLBACK_S && change_type == filewatch::Event::modified) {
                    timer.start();

                    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                    std::string pathString = converter.to_bytes(path);
                    std::replace(pathString.begin(), pathString.end(), '\\', '/');

                    for (auto &pair : pipelineStateRegistry) {
                        if (pathString.find(pair.first) != std::wstring::npos) {
                            //BZ_LOG_CORE_INFO("FileWatcher detected dhader change: {}.", path);
                            pipelinesToReload.insert(pair.second);
                        }
                    }
                }
            });

        BZ_LOG_CORE_INFO("FileWatcher is watching {}.", watchFolder);
    }

    void FileWatcher::registerPipelineState(PipelineState &pipelineState) {
        const Ref<Shader> &shader = pipelineState.getData().shader;
        for (const auto &path : shader->getAllFilePaths()) {
            pipelineStateRegistry[path] = &pipelineState;
        }
    }

    void FileWatcher::performReloads() {
        for (auto pipelineState : pipelinesToReload) {
            pipelineState->reload();
        }
        pipelinesToReload.clear();
    }
}