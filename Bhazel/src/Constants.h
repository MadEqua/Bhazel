#pragma once

constexpr static uint32 MAX_FRAMES_IN_FLIGHT = 3;
constexpr static uint32 MAX_FRAMEBUFFER_ATTACHEMENTS = 8;

constexpr static uint32 RENDERER_GLOBAL_DESCRIPTOR_SET_IDX = 0;
constexpr static uint32 RENDERER_PASS_DESCRIPTOR_SET_IDX = 1;
constexpr static uint32 RENDERER_SCENE_DESCRIPTOR_SET_IDX = 2;
constexpr static uint32 RENDERER_ENTITY_DESCRIPTOR_SET_IDX = 3;
constexpr static uint32 RENDERER_MATERIAL_DESCRIPTOR_SET_IDX = 4;
constexpr static uint32 APP_FIRST_DESCRIPTOR_SET_IDX = 5;

constexpr static uint32 SHADOW_MAPPING_CASCADE_COUNT = 4;

constexpr static uint32 MAX_VIEWPORTS = 16;

constexpr static uint32 MAX_COMMAND_BUFFERS = 8;
constexpr static uint32 MAX_COMMANDS_PER_BUFFER = 1024 * 4;
constexpr static uint32 MAX_DESCRIPTOR_DYNAMIC_OFFSETS = 4;

constexpr static uint32 MAX_DIR_LIGHTS_PER_SCENE = 2;
constexpr static uint32 MAX_PASSES_PER_FRAME = MAX_DIR_LIGHTS_PER_SCENE * SHADOW_MAPPING_CASCADE_COUNT + 1; //Depth Passes + Color Pass
constexpr static uint32 MAX_ENTITIES_PER_SCENE = 64;
constexpr static uint32 MAX_MATERIALS_PER_SCENE = 64;

constexpr static uint32 MAX_PUSH_CONSTANT_SIZE = 128;
constexpr static uint32 MIN_UNIFORM_BUFFER_OFFSET_ALIGN = 256;