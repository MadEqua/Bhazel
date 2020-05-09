# Bhazel

Work in progress game engine written in C++.

# Current features:
-  Built on top of Vulkan.
-  Physically-based renderer with image-based indirect lighting.
-  Cascaded shadow mapping and parallax occlusion mapping.
-  2D batch renderer.
-  Central event dispatching and polling for keyboard, mouse and window events.
-  Layer stack system allowing the application to receive events within a certain order.
-  Camera abstractions and simple controllers.
-  Simple instrumentation for function and scope profiling.
-  ImGui integration through the engine APIs (both rendering and input).
-  Example Brick Breaker game as a demo and test case for 2D rendering.
-  Premake build system.

Inspired by [TheCherno's Hazel Engine](https://github.com/TheCherno/Hazel) architecture.

# Gallery

BrickBreaker game showcasing particle systems and 2D batching. Also ImGui showing useful rendering statistics.

![BrickBreaker](http://www.bmlourenco.com/public/images/bhazel/BrickBreaker.gif)


2D Particle system test with over 100k particles from 2 emitters.

![ParticleSystem](http://www.bmlourenco.com/public/images/bhazel/ParticleSystem2D.gif)