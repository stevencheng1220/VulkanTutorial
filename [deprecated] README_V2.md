# Introduction to Vulkan Tutorial Notes
- Following Vulkan Tutorial at https://vulkan-tutorial.com/Introduction

- Notes
    - Any section where it's mostly implementation, you can skip and write:
    ```
    Implementation details (skip)
    ```
<br></br>
<br></br>

# Drawing a triangle

## Setup

### Base Code
- General Structure
    - This code includes the necessary headers and defines macros for a Vulkan program. The program is structured as a class with private members for Vulkan objects and functions to initialize them. 
    The main loop is used to render frames. 
    Resources are deallocated in the cleanup function. 
    If a fatal error occurs, a descriptive message is thrown as a std::runtime_error exception, which is caught in the main function and printed. One example of an error is the lack of support for a required extension.
- Resource Management
    - Vulkan objects need to be explicitly destroyed when no longer needed. While C++ offers automatic resource management options, the tutorial chooses explicit allocation and deallocation for better understanding of the API. 
    - After completing the tutorial, one can implement automatic resource management using C++ classes or custom deleters.
    - Vulkan objects are created and allocated using specific functions and destroyed using corresponding functions. The pAllocator parameter, which allows custom memory allocator callbacks, is ignored in the tutorial with a nullptr argument.
- Integrating GLFW
    - GLFW (Graphics Library Framework) is a portable open-source library that provides a simple API for creating and managing windows, handling user input, and managing OpenGL contexts. 
    - In the context of Vulkan, GLFW is commonly used as a utility library to handle window creation and management, input handling, and surface creation for Vulkan rendering applications.

<br></br>


### Instance
- Creating an instance
    - The very first thing you need to do is initialize the Vulkan library by creating an instance. 
    - The VkInstance instance is created by specifying various parameters and options during its creation. 
    - These parameters include application information, such as the application name and version, as well as the required extensions and validation layers.
    - Once the VkInstance is created, it serves as the entry point for interacting with the Vulkan API. 
    - It acts as a handle through which the application can access various Vulkan features and functionalities. The instance is used to create logical devices, manage resources, query device capabilities, and performs other Vulkan operations.
- Encountered VK_ERROR_INCOMPATIBLE_DRIVER
    - VK_ERROR_INCOMPATIBLE_DRIVER is a Vulkan error message indicating that the installed graphics driver on your system is not compatible with the Vulkan API version or does not meet the minimum requirements. 
    - To resolve the issue, you can try updating your graphics driver, ensuring your GPU supports Vulkan, checking for the required Vulkan API version, verifying operating system compatibility, and addressing any system configuration issues. 
- Checking for extension support
    - To check for extension support in Vulkan, you can use the vkEnumerateInstanceExtensionProperties function before creating an instance. This function retrieves a list of supported extensions and stores the details in an array of VkExtensionProperties. 
    - By providing a pointer to a variable, you can obtain the number of extensions available. While essential extensions like the window system interface can be required and checked for error code VK_ERROR_EXTENSION_NOT_PRESENT, optional functionality can be checked using this method.
- Cleaning up
    - The VkInstance should be destroyed only when the program is about to exit. 
    - In the cleanup function, the vkDestroyInstance function is used to destroy the instance. 
    - The function takes the instance as a parameter, and since the allocator callback is optional, nullptr is passed to it. 
    - It's important to clean up all other Vulkan resources created before destroying the instance.

<br></br>


### Validation layers
- What are validation layers?
    - The Vulkan API prioritizes minimal driver overhead, resulting in limited default error checking. Mistakes like incorrect enumerations or null pointers may cause crashes or undefined behavior. 
    - To address this, Vulkan offers optional validation layers. Validation layers are components that intercept Vulkan function calls to perform additional operations:
        1. Checking the values of parameters against the specification to detect misuse.
        2. Tracking creation and destruction of objects to find resource leaks.
        3. Checking thread safety by tracking threads that calls originate from.
        4. Logging every call and its parameters to the standard output.
        5. Tracing Vulkan calls for profiling and replaying.
- Using validation layers
    - Implementation details (skip)
- Message callback
    - The validation layers will print debug messages to the standard output by default, but we can also handle them ourselves by providing an explicit callback in our program. 
    - To set up a callback in the program to handle messages and the associated details, we have to set up a debug messenger with a callback using the VK_EXT_debug_utils extension.
- Debuggin instance creation and destruction
    - In addition to validation layers, Vulkan provides the Debug Utils Messenger extension (VK_EXT_debug_utils), which allows for more detailed debugging and logging capabilities. 
    - The vkCreateDebugUtilsMessengerEXT function is used to create a debug messenger object that receives debugging callbacks for various Vulkan events. This function requires a valid instance to have been created beforehand.
    - During instance destruction, it is necessary to properly clean up the debug messenger object using the vkDestroyDebugUtilsMessengerEXT function. This function takes the instance and the debug messenger object as parameters and ensures that all resources associated with the debug messenger are properly released.
