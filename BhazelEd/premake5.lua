project "BhazelEd"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"

    targetdir "../bin/%{OUTPUT_DIR}/%{prj.name}"
    objdir "../bin-int/%{OUTPUT_DIR}/%{prj.name}"
    debugdir "../bin/%{OUTPUT_DIR}/%{prj.name}"

    files {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs {
        "src",
        "../Bhazel/src",
        "../Bhazel/vendor/spdlog/include",
        "../Bhazel/vendor/glm",
        "../Bhazel/vendor/ImGui",
        "../Bhazel/vendor/VulkanMemoryAllocator",
        "%{VULKAN_SDK_DIR}/Include",
    }

    links {
        "Bhazel"
    }