#pragma once

#define BZ_GRAPHICS_DEVICE BZ::Engine::get().getGraphicsContext().getDevice()
#define BZ_MEM_ALLOCATOR BZ::Engine::get().getGraphicsContext().getMemoryAllocator()


namespace BZ {

template <typename HandleT> class GpuObject {
  public:
    HandleT getHandle() const { return handle; }

  protected:
    HandleT handle;
};
}