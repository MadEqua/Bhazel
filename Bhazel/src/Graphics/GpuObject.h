#pragma once

#define BZ_GRAPHICS_DEVICE BZ::Application::get().getGraphicsContext().getDevice()
#define BZ_MEM_ALLOCATOR BZ::Application::get().getGraphicsContext().getMemoryAllocator()


namespace BZ {

template <typename HandleT> class GpuObject {
  public:
    HandleT getHandle() const { return handle; }

  protected:
    HandleT handle;
};
}