- Testing
    - Implementation details (skip)
- Configuration
    - When working with validation layers in Vulkan, there are additional settings available beyond the flags provided in the VkDebugUtilsMessengerCreateInfoEXT structure. These settings allow for further customization of the behavior of the validation layers.
    - To access and configure these layer settings, you can navigate to the Vulkan SDK installation directory and locate the "Config" directory. Inside this directory, you will find a file named "vk_layer_settings.txt" that provides instructions on how to configure the layers.
    - To configure the layer settings for your specific application, you can copy the "vk_layer_settings.txt" file to the "Debug" and "Release" directories of your project. Then, follow the instructions provided in the file to modify the settings according to your desired behavior.
<br></br>


### Physical devices and queue families
- Selecting a physical device
    - To use Vulkan, you need to select a suitable graphics card, also known as a physical device. 
    - After initializing the Vulkan library with a VkInstance, you can query the available physical devices using vkEnumeratePhysicalDevices. Next, evaluate each device's properties and features to determine its suitability for your application. Choose the best device based on your requirements. 
    - Once selected, you can create a logical device using vkCreateDevice to interact with the chosen physical device.
- Base device suitability checks
    - To evaluate the suitability of a physical device in Vulkan, you can start by querying basic device properties using the vkGetPhysicalDeviceProperties function. 
    - This provides information such as the device name, type, and supported Vulkan version. These details help assess the device's compatibility and capabilities for your application. 
- Queue families
    - In Vulkan, operations are performed by submitting commands to queues, which originate from different queue families. Each queue family supports a specific subset of commands, such as compute or memory transfer operations. 
    - To determine the appropriate queue family for our desired commands, we implement a function called findQueueFamilies. This function queries the available queue family properties, evaluates their capabilities, and selects the queue families that support the required commands. 
    - By identifying the suitable queue families, we ensure efficient execution of operations in Vulkan. Multiple queue families may be needed for concurrent handling of different command types.
<br></br>


### Logical device and queues
- Introduction
    - In Vulkan, after selecting a physical device, the next step is to set up a logical device to interface with it. The process is similar to creating an instance and involves specifying desired features, extensions, and queue configurations. 
    - You can create multiple logical devices from the same physical device to meet varying requirements. 
    - The logical device serves as the communication channel between your application and the physical device, allowing you to execute Vulkan commands and access resources efficiently. 
- Specifying the queues to be created
    - When creating a logical device in Vulkan, you need to specify the queues to be created using the VkDeviceQueueCreateInfo structure. 
    - For a graphics queue, you set the queue family index to the desired queue family that supports graphics capabilities. 
    - Typically, only one queue is needed for graphics operations. The queueCount parameter is set to 1, and the pQueuePriorities parameter can be set to a single value of 1.0f. 
- Specifying used device features
    - The next information to specify is the set of device features that we'll be using. These are the features that we queried support for with vkGetPhysicalDeviceFeatures.
- Creating the logical device
    - To create a logical device in Vulkan, you need to fill in the VkDeviceCreateInfo structure. This structure holds various parameters that define the configuration and features of the logical device. 
    - Key fields to consider include sType, pNext, flags, queueCreateInfoCount, pQueueCreateInfos, enabledLayerCount, ppEnabledLayerNames, enabledExtensionCount, and ppEnabledExtensionNames.
    - Once the structure is filled, you can create the logical device using the vkCreateDevice function. The logical device connects your application to the selected physical device, enabling the execution of Vulkan commands and utilization of device capabilities.
- Retrieving queue handles
    - When creating a logical device in Vulkan, queues are automatically created along with the device. However, to interact with these queues and submit commands, you need to retrieve their handles. 
    - By adding a class member of type VkQueue, you can store a handle to the desired queue, such as a graphics queue. To retrieve the handle, use the vkGetDeviceQueue function, passing in the logical device, the queue family index, and the index within the queue family.
<br></br>


## Presentation

### Window surface
- Window surface creation
    - In Vulkan, to connect with the window system and present rendered images, Window System Integration (WSI) extensions like VK_KHR_surface are used. 
    - This extension exposes a VkSurfaceKHR object representing an abstract surface for image presentation. The surface is backed by the window created using GLFW. 
    - The VK_KHR_surface extension is automatically enabled as part of the required instance extensions returned by glfwGetRequiredInstanceExtensions. The surface creation should be done after instance creation and can influence physical device selection.
