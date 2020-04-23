%VULKAN_SDK%/Bin32/glslc.exe ./Renderer2DVert.glsl -o ..\bin\Renderer2DVert.spv
%VULKAN_SDK%/Bin32/glslc.exe ./Renderer2DFrag.glsl -o ..\bin\Renderer2DFrag.spv

%VULKAN_SDK%/Bin32/glslc.exe ./ImGuiVert.glsl -o ..\bin\ImGuiVert.spv
%VULKAN_SDK%/Bin32/glslc.exe ./ImGuiFrag.glsl -o ..\bin\ImGuiFrag.spv

%VULKAN_SDK%/Bin32/glslc.exe ./RendererVert.glsl -o ..\bin\RendererVert.spv
%VULKAN_SDK%/Bin32/glslc.exe ./RendererFrag.glsl -o ..\bin\RendererFrag.spv

%VULKAN_SDK%/Bin32/glslc.exe ./SkyBoxVert.glsl -o ..\bin\SkyBoxVert.spv
%VULKAN_SDK%/Bin32/glslc.exe ./SkyBoxFrag.glsl -o ..\bin\SkyBoxFrag.spv

%VULKAN_SDK%/Bin32/glslc.exe ./DepthPassVert.glsl -o ..\bin\DepthPassVert.spv

pause