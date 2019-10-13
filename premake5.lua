workspace "Bhazel"
    architecture "x64"
    startproject "Sandbox"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

    platforms 
    {
        "Vulkan",
        "OpenGL43",
        "D3D11"
    }

    outputDir = "%{cfg.system}-%{cfg.architecture}-%{cfg.platform}-%{cfg.buildcfg}"

    VULKAN_SDK_DIR = "C:/VulkanSDK/1.1.121.2"

    include "Bhazel"
    include "Sandbox"
    include "Bhazel/vendor/glad"
    include "Bhazel/vendor/glfw_premake5.lua"
    include "Bhazel/vendor/imgui_premake5.lua"
    include "Bhazel/vendor/stb_image"