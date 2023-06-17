/**
 * Vulkan header from the LunarG SDK, provides the functions, structures, and
 * enumerations.
 */
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>

// const char* VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME = "VK_KHR_portability_enumeration";
// const auto VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR = 0x00000001;

/*
 * Variables for window dimensions
 */
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 2;

/*
 * Validation layers used
 */
const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

/*
 * Check for swapchain support
 */
const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

/*
 * Enables validation layers depending on whether the code is run in debug mode
 */
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

/**
 * Create a debug messenger for debugging and validation purposes.
 *
 * @param instance The Vulkan instance handle.
 * @param pCreateInfo Pointer to the debug messenger creation information.
 * @param pAllocator Pointer to the allocation callbacks (can be nullptr).
 * @param pDebugMessenger Pointer to store the created debug messenger handle.
 * @return VK_SUCCESS if the debug messenger is created successfully.
 *         VK_ERROR_EXTENSION_NOT_PRESENT if the debug utils extension is not present.
 */
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                      const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

/**
 * Destroy a previously created debug messenger.
 *
 * @param instance The Vulkan instance handle.
 * @param debugMessenger The debug messenger to destroy.
 * @param pAllocator Pointer to the allocation callbacks (can be nullptr).
 */
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

/**
 * A struct that encompasses a queue family and whether it is supported by the device
 */
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

/**
 * Struct for querying details for swap chain support. Checks:
 *      Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
 *      Surface formats (pixel format, color space)
 *      Available presentation modes
 */
struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;

    std::vector<VkPresentModeKHR> presentModes;
};

/**
 * Struct for storing vertex data that will be passed to the vertex shader.
 */
struct Vertex
{
    glm::vec2 pos;
    glm::vec3 color;

    /**
     * Retrieves the vertex input binding description.
     *
     * @return VkVertexInputBindingDescription - The vertex input binding description, initialized with default values.
     * The binding description specifies the binding index, stride, and input rate for vertex data.
     */
    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    /**
     * Retrieves the vertex input attribute descriptions.
     *
     * @return std::array<VkVertexInputAttributeDescription, 2> - An array of vertex input attribute descriptions. The attribute descriptions specify the binding, location, format, and offset for each attribute in the vertex data.
     */
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
}

/*
 * Positions and colors of the vertices that will be displayed
 */
const std::vector<Vertex>
    vertices = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};

/*
    Reusing vertices using indices instead of creating more vertices
*/
const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0};

/**
 * Main class
 */
class HelloTriangleApplication
{
public:
    /**
     * Runs all Vulkan functions
     */
    void run()
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow *window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface; // For showing results to the screen

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device; // logical device

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkCommandPool commandPool;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void *> uniformBuffersMapped;

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;

    bool framebufferResized = false;

