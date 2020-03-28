#pragma once

constexpr static uint32 MAX_FRAMES_IN_FLIGHT = 3;
constexpr static uint32 MAX_FRAMEBUFFER_ATTACHEMENTS = 8;

constexpr static uint32 BHAZEL_FRAME_DESCRIPTOR_SET_IDX = 0;
constexpr static uint32 BHAZEL_SCENE_DESCRIPTOR_SET_IDX = 1;
constexpr static uint32 BHAZEL_OBJECT_DESCRIPTOR_SET_IDX = 2;
constexpr static uint32 APP_FIRST_DESCRIPTOR_SET_IDX = 3;

constexpr static uint32 MAX_VIEWPORTS = 16;

constexpr static uint32 MAX_COMMAND_BUFFERS = 8;
constexpr static uint32 MAX_COMMANDS_PER_BUFFER = 1024 * 4;
constexpr static uint32 MAX_DESCRIPTOR_DYNAMIC_OFFSETS = 8;

constexpr static uint32 MAX_SCENES_PER_FRAME = 8;
constexpr static uint32 MAX_OBJECTS_PER_FRAME = 64;
constexpr static uint32 MAX_DIR_LIGHTS_PER_SCENE = 2;

constexpr static uint32 MAX_PUSH_CONSTANT_SIZE = 128;