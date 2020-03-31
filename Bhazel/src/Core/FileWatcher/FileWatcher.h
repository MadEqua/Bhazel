#pragma once


namespace filewatch {
    template<class T>
    class FileWatch;
}

namespace BZ {
    class Shader;
    class PipelineState;

    constexpr float TIME_TO_DEBOUNCE_CALLBACK_S = 0.1f;

    class FileWatcher {
    public:
        ~FileWatcher();

        void startWatching();

        void registerPipelineState(PipelineState &pipelineState);

        bool hasPipelineStatesToReload() const { return !pipelinesToReload.empty(); }
        void performReloads();

    private:
        filewatch::FileWatch<std::wstring> *fileWatcher;

        std::unordered_map<std::string, PipelineState*> pipelineStateRegistry;
        Timer timer;

        std::set<PipelineState*> pipelinesToReload;
    };
}