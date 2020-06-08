#include <bzpch.h>

#include <FileWatch.hpp>

#include "Core/Application.h"
#include "FileWatcher.h"
#include "Graphics/PipelineState.h"
#include "Graphics/Shader.h"

#include <codecvt>


namespace BZ {

constexpr uint32 TIME_TO_DEBOUNCE_CALLBACK_MS = 100;

FileWatcher::~FileWatcher() {
    delete fileWatcher;
}

void FileWatcher::startWatching() {

    const auto &watchFolder = Application::get().getAssetsPath();

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wideWatchFolder = converter.from_bytes(watchFolder);

    // Filter bursts of callbacks. Happens a lot on Windows.
    timer.start();

    // This lambda will be called on a worker thread from filewatch.
    fileWatcher = new filewatch::FileWatch<std::wstring>(
        wideWatchFolder, [this](const std::wstring &path, const filewatch::Event change_type) {
            uint32 timeSinceLast = timer.getCountedTime().asMillisecondsUint32();
            if (timeSinceLast > TIME_TO_DEBOUNCE_CALLBACK_MS && change_type == filewatch::Event::modified) {

                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                std::string pathString = converter.to_bytes(path);
                std::replace(pathString.begin(), pathString.end(), '\\', '/');

                for (auto &pair : pipelineStateRegistry) {
                    if (pathString.find(pair.first) != std::string::npos &&
                        pipelinesToReload.find(pair.second) == pipelinesToReload.end()) {
                        BZ_LOG_CORE_INFO("FileWatcher detected shader change: {}. Scheduling reload.", pathString);
                        pipelinesToReload.insert(pair.second);
                    }
                }
            }

            timer.start();
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