    /**
     * The given code initializes a window using the GLFW library and creates a non-resizable window for Vulkan
     * rendering. It sets up the necessary window hints, such as not using any specific graphics API by default. The
     * window created has a fixed size of 800x600 pixels and a title of "Vulkan".
     */
    void initWindow()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    /**
     * Callback function for framebuffer resize event.
     *
     * @param window - Pointer to the GLFW window.
     * @param width  - The new width of the framebuffer.
     * @param height - The new height of the framebuffer.
     */
    static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
    {
        auto app = reinterpret_cast<HelloTriangleApplication *>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    /**
     * Initiates all Vulkan objects
     */
    void initVulkan()
    {
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        createRenderPass();
        createDescriptorSetLayout();
        createGraphicsPipeline();
        createFramebuffers();
        createCommandPool();
        createVertexBuffer();
        createIndexBuffer();
        createUniformBuffers();
        createCommandBuffers();
        createSyncObjects();
    }

    /**
     * Loop that iterates until the window is closed. Once window is closed,
     * deallocates resources we've used in the cleanup function
     */
    void mainLoop()
    {
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(device);
    }

    /**
     * Cleans up the swap chain resources.
     *
     * This function destroys the framebuffers, image views, and swap chain itself.
     * It should be called during cleanup to release the resources allocated for the swap chain.
     */
    void cleanupSwapChain()
    {
        for (auto framebuffer : swapChainFramebuffers)
        {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        for (auto imageView : swapChainImageViews)
        {
            vkDestroyImageView(device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }

    /**
     * This function releases resources by destroying the Vulkan objects and GLFW
     * objects, and terminates the GLFW library.
     */
    void cleanup()
    {
        cleanupSwapChain();

        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyRenderPass(device, renderPass, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroyBuffer(device, uniformBuffers[i], nullptr);
            vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
        }

        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

        vkDestroyBuffer(device, indexBuffer, nullptr);
        vkFreeMemory(device, indexBufferMemory, nullptr);

        vkDestroyBuffer(device, vertexBuffer, nullptr);
        vkFreeMemory(device, vertexBufferMemory, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(device, commandPool, nullptr);

        vkDestroyDevice(device, nullptr);

        if (enableValidationLayers)
        {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }

    /**
     *
     */
    void recreateSwapChain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0)
        {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device);

        cleanupSwapChain();

        createSwapChain();
        createImageViews();
        createFramebuffers();
    }

    /**
     * Creating an instance, which is the connection between your application and the
     * Vulkan library.
     * Creating it involves specifying some details about your application to the driver.
     */
    void createInstance()
    {
        /*
         * Checks if validation layers are turned on and if all layers are supported
         */
        if (enableValidationLayers && !checkValidationLayerSupport())
        {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        /*
         * Fills in a struct with information about the application.
         * Provides useful information to the driver to optimize our specific application.
         */
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        /*
         * Not optional struct. Tells Vulkan driver which global extensions and
         * validation layers we want to use.
         */
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        /*
         * Return the required list of extensions based on whether validation layers are enabled or not
         */
        auto extensions = getRequiredExtensions();

        /*
         * Fix for `Encountered VK_ERROR_INCOMPATIBLE_DRIVER`
         * For more information: https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Instance
         */
        //        for (uint32_t i = 0; i < glfwExtensionCount; i++) {
        //            extensions.emplace_back(glfwExtensions[i]);
        //        }
        //
        //        extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        //        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

        /*
         * Include validation layer names in VkInstanceCreateInfo struct if enabled. Additionally, populate
         * debugCreateInfo with layer information
         */
        if (enableValidationLayers)
        {
            /*
             * static_cast performs explicit type conversions between compatible types at compile time.
             */
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }

        /*
         * Checker to see of vkCreateInstance is successful
         */
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create instance!");
        }
    }

    /**
     * Populates a createInfo struct with details about the messenger and its callback
     * @param createInfo is the struct to fill in
     */
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    /**
     * Sets up a debug messenger for Vulkan if validation layers are enabled. It populates a configuration
     * structure, creates the debug messenger using the Vulkan API, and throws an error if the creation fails.
     */
    void setupDebugMessenger()
    {
        if (!enableValidationLayers)
            return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    /**
     * Creates a window surface using GLFW library in Vulkan, associated with the provided window and Vulkan instance
     * . If the creation of the surface fails, an exception is thrown with an error message indicating the failure.
     */
    void createSurface()
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    /**
     * selects a suitable physical device (GPU) for Vulkan rendering by querying the available devices and checking
     * if they meet certain criteria. If a suitable device is found, it is assigned to the variable physicalDevice;
     * otherwise, an exception is thrown indicating the failure to find a suitable GPU.
     */
    void pickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0)
        {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        /*
         * Query all devices retrieved and see if the device is suitable
         */
        for (const auto &device : devices)
        {
            if (isDeviceSuitable(device))
            {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    /**
     * Creates a logical device in Vulkan with a single queue for the graphics family specified by the
     * QueueFamilyIndices structure. The code sets up the necessary parameters, including queue priority, device features, and validation layers if enabled. If the logical device creation is successful, it retrieves the handle for the graphics queue.
     */
    void createLogicalDevice()
    {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        /*
         * Create a vector of queues that stores VkDeviceQueueCreateInfos. This structure describes number of queues we
         * want for a single queue family
         */
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};

        /*
         * Logical device info struct
         */
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        /*
         * Enable device extensions
         */
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        /*
         * Instantiating logical device
         */
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create logical device!");
        }

        /*
         * Retrieve graphics family and presentation family handles
         */
        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    }

    /**
     * Responsible for creating a swap chain in Vulkan for presenting images on the screen. It retrieves the
     * necessary details about swap chain support, such as surface formats, present modes, and capabilities. It then
     * chooses the appropriate format, present mode, and extent for the swap chain. The method creates the swap
     * chain and retrieves the swap chain images, storing them for future use. Finally, it records the image format
     * and extent of the swap chain.
     */
    void createSwapChain()
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        /*
         * Decide how many images we would like to have in the swap chain. The implementation specifies the minimum
         * number that it requires to function:
         */
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities
                                                                                .maxImageCount)
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        /*
         * Creating the swap chain object
         */
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        /*
         * Specify how to handle swap chain images that will be used across multiple queue families.
         * There are two ways to handle images that are accessed from multiple queues:
         *      VK_SHARING_MODE_EXCLUSIVE: An image is owned by one queue family at a time and ownership must be
         *      explicitly transferred before using it in another queue family. This option offers the best performance.
         *      VK_SHARING_MODE_CONCURRENT: Images can be used across multiple queue families without explicit
         *      ownership transfers.
         */
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        // To specify that you do not want any transformation, simply specify the current transformation.
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

        // Image quality features
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        /*
         * With Vulkan it's possible that your swap chain becomes invalid or unoptimized while your application is
         * running, for example because the window was resized.
         *  In that case the swap chain actually needs to be recreated from scratch and a reference to the old one
         *  must be specified in this field.
         */

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    /**
     * Creates a basic image view for every image in the swap chain so that we can use them as color targets later on.
     */
    void createImageViews()
    {
        swapChainImageViews.resize(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];

            /*
             * The viewType and format fields specify how the image data should be interpreted. The viewType
             * parameter allows you to treat images as 1D textures, 2D textures, 3D textures and cube maps.
             */
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapChainImageFormat;

            /*
             * The components field allows you to swizzle the color channels around. For example, you can map all of
             * the channels to the red channel for a monochrome texture. You can also map constant values of 0 and 1
             * to a channel. In our case we'll stick to the default mapping.
             */
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            /*
             * The subresourceRange field describes what the image's purpose is and which part of the image should be
             * accessed. Our images will be used as color targets without any mipmapping levels or multiple layers.
             */
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create image views!");
            }
        }
    }

    /**
     * Before we can finish creating the pipeline, we need to tell Vulkan about the framebuffer attachments that will
     * be used while rendering. We need to specify how many color and depth buffers there will be, how many samples
     * to use for each of them and how their contents should be handled throughout the rendering operations. All of
     * this information is wrapped in a render pass object, for which we'll create a new createRenderPass function.
     */
    void createRenderPass()
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // no multisampling, so 1 sample

        /*
         * LoadOp and storeOp determine what to do with the data in the attachment before rendering
         * and after rendering
         */
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        /*
         * Not applying stencil data
         */
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        /*
         * Every subpass references one or more of attachments. We intend to use the attachment to function as a
         * color buffer.
         */
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        /*
         * Struct for subpass, this is a graphics subpass.
         */
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        /*
         * Create subpass dependencies
         */
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        /*
         * Create render pass using attachment and subpass referencing
         */
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void createDescriptorSetLayout()
    {
        /*
         * Describe a single binding within a descriptor set layout.
         */
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        /*
         * specify the descriptor set layout during pipeline creation to tell Vulkan which descriptors the shaders will be using.
         */
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    /**
     * TODO: Fix up docstring once complete
     */
    void createGraphicsPipeline()
    {
        /*
         * Creating fragment shader and vertex shader modules
         */
        auto vertShaderCode = readFile("/Users/stevencheng/CLionProjects/VulkanTutorial/shaders/vert.spv");
        auto fragShaderCode = readFile("/Users/stevencheng/CLionProjects/VulkanTutorial/shaders/frag.spv");

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        /*
         * Describe format of the vertex data to be passed to vertex shader
         */
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        /*
         * Describes geometry for vertices and if primitive restart should be enabled
         */
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        /*
         * Creating viewport
         * Viewport and scissor rectangles define the region of the framebuffer where rendering will occur. In this
         * code snippet, it indicates that a single viewport and scissor rectangle will be used, likely for simple
         * rendering scenarios.
         */
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        /*
         * Rasterizer takes geometry shaped by the vertices (vertex shader) and turns it into fragments to be colored
         * by fragment shader.
         * Also performs depth testing, face culling, and the scissor test.
         */
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        /*
         * Multisampling is one of the ways to perform anti-aliasing. It works by combining the fragment
         * shader results of multiple polygons that rasterize to the same pixel.
         */
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        /*
         * Sets the color write mask to include all color components, disables blending, and enables a copy logical
         * operation. Only one framebuffer attachment is used. Then, ensure that the output color of the pixel shader
         * directly overwrites the existing color in the framebuffer without any blending or additional logical
         * operations.
         */
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        /*
         * Certain properties of the Vulkan pipeline state, such as viewport size, line width, and blend constants,
         * can be modified at draw time without recreating the entire pipeline. To achieve this, utilize dynamic state.
         */
        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR};
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        /*
         * Sets up the pipeline layout
         */
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        /*
         * Initializes graphics pipeline struct
         */
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;

        /*
         * Referencing arrays of VkPipelineShaderStageCreateInfo using all previously created structs
         */
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;

        /*
         * Reference all structures describing fixed function stage
         */
        pipelineInfo.layout = pipelineLayout;

        /*
         * Reference render pass and subpass
         */
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
    }

    /**
     * Creates framebuffers by iterating through each image view and creating a framebuffer using the corresponding
     * attachments. The framebuffer is configured with the render pass, attachment count, attachments array, width,
     * height, and layers.
     */
    void createFramebuffers()
    {
        swapChainFramebuffers.resize(swapChainImageViews.size());

        /*
         * Iterate through image views and create framebuffers
         */
        for (size_t i = 0; i < swapChainImageViews.size(); i++)
        {
            VkImageView attachments[] = {
                swapChainImageViews[i]};

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    /**
     * Serves the purpose of creating a command pool, which is a container for allocating and managing command
     * buffers. Command pools are associated with specific queue families and are used to control command buffer
     * execution on those queues.
     */
    void createCommandPool()
    {
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create graphics command pool!");
        }
    }

    /**
     * Creates a vertex buffer for storing vertex data in Vulkan.
     * The vertex buffer is created using a staging buffer and transfer operations.
     *
     * The method performs the following steps:
     * 1. Calculates the required buffer size based on the vertex data size.
     * 2. Creates a staging buffer in host-visible and host-coherent memory properties,
     *    which allows for efficient data transfer from CPU to GPU.
     * 3. Maps the staging buffer memory and copies the vertex data into it.
     * 4. Unmaps the staging buffer memory.
     * 5. Creates the final vertex buffer in device-local memory,
     *    which provides optimal performance for GPU access.
     * 6. Performs a buffer-to-buffer copy operation to transfer the data from the staging buffer to the vertex buffer.
     * 7. Destroys the staging buffer and frees its associated memory.
     */
    void createVertexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void *data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), (size_t)bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

        copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    /**
     * Creates an index buffer for storing index data in Vulkan.
     * The index buffer is created using a staging buffer and transfer operations.
     *
     * The method performs the following steps:
     * 1. Calculates the required buffer size based on the index data size.
     * 2. Creates a staging buffer in host-visible and host-coherent memory properties,
     *    which allows for efficient data transfer from CPU to GPU.
     * 3. Maps the staging buffer memory and copies the index data into it.
     * 4. Unmaps the staging buffer memory.
     * 5. Creates the final index buffer in device-local memory,
     *    which provides optimal performance for GPU access.
     * 6. Performs a buffer-to-buffer copy operation to transfer the data from the staging buffer to the index buffer.
     * 7. Destroys the staging buffer and frees its associated memory.
     */
    void createIndexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void *data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.data(), (size_t)bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

        copyBuffer(stagingBuffer, indexBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    void createUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

            vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
        }
    }

    /**
     * Creates a Vulkan buffer with the specified size, usage, and memory properties.
     * The created buffer and its associated device memory are returned as output parameters.
     *
     * @param size The size of the buffer in bytes.
     * @param usage The usage flags specifying how the buffer will be used.
     * @param properties The memory property flags defining the desired memory properties for the buffer.
     * @param buffer Reference to a VkBuffer variable to store the created buffer handle.
     * @param bufferMemory Reference to a VkDeviceMemory variable to store the allocated device memory for the buffer.
     */
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory)
    {
        /*
         * Define class member to hold the buffer handle
         */
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create buffer!");
        }

