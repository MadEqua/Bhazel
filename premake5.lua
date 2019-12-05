workspace "Bhazel"
    architecture "x64"
    startproject "Sandbox"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

outputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

VULKAN_SDK_DIR = "C:/VulkanSDK/1.1.121.2"

include "Bhazel"
include "Sandbox"
include "BrickBreaker"
include "Bhazel/vendor/glad"
include "Bhazel/vendor/glfw_premake5.lua"
include "Bhazel/vendor/imgui_premake5.lua"
include "Bhazel/vendor/stb_image"