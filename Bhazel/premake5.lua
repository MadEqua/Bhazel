project "Bhazel"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"

    targetdir("../bin/" .. outputDir .. "/%{prj.name}")
    objdir("../bin-int/" .. outputDir .. "/%{prj.name}")

    pchheader "bzpch.h"
    pchsource "src/bzpch.cpp"

    files
    {
        "src/**.h",
        "src/**.cpp",
    }

    includedirs
    {
        "src",
        "vendor/spdlog/include",
        "vendor/GLFW/include",
        "vendor/glad/include",
        "vendor/ImGui",
        "vendor/glm",
        "vendor/stb_image",
        "vendor/VulkanMemoryAllocator",
        "%{VULKAN_SDK_DIR}/Include"
    }

    libdirs
    {
        "%{VULKAN_SDK_DIR}/Lib"
    }

    links
    {
        "GLFW",
        "glad",
        "ImGui",
        "stb_image",
        "vulkan-1.lib",
        "opengl32.lib",
    }

    defines
    {
        "GLFW_INCLUDE_NONE",
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "BZ_PLATFORM_WINDOWS",
        }

        links
        {
            "d3d11.lib",
            "dxguid.lib",
            "D3DCompiler.lib",
        }

    filter "configurations:Debug"
        defines "BZ_DEBUG"
        runtime "Debug"
        symbols "on"
        optimize "off"

    filter "configurations:Release"
        defines "BZ_RELEASE"
        runtime "Release"
        symbols "on"
        optimize "on"

    filter "configurations:Dist"
        defines "BZ_DIST"
        runtime "Release"
        symbols "off"
        optimize "on"