- Querying for presentation support
    - In Vulkan, querying for presentation support is necessary to determine if a device can present images to a created surface. 
    - Just because the Vulkan implementation supports window system integration doesn't guarantee that every device can do so. To check, we use the vkGetPhysicalDeviceSurfaceSupportKHR function to check if a queue family can present images. 
    - By finding a suitable queue family, you can create a logical device that supports both the necessary graphics operations and presenting images to the surface.
- Creating the presentation queue 
    - Creating the presentation queue in Vulkan involves modifying the logical device creation procedure to include the presentation queue and creating a member variable to hold the queue handle. 
    - The necessary queue families, such as the graphics and present queue families, are identified, and a set is used to store the unique queue families. 
    - Iterate over the set to create VkDeviceQueueCreateInfo structures for each queue family, specifying the queue family index, queue count (usually one), and queue priority. 
    - When creating the logical device, include the VkDeviceQueueCreateInfo structures along with other device creation parameters.
<br></br>


### Swap chain
- Checking for swap chain support
    - In Vulkan, the concept of a "default framebuffer" is not present. Instead, Vulkan requires the explicit creation of an infrastructure called a swap chain to manage the buffers used for rendering before they are presented on the screen. 
    - The swap chain acts as a queue of images waiting to be displayed. The application acquires an image from the swap chain to render to it and then returns it back to the queue. 
    - The specific behavior of the queue and the conditions for presenting an image depend on the swap chain configuration, but its main purpose is to synchronize the presentation of images with the screen's refresh rate.
    - Before creating a swap chain, it is necessary to check for swap chain support. Not all graphics cards are capable of directly presenting images to a screen, especially those designed for servers that lack display outputs. 
    - Additionally, since image presentation is closely tied to the window system and surfaces associated with windows, it is not part of the core Vulkan functionality. 
    - To use the swap chain, the application needs to enable the VK_KHR_swapchain device extension after verifying its support by querying for its availability.
- Enabling device extensions
    - To enable the VK_KHR_swapchain extension in Vulkan for working with the swap chain, you need to modify the logical device creation structure. 
    - Instead of setting createInfo.enabledExtensionCount to 0, you set it to the number of enabled extensions (in this case, the VK_KHR_swapchain extension). Additionally, createInfo.ppEnabledExtensionNames should be set to an array containing the names of the enabled extensions.
- Querying details of swap chain support
    - When working with Vulkan, it is not enough to simply check if a swap chain is available. There are three main properties that need to be checked before proceeding with swap chain creation:
        1. Basic surface capabilities
            - This includes information such as the minimum and maximum number of images in the swap chain and the minimum and maximum width and height of the images. 
            - These capabilities help determine the appropriate settings for the swap chain to ensure it can function correctly with the window surface.
        2. Surface formats
            - This property refers to the pixel format and color space of the images in the swap chain. Different platforms and systems may support different formats and color spaces. 
            - Querying the available surface formats allows the application to select the most suitable format for rendering and displaying images.
        3. Presentation modes
            - The presentation mode determines how the images in the swap chain are presented to the screen. Different modes may offer different synchronization and display behaviors.
- Choosing the right settings for the swap chain
    - It is crucial to choose the optimal settings for the swap chain. This involves determining the surface format (color depth), presentation mode (conditions for swapping images to the screen), and swap extent (resolution of images in the swap chain). 
    - By querying the available surface formats, presentation modes, and surface capabilities, you can select the most suitable options based on factors such as color depth, desired performance, synchronization, and window system constraints. 
- Creating the swap chain
    - The createSwapChain function is called after logical device creation in the initVulkan function. Within the createSwapChain function, the surface format, presentation mode, swap extent, and other relevant settings are determined. 
    - Finally, the swap chain is created using the vkCreateSwapchainKHR function, ensuring that the swap chain is properly configured for synchronized image presentation with the screen.
- Retrieving the swap chain images
    - To summarize, after creating the swap chain in Vulkan, the next step is to retrieve the handles of the VkImages within the swap chain. This can be done by calling vkGetSwapchainImagesKHR function. 
    - First, query the number of images in the swap chain to resize the container accordingly. Then, call vkGetSwapchainImagesKHR again to retrieve the handles and store them in a vector. These handles will be used for rendering operations. 
    - No explicit cleanup is needed for the swap chain images as they will be automatically cleaned up when the swap chain is destroyed.
<br></br>


### Image views
- In Vulkan, an image view is used to access the data stored in a VkImage object. It acts as a window or view into the image, specifying how to interpret and access the pixels. Creating a VkImageView is necessary to use VkImages in the render pipeline. 
- For the swap chain images, a basic image view is created for each image, allowing them to be used as color targets in rendering operations.
<br></br>


## Graphics Pipeline Basics

