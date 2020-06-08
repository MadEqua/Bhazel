#pragma once

#include "Graphics/Internal/VulkanIncludes.h"


namespace BZ {

class Device;

class QueryPool {
  public:
    void init(const Device &device, VkQueryType queryType, uint32 queryCount,
              VkQueryPipelineStatisticFlags pipelineStatistics);
    void destroy();

    void getResults(uint32 firstQuery, uint32 queryCount, uint32 dataSize, void *data, uint32 stride,
                    VkQueryResultFlags flags);

    VkQueryPool getHandle() const { return handle; }

  private:
    const Device *device;

    VkQueryPool handle;
};
}
