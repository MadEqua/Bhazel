#pragma once


namespace BZ {

    enum class RenderQueueFamily {
        Graphics,
        Compute,
        Transfer,
        Present,

        Count
    };


    class CommandBuffer {
    public:
    };
}