        /*
         * Determine the right memory type for the buffer and allocate memory
         */
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        /*
         * Associate memory with the buffer, fill the vertex buffer, then
         * unmaps the memory allocated for the buffer.
         */
        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }

    /**
     * Copies data from the source buffer to the destination buffer using Vulkan commands.
     *
     * @param srcBuffer The source buffer from which data will be copied.
     * @param dstBuffer The destination buffer where data will be copied to.
     * @param size The size of the data to be copied in bytes.
     */
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        /*
         * Allocate temporary command buffer for transfer
         */
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        /*
         * Start recording command buffer
         */
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        /*
         * Contents of buffer are transferred
         */
        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        vkEndCommandBuffer(commandBuffer);

        /*
         * Create queue info and execute the transfer on the buffers immediately
         */
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

    /**
     * Finds a suitable memory type based on the given type filter and memory property flags.
     * The method searches through the available memory types provided by the physical device
     * and selects the first memory type that satisfies both the type filter and the specified
     * memory property flags.
     *
     * @param typeFilter The desired memory type filter. This is a bitmask where each bit
     * represents a memory type index. The method checks if a memory type
     * with the corresponding index is supported by the physical device.
     * @param properties The desired memory property flags. These flags define the properties
     * required for the memory type, such as being device-local, host-visible,
     * or coherent, among others.
     * @return The index of the suitable memory type found, or an invalid index if
     * no suitable memory type is found.
     */
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if (
                (typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    /**
     * Responsible for allocating command buffers within a specified command pool in Vulkan. A command buffer is a
     * data structure that holds a sequence of commands to be executed by the GPU.
     */
    void createCommandBuffers()
    {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    /**
     * Responsible for recording commands into a specified command buffer in Vulkan. This is a critical step in the
     * graphics pipeline as it defines the sequence of operations to be executed by the GPU for rendering.

     * Begins by initializing the command buffer and setting up the render pass. Then, it binds the graphics pipeline
     * and specifies viewport and scissor settings. The code includes a draw command to define the rendering
     * parameters. Finally, the render pass is ended, and the command buffer recording is completed. This process
     * establishes the sequence of operations to be executed by the GPU for rendering a frame in Vulkan.
     */
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        /*
         * Initialize command buffer
         */
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        /*
         * Initializing render pass
         */
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChainExtent;

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        /*
         * Bind graphics pipeline by specifying pipeline is a graphics one.
         * Then, specify viewport and scissor state for this pipeline to be dynamic.
         */
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChainExtent.width);
        viewport.height = static_cast<float>(swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkBuffer vertexBuffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        uint32_t firstBinding = 0;
        uint32_t bindingCount = 1;
        vkCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
        uint32_t instanceCount = 1;
        uint32_t firstIndex = 0;
        uint32_t vertexOffset = 0;
        uint32_t firstInstance = 0;
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), instanceCount, firstIndex, vertexOffset, firstInstance);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    /**
     * Responsible for creating synchronization objects, namely semaphores and fences, which ensure proper
     * coordination and synchronization between different stages of rendering.
     */
    void createSyncObjects()
    {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    /**
     * Updates the uniform buffer for the specified image.
     *
     * This function measures the elapsed time in seconds between the first and
     * current calls using a high-resolution clock and stores it in the 'time' variable.
     * The initial starting time is preserved across multiple function calls.
     *
     * @param currentImage The index of the current image.
     */
    void updateUniformBuffer(uint32_t currentImage)
    {
        /*
         * Measures the elapsed time in seconds between the first and
         * current calls to the function using a high-resolution clock and stores it
         * in the time variable. The static keyword ensures that the initial
         * starting time is preserved across multiple function calls.
         */
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        /*
         * Define model, view, and projection transformations in the uniform buffer
         * object. The model rotation will be a simple rotation around the Z-axis
         * using the time variable.
         *
         * The glm::rotate function takes an existing transformation, rotation angle
         * and rotation axis as parameters. The glm::mat4(1.0f) constructor returns
         * an identity matrix. Using a rotation angle of time * glm::radians(90.0f)
         * accomplishes the purpose of rotation 90 degrees per second.
         */
        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        /*
         * For the view transformation I've decided to look at the geometry from
         * above at a 45 degree angle. The glm::lookAt function takes the eye
         * position, center position and up axis as parameters.
         */
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        /*
         * Use a perspective projection with a 45 degree vertical field-of-view. The
         * other parameters are the aspect ratio, near, and far view planes.
         * It is important to use current swap chain extent to calculate the aspect
         * ratio to take into account the new width and height of the window after a
         * resize.
         */
        ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);

        /*
         * GLM was originally designed for OpenGL, where the Y coordinate of the
         * clip coordinates is inverted. The easiest way to compensate for that is
         * to flip the sign on the scaling factor of the Y axis in the projection
         * matrix. If you don't do this, then the image will be rendered upside down.
         */
        ubo.proj[1][1] *= -1;

        /*
         * Copy data in UBO to current uniform buffer.
         */
        memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }

    /**
     * Handles the rendering of a single frame. It waits for the previous frame to finish using fences, acquires the
     * next available image from the swap chain, records the rendering commands into a command buffer, submits the
     * command buffer to the graphics queue, and presents the result back to the swap chain for display. This process
     * ensures proper synchronization and execution of rendering operations for each frame.
     */
    void drawFrame()
    {
        /*
         * Wait for the fences to be signaled before proceeding.
         * Acquire the next image from the swap chain.
         * If swap chain is out of date, recreate it and return.
         * If acquiring the swap chain image fails, throw an exception.
         * Reset the fence for next frame.
         */
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        /*
         * Acquire next image from swap chain
         */
        VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX,
                                                imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        /*
         * Record command buffer, first reset command buffer.
         */
        vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        /*
         * Submit command buffer
         */
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        /*
         * Presentation - submitting result back to swap chain to show up on screen
         */
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
        {
            framebufferResized = false;
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    /**
     * Create a Vulkan shader module from the provided code.
     *
     * @param code A vector containing the binary code of the shader.
     * @return A Vulkan shader module.
     * @throws std::runtime_error if the shader module creation fails.
     */
    VkShaderModule createShaderModule(const std::vector<char> &code)
    {
        /*
         * The sType member is set to indicate the type of structure, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO.
         * The codeSize member is assigned the size of the code vector, while the pCode member is assigned a
         * reinterpretation of the code vector as a pointer to an array of const uint32_t.
         */
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        VkShaderModule shaderModule;

        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create shader module!"); /* Throw an error if creation fails. */
        }

        return shaderModule;
    }

    /**
     * Chooses the preferred surface format for rendering graphics on a Vulkan surface. It tries to find a surface
     * format that has a specific format and color space, and if found, returns that format.
     * @param availableFormats is a vector of all possible surface formats
     * @return the first surface format that matches the format and colorspace
     */
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
    {
        for (const auto &availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace ==
                                                                         VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    /**
     * Chooses the preferred presentation mode for displaying images on a Vulkan surface.
     * @param availablePresentModes is a vector of all possible presentation modes
     * @return the preferred mailbox mode
     */
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
    {
        for (const auto &availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    /**
     * Chooses the appropriate swap extent (size) for a Vulkan surface. It checks if the extent is already specified
     * in the capabilities object, and if not, it determines the extent based on the current framebuffer size and
     * restricts it to the supported range.
     * @param capabilities The VkSurfaceCapabilitiesKHR object representing the capabilities of the Vulkan surface.
     * @return The chosen VkExtent2D object representing the swap extent (size) for the Vulkan surface.
     */
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        else
        {
            int width, height;

            /*
             * We must use glfwGetFramebufferSize to query the resolution of the window in pixel before matching it
             * against the minimum and maximum image extent.
             * If you are using a high DPI display (like Apple's Retina display), screen coordinates don't correspond
             * to pixels.
             * Instead, due to the higher pixel density, the resolution of the window in pixel will be larger than
             * the resolution in screen, hence requiring conversion.
             */
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)};

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    /**
     * Retrieves information about the capabilities and support of a swap chain for a specific Vulkan physical device.
     * We check:
     *  Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
     *  Surface formats (pixel format, color space)
     *  Available presentation modes
     * @param device is a VkPhysicalDevice
     * @return a filled in SwapChainSupportDetails object
     */
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)
    {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    /**
     * Checks if a given device is suitable to be used as GPU
     * @param device is a VkPhysicalDevice object
     * @return boolean of if the given device has the support of the queue families
     */
    bool isDeviceSuitable(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices = findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    /**
     * Checks device support for swap chain by enumerating the extensions and check if all of the required extensions
     * are amongst them.
     * @param device is a VkPhysicalDevice object
     * @return boolean of ch
     */
    bool checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto &extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    /**
     * Checks which queue families are supported by the device and which one of these supports the commands that we
     * want to use.

     * @param device is a VkPhysicalDevice object
     * @return an index to the suitable queue family
     */
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        /*
         * We need to find at least one queue family that supports VK_QUEUE_GRAPHICS_BIT
         */
        int i = 0;
        for (const auto &queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (presentSupport)
            {
                indices.presentFamily = i;
            }

            if (indices.isComplete())
            {
                break;
            }

            i++;
        }

        return indices;
    }

    /**
     * Callback function that will return the required list of extensions based on whether validation layers are
     * enabled or not
     * @return a vector with extension names
     */
    std::vector<const char *> getRequiredExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        /*
         * VK_EXT_DEBUG_UTILS_EXTENSION_NAME macro here which is equal to the literal string "VK_EXT_debug_utils".
         * Using this macro lets you avoid typos.
         */
        if (enableValidationLayers)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    /**
     * @return true or false depending on if all the requested layers are available
     */
    bool checkValidationLayerSupport()
    {
        /*
         * Lists all available layers
         */
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        /*
         * Check if all the layers in validationLayers exist in the availableLayers
         * list. We use strcmp to check identical layer names
         */
        for (const char *layerName : validationLayers)
        {
            bool layerFound = false;

            for (const auto &layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound)
            {
                return false;
            }
        }

        return true;
    }

    /**
     * Read the contents of a file and return them as a vector of characters.
     *
     * @param filename The name of the file to read.
     * @return A vector containing the characters read from the file.
     * @throws std::runtime_error if the file fails to open.
     */
    static std::vector<char> readFile(const std::string &filename)
    {
        /*
         * Open the file in binary mode, starting at the end
         * to determine the file size.
         */
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            throw std::runtime_error("failed to open file!");
        }

        /*
         * Determine the file size and allocate a buffer accordingly.
         */
        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        /*
         * Move the file pointer to the beginning and read
         * the file contents into the buffer.
         */
        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    /**
     * The VKAPI_ATTR and VKAPI_CALL ensure that the function has the right signature for Vulkan to call it.
     *
     * @param messageSeverity specifies severity of the message
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: Diagnostic message
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: Informational message like the creation of a resource
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: Message about behavior that is not necessarily an error, but very likely a bug in your application
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: Message about behavior that is invalid and may cause crashes
     * @param messageType specifies the message type
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: Some event has happened that is unrelated to the specification or performance
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: Something has happened that violates the specification or indicates a possible mistake
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: Potential non-optimal use of Vulkan
     * @param pCallbackData refers to a VkDebugUtilsMessengerCallbackDataEXT struct containing details of the message
     * itself, with most important members being:
            pMessage: The debug message as a null-terminated string
            pObjects: Array of Vulkan object handles related to the message
            objectCount: Number of objects in array
     * @param pUserData contains a pointer that was specified during setup of the callback and allows you to pass
     * your own data to it.
     * @return a boolean that indicates if the Vulkan call that triggered the validation layer message should
     * be aborted
     */
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
    {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }
};

/**
 * Main code that is compiled and run
 * @return exit code
 */
int main()
{
    HelloTriangleApplication app;

    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}