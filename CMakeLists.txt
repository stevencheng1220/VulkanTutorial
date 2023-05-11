cmake_minimum_required(VERSION 3.21)
project(VulkanTutorial)

# Turn on C++11 to fix issue of
# Non-aggregate type 'const std::vector<type>' cannot be initialized with an initializer list
# Update: 3/6/2023 Change back from C++11 to C++17
# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_CXX_STANDARD 17)

# Add header file
set(GENERAL /usr/local/include)
set(GLEW_H /usr/local/Cellar/glew/2.2.0_1/include)
set(GLFW_H /usr/local/Cellar/glfw/3.3.8/include)
set(GLM_H /usr/local/Cellar/glm/0.9.9.8/include)
set(ASSIMP_H /usr/local/Cellar/assimp/5.2.5/include)
set(VULKAN_H /usr/local/VulkanSDK/macOS/include)
include_directories(${GENERAL} ${GLEW_H} ${GLFW_H} ${GLM_H} ${ASSIMP_H} ${VULKAN_H})

# Add target link
set(GLEW_LINK /usr/local/Cellar/glew/2.2.0_1/lib/libGLEW.2.2.0.dylib)
set(GLFW_LINK /usr/local/Cellar/glfw/3.3.8/lib/libglfw.3.3.dylib)
set(ASSIMP_LINK /usr/local/Cellar/assimp/5.2.5/lib/libassimp.5.2.4.dylib)
set(VULKAN_LINK /usr/local/VulkanSDK/macOS/lib/libvulkan.1.2.198.dylib)
set(VULKAN1_LINK /usr/local/VulkanSDK/macOS/lib/libvulkan.1.dylib)
link_libraries(${OPENGL} ${GLEW_LINK} ${GLFW_LINK} ${ASSIMP_LINK} ${VULKAN_LINK} ${VULKAN1_LINK})

# Set environment variables
set(VK_ICD_FILENAMES /usr/local/VulkanSDK/macOS/share/vulkan/icd.d/MoltenVK_icd.json)
set(VK_LAYER_PATH /usr/local/VulkanSDK/macOS/share/vulkan/explicit_layer.d)

# Executable files
add_executable(VulkanTutorial main.cpp)