### Introduction
![Alt text](README_Media/vulkan_simplified_pipeline.svg)
1. Input assembler
    - Collects raw vertex data from the buffers you specify and may also use an index buffer to repeat certain elements without having to duplicate the vertex data itself.
2. Vertex shader
    - Run for every vertex and generally applies transformations to turn vertex positions from model space to screen space. It also passes per-vertex data down the pipeline.
3. Tessellation shaders
    - Allows you to subdivide geometry based on certain rules to increase the mesh quality. This is often used to make surfaces like brick walls and staircases look less flat when they are nearby.
4. Geometry shader
    - Run on every primitive (triangle, line, point) and can discard it or output more primitives than the amount that came in.
5. Rasterization stage
    - Discretizes the primitives into fragments. These are the pixel elements that they fill on the framebuffer. 
    - Any fragments that fall outside the screen are discarded and the attributes outputted by the vertex shader are interpolated across the fragments.
    - Usually fragments that are behind other primitive fragments are also discarded here because of depth testing.
6. Fragment shader
    - Invoked for every fragment that survives and determines which framebuffer(s) the fragments are written to and with which color and depth values.
    - It can do this using the interpolated data from the vertex shader, which can include things like texture coordinates and normals for lighting.
7. Color blending
    - Applies operations to mix different fragments that map to the same pixel in the framebuffer.
    - Fragments can simply overwrite each other, add up or be mixed based upon transparency.
<br></br>

- Difference between fixed-function and programmable stages.
    - Stages with a green color are fixed-function stages. These stages allow you to tweak their operations using parameters, but the way they work is predefined.
    - Stages with an orange color on the other hand are programmable, meaning that you can upload your own code to the graphics card to apply exactly the operations you want.
        - This allows you to use fragment shaders, for example, to implement anything from texturing and lighting to ray tracers. These programs run on many GPU cores simultaneously to process many objects, like vertices and fragments in parallel.
<br></br>


### Shader modules
- Introduction
    - In Vulkan, shader code needs to be specified in a bytecode format called SPIR-V, unlike earlier APIs that used human-readable syntax like GLSL and HLSL. 
    - SPIR-V is designed to be used with Vulkan and OpenCL and offers advantages such as simplified compiler implementation and improved compatibility across different GPU vendors. 
    - Khronos provides a vendor-independent compiler that translates GLSL code to SPIR-V, ensuring compliance with the standard. Google's glslc.exe can also be used for this purpose and provides additional features. 
    - GLSL itself is a shading language with a C-style syntax, featuring global variables for input and output handling, built-in vector and matrix primitives, and functions for graphics programming operations.
- Vertex shader
    - The vertex shader in Vulkan processes individual vertices, taking attributes like position, color, normal, and texture coordinates as input. 
    - It calculates the final position in clip coordinates and determines the attributes to be passed on to the fragment shader, such as color and texture coordinates. These values are then interpolated by the rasterizer to create a smooth gradient across the fragments.
    - Clip coordinates are four-dimensional vectors generated by the vertex shader. These coordinates are transformed into normalized device coordinates by dividing the vector by its last component. 
    - Normalized device coordinates are homogeneous coordinates that map the framebuffer to a coordinate system ranging from -1 to 1 in both dimensions. Additionally, the Z coordinate now ranges from 0 to 1.
![Normalized Device Coordinates](README_Media/normalized_device_coordinates.svg)
- Fragment shader
    - The triangle that is formed by the positions from the vertex shader fills an area on the screen with fragments. The fragment shader is invoked on these fragments to produce a color and depth for the framebuffer (or framebuffers).
- Per-vertex colors
    - Implementation details (skip)
- Compiling the shaders
    - In Vulkan, to compile shaders, you need to create a "shaders" directory in your project's root directory. Inside this directory, you store the vertex shader in a file called "shader.vert" and the fragment shader in a file called "shader.frag".
- Loading a shader
    - In Vulkan, after compiling shaders into SPIR-V bytecode, they need to be loaded into the program for use in the graphics pipeline. 
    - To simplify this process, a helper function can be implemented to load the binary data from the shader files. This function reads the contents of the shader files, retrieves the binary data, and stores it in a suitable data structure.
- Creating shader modules
    - In Vulkan, shader code needs to be wrapped in a VkShaderModule object before it can be used in the graphics pipeline.
    - This shader module can be utilized when defining the shader stages in the graphics pipeline setup.
- Shader stage creation
    - In Vulkan, shaders are assigned to specific pipeline stages using VkPipelineShaderStageCreateInfo structures during the creation of the graphics pipeline. These structures contain information about the shader stage type, shader module, entry point function name, and specialization data.
    - To assign the vertex shader, the VkPipelineShaderStageCreateInfo structure is filled with the appropriate stage type (VK_SHADER_STAGE_VERTEX_BIT), the VkShaderModule object containing the shader code.
