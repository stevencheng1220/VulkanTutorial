# VulkanTutorial
Following Vulkan Tutorial at https://vulkan-tutorial.com/Introduction

## Drawing a triangle

### Setup
#### Base code
- Starting from scratch and creating the base code for this tutorial.
- Vulkan works perfectly fine without creating a window if you want to use it for 
  off-screen rendering. However, in order to create a window, we need to use GLFW.
- GLFW (Graphics Library Framework) is an open-source, multi-platform library that 
  provides a simple API for creating windows, handling input events, and managing 
  OpenGL/Vulkan contexts. 

#### Instance
- The code demonstrates how to initialize the Vulkan library by creating an instance. 
- It involves providing information about the application to the driver, such as the 
  application name, version, and desired extensions. 
- The createInstance() function sets up the necessary structures and parameters, 
  including the application information and the desired global extensions, then calls 
  vkCreateInstance() to create the Vulkan instance. 
- Error handling is implemented to check for successful instance creation, and a 
  cleanup function is provided to destroy the instance before program termination.

#### Validation layers
- Validation layers in Vulkan are optional components that can be added to the Vulkan 
  API to provide additional error checking and debugging capabilities.
- They hook into Vulkan function calls to perform operations such as checking 
  parameter values against specifications, tracking object creation and destruction to 
  detect resource leaks, checking thread safety, logging function calls and parameters,
  and tracing for profiling and replaying purposes. 
- These validation layers help identify errors and potential issues in Vulkan 
  applications, improving reliability and ensuring adherence to the Vulkan 
  specification. 
- They can be enabled during development and debugging and disabled in release builds 
  for optimal performance.

#### Physical devices and queue families
- After initializing the Vulkan library with a VkInstance, the next step is to select 
  a suitable graphics card (or physical device) that supports the necessary features. 
- To begin, the available physical devices are queried using vkEnumeratePhysicalDevices.
  If no devices are found, an error is thrown. Otherwise, an array is allocated to 
  hold the physical device handles.
- The suitability of each physical device is then evaluated, which is done by 
  checking the device properties and features obtained through 
  vkGetPhysicalDeviceProperties and vkGetPhysicalDeviceFeatures, respectively.
- Next, the tutorial introduces the concept of queue families. 
- Different types of queues from different families support specific types of commands. 
- The findQueueFamilies function is added to search for the necessary queue families 
  supported by the device. The function queries the queue family properties using 
vkGetPhysicalDeviceQueueFamilyProperties. It iterates over the queue families and 
  checks for the presence of a graphics queue using the VK_QUEUE_GRAPHICS_BIT flag. 
- If a suitable graphics queue family is found, its index is stored in the 
  QueueFamilyIndices struct. 

#### Logical device and queues
- To interface with the selected physical device, a logical device needs to be set up 
  in Vulkan. This process is similar to creating an instance and involves specifying 
  the desired queues and device features. Multiple logical devices can be created from 
  the same physical device to accommodate varying requirements.
- The creation of a logical device includes defining the queues to be created, such as 
  a graphics queue, using VkDeviceQueueCreateInfo. The number of queues and their 
  priorities can be specified.
- The main VkDeviceCreateInfo struct is then filled with the queue creation info and 
  device features pointers. The logical device is instantiated using vkCreateDevice. 
- The handle to interact with the created queues, such as the graphics queue, can be 
  retrieved using vkGetDeviceQueue. These queues are automatically cleaned up when the 
  device is destroyed.








