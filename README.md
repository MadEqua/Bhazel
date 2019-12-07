# Bhazel

Work in progress game engine written in C++.

Current features:
-  Central event dispatching system for keyboard, mouse and window events.
-  Input polling facilities.
-  Layer stack system allowing the application to receive events within a certain order. Layers also have a specified update and render order.
-  Support for multiple rendering APIs (OpenGL, Direct3D 11 and Vulkan) and platforms (GLFW and Win32).
-  Thin abstraction of the rendering API objects and command buffer generation.
-  2D Renderer with batching written on top of the rendering API abstraction.
-  Camera abstractions and simple controllers.
-  ImGui integration through the engine APIs (both rendering and input).
-  Example Sandbox application for experimentation as a client application.
-  Example Brick Breaker game as a demo and test case for 2D rendering.
-  Premake build system.

Inspired by [TheCherno's Hazel Engine](https://github.com/TheCherno/Hazel) architecture.

# Gallery

BrickBreaker game showcasing particle systems and 2D batching. Also ImGui showing useful rendering statistics.

![BrickBreaker](https://github.com/MadEqua/Bhazel/blob/master/media/BrickBreaker.gif)


2D Particle system test with over 100k particles from 2 emitters.

![ParticleSystem](https://github.com/MadEqua/Bhazel/blob/master/media/ParticleSystem2D.gif)