<br></br>


### Fixed functions
- Dynamic state
    - Dynamic state allows for certain properties in a graphics pipeline to be changed during drawing without recreating the entire pipeline. Examples include viewport size, line width, and blend constants. 
    To utilize dynamic state, a VkPipelineDynamicStateCreateInfo structure is used. This approach provides more flexibility and is commonly used for properties like viewport and scissor state, simplifying the pipeline setup.
- Vertex input
    - The VkPipelineVertexInputStateCreateInfo structure describes the format of the vertex data that will be passed to the vertex shader. It describes this in roughly two ways:
        - Bindings: spacing between data and whether the data is per-vertex or per-instance (see instancing).
        - Attribute descriptions: type of the attributes passed to the vertex shader, which binding to load them from and at which offset.
- Input Assembly
    - The VkPipelineInputAssemblyStateCreateInfo struct describes two things: what kind of geometry will be drawn from the vertices and if primitive restart should be enabled. 
- Viewports and scissors
    - The viewport specifies how the normalized device coordinates are transformed into the pixel coordinates of the framebuffer (3D to 2D).
    - Scissor is the area where you can render, this is similar to viewport in that regard but changing the scissor rectangle doesn't affect the coordinates.
    - These properties can be set statically in the pipeline or dynamically during command buffer recording. Dynamic state offers more flexibility and allows for multiple viewports and scissor rectangles. 
- Rasterizer
    - The rasterizer takes the geometry that is shaped by the vertices from the vertex shader and turns it into fragments to be colored by the fragment shader.
    - It also performs depth testing, face culling and the scissor test, and it can be configured to output fragments that fill entire polygons or just the edges (wireframe rendering). 
    - All this is configured using the VkPipelineRasterizationStateCreateInfo structure.
- Multisampling
    - The VkPipelineMultisampleStateCreateInfo struct configures multisampling, which is one of the ways to perform anti-aliasing. 
    - It works by combining the fragment shader results of multiple polygons that rasterize to the same pixel. This mainly occurs along edges, which is also where the most noticeable aliasing artifacts occur. 
    - Because it doesn't need to run the fragment shader multiple times if only one polygon maps to a pixel, it is significantly less expensive than simply rendering to a higher resolution and then downscaling. Enabling it requires enabling a GPU feature.
- Depth and stencil testing
    - If you are using a depth and/or stencil buffer, then you also need to configure the depth and stencil tests using VkPipelineDepthStencilStateCreateInfo.
- Color blending
    - After a fragment shader has returned a color, it needs to be combined with the color that is already in the framebuffer. This transformation is known as color blending and there are two ways to do it:
        - Mix the old and new value to produce a final color
        - Combine the old and new value using a bitwise operation
    - There are two types of structs to configure color blending. The first struct, VkPipelineColorBlendAttachmentState contains the configuration per attached framebuffer and the second struct, VkPipelineColorBlendStateCreateInfo contains the global color blending settings.
- Pipeline layout
    - Pipeline layout is used to specify uniform values in shaders, allowing for dynamic changes without shader recreation. These values are often used for transformations or texture samplers. 
    - During pipeline creation, a VkPipelineLayout object needs to be created, even if it's empty at this stage.
<br></br>


### Render pass
- Setup
    - Before we can finish creating the pipeline, we need to tell Vulkan about the framebuffer attachments that will be used while rendering. 
    - We need to specify how many color and depth buffers there will be, how many samples to use for each of them and how their contents should be handled throughout the rendering operations. All of this information is wrapped in a render pass object.
- Attachment description
    - The code is defining a render pass in Vulkan with a single color buffer attachment. The color attachment's format matches the swap chain images, and there is no multisampling. 
    - The loadOp is set to clear the attachment to a constant value (black) before rendering, and the storeOp is set to store the rendered contents for later reading. The stencilLoadOp and stencilStoreOp are set to VK_ATTACHMENT_LOAD_OP_DONT_CARE and VK_ATTACHMENT_STORE_OP_DONT_CARE, respectively, indicating that the stencil buffer is not used. 
    - The initialLayout is VK_IMAGE_LAYOUT_UNDEFINED, indicating that the previous layout of the image doesn't matter, and the finalLayout is VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, indicating that the image should be ready for presentation using the swap chain after rendering.
- Subpasses and attachment references
    - A single render pass can consist of multiple subpasses. Subpasses are subsequent rendering operations that depend on the contents of framebuffers in previous passes, for example a sequence of post-processing effects that are applied one after another. 
    - If you group these rendering operations into one render pass, then Vulkan is able to reorder the operations and conserve memory bandwidth for possibly better performance. 
- Render pass
    - Implementation details (skip)

<br></br>


