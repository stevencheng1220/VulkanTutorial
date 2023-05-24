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
- Querying for presentation support
- Creating the presentation queue 

<br></br>


### Swap chain
- Checking for swap chain support
- Enabling device extensions
- Querying details of swap chain support
- Choosing the right settings for the swap chain
- Creating the swap chain
- Retrieving the swap chain images

<br></br>


### Image views

<br></br>


## Graphics Pipeline Basics

### Introduction

<br></br>


### Shader modules
- Vertex shader
- Fragment shader
- Per-vertex colors
- Compiling the shaders
- Loading a shader
- Creating shader modules
- Shader stage creation

<br></br>


### Fixed functions
- Dynamic state
- Vertex input
- Input Assembly
- Viewports and scissors
- Rasterizer
- Multisampling
- Depth and stencil testing
- Color blending
- Pipeline layout
- Conclusion

<br></br>


### Render pass
- Setup
- Attachment description
- Subpasses and attachment references
- Render pass

<br></br>


### Conclusion

<br></br>


## Drawing

<br></br>


### Framebuffers

<br></br>


### Command buffers
- Command pools
- Command buffer allocation
- Command buffer recording
- Starting a render pass
- Basic drawing commands
- Finishing up

<br></br>


### Rendering and presentation
- Outline of a frame
- Synchronization
- Creating the synchronization objects
- Waiting for the previous frame
- Acquiring an image from the swap chain
- Recording the command buffer
- Submitting the command buffer
- Subpass dependencies
- Presentation
- Conclusion

<br></br>


### Frames in flight

<br></br>


## Swap chain recreation
- Introduction
- Recreating the swap chain
- Suboptimal or out-of-date swap chain
- Fixing a deadlock
- Handling resizes explicitly
- Handling minimization
