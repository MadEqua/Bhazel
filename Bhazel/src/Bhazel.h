#pragma once

// Convenience header for use by Bhazel client applications.

#include "bzpch.h"

#include "Core/Application.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"
#include "Core/Timer.h"

#include "Layers/Layer.h"

#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Events/WindowEvent.h"

#include "Graphics/Buffer.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/DescriptorSet.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/PipelineState.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"

#include "Entities/CameraController.h"
#include "Renderer/Camera.h"
#include "Renderer/Material.h"
#include "Renderer/Mesh.h"
#include "Renderer/ParticleSystem2D.h"
#include "Renderer/Renderer.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/Scene.h"

#include "Collisions/AABB.h"
#include "Collisions/BoundingSphere.h"
#include "Collisions/CollisionUtils.h"