### Conclusion
- We can now combine all the structures and objects from the previous chapters to create a graphics pipeline! Here's the types of objects we have now, as a quick recap:
    - Shader stages
        - The shader modules that define the functionality of the programmable stages of the graphics pipeline.
    - Fixed-function state
        - All of the structures that define the fixed-function stages of the pipeline, like input assembly, rasterizer, viewport and color blending.
    - Pipeline layout
        - The uniform and push values referenced by the shader that can be updated at draw time.
    - Render pass
        - The attachments referenced by the pipeline stages and their usage.

<br></br>


## Drawing

### Framebuffers
- A framebuffer is a collection of image resources used for rendering or post-processing, It represents the final output displayed on the screen or used for further processing.
- A framebuffer consists of attachments, such as color buffers and depth buffers, which store pixel data. These attachments are created with specific properties, like format and size, and are associated with a render pass that defines the rendering operations.
- By binding a framebuffer to the graphics pipeline, the output of rendering commands can be directed to its attachments.
<br></br>


### Command buffers
- Command pools
    - Commands in Vulkan, like drawing operations and memory transfers, are not executed directly using function calls. You have to record all of the operations you want to perform in command buffer objects.
    - The advantage of this is that when we are ready to tell the Vulkan what we want to do, all of the commands are submitted together and Vulkan can more efficiently process the commands since all of them are available together.
    - Before creating command buffers, we have to create a command pool. Command pools manage the memory that is used to store the buffers and command buffers are allocated from them.
- Command buffer allocation
    - To allocate command buffers in Vulkan, we create a VkCommandBuffer object and allocate it from a command pool using the vkAllocateCommandBuffers function. 
    - The allocation is performed in the createCommandBuffer function, which takes a VkCommandBufferAllocateInfo struct specifying the command pool and the number of buffers to allocate. 
    - The level parameter determines whether the command buffers are primary or secondary, with primary buffers being executable and secondary buffers being callable from primary ones. 
- Command buffer recording
    - Recording begins by calling vkBeginCommandBuffer, passing a VkCommandBufferBeginInfo structure that specifies the details of the command buffer usage.
    - If the recording starts successfully, we can proceed to add commands to the buffer. It's important to note that once a command buffer is recorded, it cannot be appended to later. If needed, the buffer must be reset before starting a new recording.
- Starting a render pass
    - To begin rendering in Vulkan, we use the vkCmdBeginRenderPass command. It requires a VkRenderPassBeginInfo structure to configure the render pass. This structure includes the render pass itself and the framebuffer to bind. 
    - The framebuffer is selected based on the swapchain image index to determine the appropriate color attachment. The render area is specified to define the size of the area where shader loads and stores will occur. It should match the attachment size for optimal performance. 
    - Clear values can be provided for clearing the attachments, such as the color attachment, and in this case, a black color with full opacity is used.
- Basic drawing commands
    - Basic drawing commands involve binding the graphics pipeline using vkCmdBindPipeline, setting the viewport and scissor state with vkCmdSetViewport and vkCmdSetScissor, and issuing the draw command with vkCmdDraw.
<br></br>


### Rendering and presentation
- Outline of a frame
    - At a high level, rendering a frame in Vulkan consists of a common set of steps:
        - Wait for the previous frame to finish.
        - Acquire an image from the swap chain.
        - Record a command buffer which draws the scene onto that image.
        - Submit the recorded command buffer.
        - Present the swap chain image.
- Synchronization
    - A core design philosophy in Vulkan is that synchronization of execution on the GPU is explicit. 
    - The order of operations is up to us to define using various synchronization primitives which tell the driver the order we want things to run in. This means that many Vulkan API calls which start executing work on the GPU are asynchronous, the functions will return before the operation has finished.
    - In this chapter there are a number of events that we need to order explicitly because they happen on the GPU, such as:
        - Acquire an image from the swap chain
        - Execute commands that draw onto the acquired image
        - Present that image to the screen for presentation, returning it to the swapchain
    - Each of these events is set in motion using a single function call, but are all executed asynchronously. 
    - The function calls will return before the operations are actually finished and the order of execution is also undefined. We need to explore which primitives we can use to achieve the desired ordering.
- Semaphores
    - A semaphore is used to add order between queue operations. Queue operations refer to the work we submit to a queue, either in a command buffer or from within a function.
    - There are two kinds of semaphores in Vulkan, binary and timeline, Because only binary semaphores will be used in this tutorial, we will not discuss timeline semaphores.
    - A semaphore is a synchronization mechanism that can be in either a signaled or unsignaled state. Initially, a semaphore is unsignaled. In the context of ordering queue operations, we can use a semaphore by designating it as a "signal" semaphore in one operation and as a "wait" semaphore in another operation.
