# Bhazel

Work in progress C++ and Vulkan game engine.

# Current features:
-  Physically-based renderer with image-based lighting and HDR.
-  Cascaded shadow mapping and parallax occlusion mapping.
-  Post-processing effects: bloom, FXAA and tone mapping.
-  2D batch renderer.
-  Central event dispatching and polling for keyboard, mouse and window events.
-  Simple instrumentation for function and scope profiling.
-  Dear ImGui integration through the engine APIs (both rendering and input).
-  Premake build system.

Inspired by [TheCherno's Hazel Engine](https://github.com/TheCherno/Hazel) architecture.

# Gallery

PBR test.

![PBRTest](http://www.bmlourenco.com/portfolio/bhazel/images/gallery/4.jpg)

Scene test.

![SceneTest](http://www.bmlourenco.com/portfolio/bhazel/images/gallery/1.jpg)

BrickBreaker game showcasing particle systems and 2D batching. Also ImGui showing useful rendering statistics.

![BrickBreaker](http://www.bmlourenco.com/portfolio/bhazel/images/gallery/2.gif)

2D Particle system test with over 100k particles from 2 emitters.

![ParticleSystem](http://www.bmlourenco.com/portfolio/bhazel/images/gallery/3.gif)