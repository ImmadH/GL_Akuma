## About

- This is an OpenGL 4.6 Rendering Engine that supports GLTF 2.0 Loading & Texturing (non PBR pipeline).
- The project was mainly written in C11 except for a tiny wrapper for C++ IMGUI Libraries.
- I created this in pursuit of knowledge for 3D computer graphics and to ease my transfer into the Vulkan API
- More about the project here: (https://immadh.com/projects/glrenderer/)

## Features
- Windowing/context creation from win32 dependencies
- Noclip Camera Subsytem
- Shader Loading from file & utility
- Multi Asset GLTF 2.0 Loading and texturing (via CGLTF)
- Shadow Mapping
- Blinn Phong Lighting with Fresnel Reflectance approximation for better reflectivity/specular
- Directional Lighting
- Point Light system w/ distance based attenuation
- Scene/Asset Manager (Scale, Translation, Rotation, Color)
- GUI Subsytem (via IMGUI)
- Cubemap Skybox

## Dependencies

- CIMGUI/IMGUI
- CGLM
- CGLTF 
- STBI
- GLAD


## Resources
- (https://learnopengl.com/)
- Victor Gordon
- OGLDev
- Anton's OpenGL 4 Tutorials

## TODO
- Clean/Refactor unused code
- Add normal mapping 
- Use a uniform buffer object 
- SSAO?
- Look into utilizing more GL4.x features 