- Fences
    - A fence has a similar purpose, in that it is used to synchronize execution, but it is for ordering the execution on the CPU, otherwise known as the host. Simply put, if the host needs to know when the GPU has finished something, we use a fence.
    - Similar to semaphores, fences are either in a signaled or unsignaled state. Whenever we submit work to execute, we can attach a fence to that work. 
    - When the work is finished, the fence will be signaled. Then we can make the host wait for the fence to be signaled, guaranteeing that the work has finished before the host continues.
- Creating the synchronization objects
    - Implementation details (skip)
- Waiting for the previous frame
    - In order to wait for the previous frame to finish before starting a new frame in Vulkan, we use the vkWaitForFences function. It waits for the specified fence (inFlightFence) to be signaled before continuing. 
    - After waiting, we reset the fence to an unsignaled state using vkResetFences. However, in the first frame, there is no previous frame to signal the fence, so we create the fence with the VK_FENCE_CREATE_SIGNALED_BIT flag to ensure it is initially signaled and the waiting call returns immediately.
- Acquiring an image from the swap chain
    - The next thing we need to do is acquire an image from the swap chain.
    - The first two parameters of vkAcquireNextImageKHR are the logical device and the swap chain from which we wish to acquire an image. The third parameter specifies a timeout in nanoseconds for an image to become available.
    - The next two parameters specify synchronization objects that are to be signaled when the presentation engine is finished using the image. That's the point in time where we can start drawing to it. It is possible to specify a semaphore, fence or both.
    - The last parameter specifies a variable to output the index of the swap chain image that has become available. 
- Recording the command buffer
    - With the imageIndex specifying the swap chain image to use in hand, we can now record the command buffer. First, we call vkResetCommandBuffer on the command buffer to make sure it is able to be recorded.
    - Then we can record command buffer.
- Submitting the command buffer
    - In Vulkan, a command buffer is submitted through queue submission, using the VkSubmitInfo structure to manage synchronization. 
    - Semaphores dictate when to begin execution and at which pipeline stages, aiming to delay writing colors until images are available. 
    - Command buffers are submitted for execution, and specified semaphores are signaled once this is complete. 
    - Through the use of vkQueueSubmit and an optional fence, this process can manage command buffer reuse, by signalling when execution has finished and it's safe to begin recording new commands.
- Subpass dependencies
    - In Vulkan, subpass dependencies manage image layout transitions within a render pass. These dependencies specify memory and execution relations between subpasses, including operations before and after the subpass. 
    - Built-in dependencies handle transitions at the start and end of the render pass, but timing issues can arise. One solution is to make the render pass wait for a specific pipeline stage. 
    - Subpass dependencies are defined via VkSubpassDependency structures, detailing the dependency and dependent subpass, operations to wait on and their respective stages. Ultimately, these configurations ensure proper execution sequence, preventing premature transitions and ensuring that necessary operations like color writing are completed at the right time.
- Presentation
    - In Vulkan, the final stage of rendering a frame is submission of the frame to the swap chain for display, a process known as presentation. 
    - This process utilizes a VkPresentInfoKHR structure to configure the parameters. The waitSemaphoreCount and pWaitSemaphores parameters define which semaphores should be waited on before the presentation begins, while the swapchainCount, pSwapchains, and pImageIndices parameters specify the swap chains to which images are presented and the index of the image in each swap chain.
    - The vkQueuePresentKHR function is used to submit the request to present an image to the swap chain.
<br></br>

### Frames in flight
- Frames in flight
    - Right now our render loop has one glaring flaw. We are required to wait on the previous frame to finish before we can start rendering the next which results in unnecessary idling of the host.
    - The way to fix this is to allow multiple frames to be in-flight at once, that is to say, allow the rendering of one frame to not interfere with the recording of the next. 
    - How do we do this? Any resource that is accessed and modified during rendering must be duplicated. Thus, we need multiple command buffers, semaphores, and fences. In later chapters we will also add multiple instances of other resources, so we will see this concept reappear.
<br></br>


## Swap chain recreation
- Introduction
    - The application we have now successfully draws a triangle, but there are some circumstances that it isn't handling properly yet. It is possible for the window surface to change such that the swap chain is no longer compatible with it. One of the reasons that could cause this to happen is the size of the window changing. We have to catch these events and recreate the swap chain.
- Recreating the swap chain
    - Swap chain can be recreated through a sequence of operations involving: waiting for the device to become idle, cleaning up the old swap chain and related resources, then creating a new swap chain, image views, and framebuffers.
    - Cleanup includes destroying the framebuffers, image views, and the swap chain itself. While this approach requires halting all rendering during the swap chain recreation, it's possible to construct a new swap chain while commands from the old swap chain are still in-flight. 
    - However, this involves passing the previous swap chain to the oldSwapChain field in the VkSwapchainCreateInfoKHR structure, and destroying it once it's no longer in use.
