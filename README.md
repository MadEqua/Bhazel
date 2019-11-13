# Bhazel

Work in progress game engine written in C++.

Current features:
-  Support for multiple rendering APIs (OpenGL, Direct3D 11 and Vulkan) and platforms (GLFW and Win32).
-  Layer system allowing the application to receive both engine and input events within a certain order. Layers also have a specified update and render order.
-  Thin abstraction of the rendering API objects and functionality.
-  2D Renderer written on top of the rendering API abstraction.
-  Camera abstractions and simple controllers.
-  ImGui integration through the engine APIs (both rendering and input).
-  Example Sandbox application for experimentation as a client application.


Heavily inspired by [TheCherno's Hazel Engine](https://github.com/TheCherno/Hazel) architecture.