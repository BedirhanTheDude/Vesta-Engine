// stb_image implementation for the Engine DLL. As a shared library Engine must be
// self-contained; Texture.cpp uses stbi_* and can no longer borrow the definition
// from the Sandbox translation unit at final-exe link time.
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
