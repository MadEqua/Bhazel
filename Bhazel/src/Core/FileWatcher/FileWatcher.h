#pragma once


namespace filewatch {
template <class T> class FileWatch;
}

namespace BZ {

class Shader;
class PipelineState;

class FileWatcher {
  public:
    FileWatcher() = default;
    ~FileWatcher();

    BZ_NON_COPYABLE(FileWatcher);

    void startWatching();

    void registerPipelineState(PipelineState &pipelineState);

    bool hasPipelineStatesToReload() const { return !pipelinesToReload.empty(); }
    void performReloads();

  private:
    filewatch::FileWatch<std::wstring> *fileWatcher;

    std::unordered_map<std::string, PipelineState *> pipelineStateRegistry;
    Timer timer;

    std::set<PipelineState *> pipelinesToReload;
};
}