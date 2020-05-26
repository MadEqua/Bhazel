#include "bzpch.h"

#include "QueryPool.h"

#include "Graphics/Internal/Device.h"


namespace BZ {

    void QueryPool::init(const Device &device, VkQueryType queryType, uint32 queryCount, VkQueryPipelineStatisticFlags pipelineStatistics) {
        this->device = &device;

        VkQueryPoolCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
        createInfo.queryType = queryType;
        createInfo.queryCount = queryCount;
        createInfo.pipelineStatistics = pipelineStatistics;

        BZ_ASSERT_VK(vkCreateQueryPool(device.getHandle(), &createInfo, nullptr, &handle));
    }

    void QueryPool::destroy() {
        vkDestroyQueryPool(device->getHandle(), handle, nullptr);
    }

    void QueryPool::getResults(uint32 firstQuery, uint32 queryCount, uint32 dataSize, void *data, uint32 stride, VkQueryResultFlags flags) {
        VkResult res = vkGetQueryPoolResults(device->getHandle(), handle, firstQuery, queryCount, dataSize, data, stride, flags);
        if(res == VK_NOT_READY) {
            BZ_LOG_CORE_INFO("QueryPool is being asked for results, but results are not ready.");
        }
        else if(res < 0) {
            BZ_ASSERT_ALWAYS_CORE("VkResult is not Success! Error: {}.", static_cast<int>(res));
        }
    }
};