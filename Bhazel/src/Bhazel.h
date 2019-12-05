#pragma once

//Convenience header for use by Bhazel client applications

#include "bzpch.h"

#include "Core/Application.h"
#include "Core/Timer.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"

#include "Layers/Layer.h"

#include "Events/WindowEvent.h"
#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

#include "Graphics/Graphics.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/PipelineState.h"
#include "Graphics/Buffer.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
#include "Graphics/DescriptorSet.h"

#include "Renderer/Renderer2D.h"
#include "Renderer/Camera.h"
#include "Renderer/ParticleSystem.h"
#include "Entities/CameraController.h"

#include "Collisions/AABB.h"
#include "Collisions/BoundingSphere.h"
#include "Collisions/CollisionUtils.h"