- Suboptimal or out-of-date swap chain
    - Swap chain recreation is necessitated by two key signals: VK_ERROR_OUT_OF_DATE_KHR, indicating incompatibility with the surface often due to window resizing, and VK_SUBOPTIMAL_KHR, indicating that the swap chain can present to the surface, but its properties no longer match it perfectly. 
    - Both vkAcquireNextImageKHR and vkQueuePresentKHR functions can return these values. When either of these conditions is encountered, the swap chain should be recreated to ensure optimal rendering. This recreation should ideally take place immediately upon receiving these signals to avoid presentation issues.
- Fixing a deadlock
    - A deadlock can occur if an application reaches vkWaitForFences but never proceeds beyond it, often due to the early resetting of a fence which is then never signaled. 
    - This can occur when vkAcquireNextImageKHR returns VK_ERROR_OUT_OF_DATE_KHR, leading to the swapchain's recreation and an immediate return from drawFrame. The deadlock can be prevented by delaying the resetting of the fence until it is certain that work will be submitted with it. 
    - If an early return occurs, the fence remains signaled, and vkWaitForFences won't cause a deadlock during the next use of the same fence object.
- Handling resizes explicitly
    - Although many platforms automatically trigger a VK_ERROR_OUT_OF_DATE_KHR upon window resizing, Vulkan doesn't guarantee this. Therefore, explicit handling of resizes is crucial. 
    - This can be achieved by adding a framebufferResized boolean variable and checking it alongside other conditions in the drawFrame function. To detect resizes, GLFW's glfwSetFramebufferSizeCallback function is used to set up a callback. 
    - Further, an arbitrary pointer can be stored in the GLFWwindow using glfwSetWindowUserPointer, enabling retrieval of the application instance within the callback to properly set the framebufferResized flag. This mechanism ensures the framebuffer resizes properly along with the window.
- Handling minimization
    - There is another case where a swap chain may become out of date and that is a special kind of window resizing: window minimization. 
    - This case is special because it will result in a frame buffer size of 0. In this tutorial we will handle that by pausing until the window is in the foreground again.
<br></br>

# Vertex buffers

## Vertex input description
- Introduction
- Vertex shader
- Vertex data
- Binding descriptions
- Attribute descriptions
- Pipeline vertex input
<br></br>

## Vertex buffer creation
- Introduction
- Buffer creation
- Memory requirements
- Memory allocation
- Filling the vertex buffer
- Binding the vertex buffer
<br></br>

## Staging buffer
- Introduction
- Transfer queue
- Abstracting buffer creation
- Using a staging buffer
- Conclusion
<br></br>

## Index buffer
- Introduction
- Index buffer creation
- Using an index buffer
<br></br>

# Uniform buffers

## Descriptor layout and buffer
- Introduction
- Vertex shader
- Descriptor set layout
- Uniform buffer
- Updating uniform data
<br></br>

## Descriptor pool and sets
- Introduction
- Descriptor pool
- Descriptor set
- Using descriptor sets
- Alignment requirements
- Multiple descriptor sets
<br></br>

# Texture mapping

## Images
- Introduction
- Imagae library
- Loading an image
- Staging buffer
- Texture Image
- Layout transitions
- Copying buffer to image
- Preparing the texture image
- Transition barrier masks
- Cleanup
<br></br>

## Image view and sampler
- Texture image view
- Samplers
- Anisotropy device feature
<br></br>

## Combined image sampler
- Introduction
- Updating the descriptors
- Texture coordinates
- Shaders
<br></br>

# Depth buffering
- Introduction
- 3D geometry
- Depth image and view
- Explicitly transitioning the depth image
- Render pass
- Framebuffer
- Clear values
- Depth and stencil state
- Handling window resize
<br></br>

# Loading models
- Introduction
- Library
- Sample mesh
- Loading vertices and indices
- Vertex deduplication
<br></br>

# Generating Mipmaps
- Introduction
- Image creation
- Generating Mipmaps
- Linear filtering support
- Sampler
<br></br>

# Multisampling
- Introduction
- Getting available sample count
- Setting up a render target
- Adding new attachments
- Quality improvements
- Conclusion
<br></br>

# Computer Shader
- Introduction
- Advantages
- The Vulkan pipeline
- An example
- Data manipulation
- Shader storage buffer objects (SSBO)
- Storage images
- Compute queue families
- The compute shader stage
- Loading compute shaders
- Preparing the shader storage buffers
- Descriptors
- Compute pipelines
- Compute space
- Compute shaders
- Running compute commands
- Dispatch
- Submitting work
- Synchronizing graphics and compute
- Drawing the particle system
- Conclusion
<br></br>