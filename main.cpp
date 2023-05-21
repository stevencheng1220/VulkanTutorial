/**
 * Vulkan header from the LunarG SDK, provides the functions, structures, and
 * enumerations.
 */
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>

/*
 * Variables for window dimensions
 */
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

/*
 * Validation layers used
 */
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

/*
 * Check for swapchain support
 */
const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

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
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT*
                                      pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
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
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const
VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,"vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

/**
 * A struct that encompasses a queue family and whether it is supported by the device
 */
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

/**
 * Struct for querying details for swap chain support. Checks:
 *      Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
 *      Surface formats (pixel format, color space)
 *      Available presentation modes
 */
 struct SwapChainSupportDetails {
     VkSurfaceCapabilitiesKHR capabilities;
     std::vector<VkSurfaceFormatKHR> formats;

     std::vector<VkPresentModeKHR> presentModes;
 };


/**
 * Main class
 */
class HelloTriangleApplication {
public:
    /**
     * Runs all Vulkan functions
     */
    void run() {
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

    /**
     * The given code initializes a window using the GLFW library and creates a non-resizable window for Vulkan
     * rendering. It sets up the necessary window hints, such as not using any specific graphics API by default. The
     * window created has a fixed size of 800x600 pixels and a title of "Vulkan".
     */
    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    /**
     * Initiates all Vulkan objects
     */
    void initVulkan() {
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
    }

    /**
     * Loop that iterates until the window is closed. Once window is closed,
     * deallocates resources we've used in the cleanup function
     */
    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    /**
     * This function releases resources by destroying the Vulkan objects and GLFW
     * objects, and terminates the GLFW library.
     */
    void cleanup() {
        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(device, swapChain, nullptr);
        vkDestroyDevice(device, nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }

    /**
     * Creating an instance, which is the connection between your application and the
     * Vulkan library.
     * Creating it involves specifying some details about your application to the driver.
     */
    void createInstance() {
        /*
         * Checks if validation layers are turned on and if all layers are supported
         */
        if (enableValidationLayers && !checkValidationLayerSupport()) {
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
         * Vulkan is a platform agnostic API, which means that you need an extension to
         * interface with the window system. GLFW has a handy built-in function that
         * returns the extension(s) it needs to do that which we can pass to the struct.
         * The last two members of the struct determine the global validation layers to enable.
         */
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
        createInfo.enabledLayerCount = 0;

        /*
         * Return the required list of extensions based on whether validation layers are enabled or not
         */
        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

        /*
         * Include validation layer names in VkInstanceCreateInfo struct if enabled. Additionally, populate
         * debugCreateInfo with layer information
         */
        if (enableValidationLayers) {
            /*
             * static_cast performs explicit type conversions between compatible types at compile time.
             */
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }

        /*
         * Checker to see of vkCreateInstance is successful
         */
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    /**
     * Populates a createInfo struct with details about the messenger and its callback
     * @param createInfo is the struct to fill in
     */
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
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
    void setupDebugMessenger() {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    /**
     * Creates a window surface using GLFW library in Vulkan, associated with the provided window and Vulkan instance
     * . If the creation of the surface fails, an exception is thrown with an error message indicating the failure.
     */
    void createSurface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    /**
     * selects a suitable physical device (GPU) for Vulkan rendering by querying the available devices and checking
     * if they meet certain criteria. If a suitable device is found, it is assigned to the variable physicalDevice;
     * otherwise, an exception is thrown indicating the failure to find a suitable GPU.
     */
    void pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        /*
        * Query all devices retrieved and see if the device is suitable
        */
        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    /**
     * Creates a logical device in Vulkan with a single queue for the graphics family specified by the
     * QueueFamilyIndices structure. The code sets up the necessary parameters, including queue priority, device features, and validation layers if enabled. If the logical device creation is successful, it retrieves the handle for the graphics queue.
     */
    void createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        /*
        * Create a vector of queues that stores VkDeviceQueueCreateInfos. This structure describes number of queues we
         * want for a single queue family
        */
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
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

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        /*
        * Instantiating logical device
        */
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
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
    void createSwapChain() {
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
        .maxImageCount) {
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

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
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
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
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
    void createImageViews() {
        swapChainImageViews.resize(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); i++) {
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

            if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image views!");
            }
        }
    }

    /**
     * Chooses the preferred surface format for rendering graphics on a Vulkan surface. It tries to find a surface
     * format that has a specific format and color space, and if found, returns that format.
     * @param availableFormats is a vector of all possible surface formats
     * @return the first surface format that matches the format and colorspace
     */
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace ==
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    /**
     * Chooses the preferred presentation mode for displaying images on a Vulkan surface.
     * There are four possible modes available in Vulkan:
     *      VK_PRESENT_MODE_IMMEDIATE_KHR: Images submitted by your application are transferred to the screen right
     *      away, which may result in tearing.
     *      VK_PRESENT_MODE_FIFO_KHR: The swap chain is a queue where the display takes an image from the front of
     *      the queue when the display is refreshed and the program inserts rendered images at the back of the queue.
     *      If the queue is full then the program has to wait. This is most similar to vertical sync as found in
     *      modern games. The moment that the display is refreshed is known as "vertical blank".
     *      VK_PRESENT_MODE_FIFO_RELAXED_KHR: This mode only differs from the previous one if the application is
     *      late and the queue was empty at the last vertical blank. Instead of waiting for the next vertical blank,
     *      the image is transferred right away when it finally arrives. This may result in visible tearing.
     *      VK_PRESENT_MODE_MAILBOX_KHR: This is another variation of the second mode. Instead of blocking the
     *      application when the queue is full, the images that are already queued are simply replaced with the
     *      newer ones. This mode can be used to render frames as fast as possible while still avoiding tearing,
     *      resulting in fewer latency issues than standard vertical sync. This is commonly known as "triple
     *      buffering", although the existence of three buffers alone does not necessarily mean that the framerate is
     *      unlocked.
     * @param availablePresentModes is a vector of all possible presentation modes
     * @return the preferred mailbox mode
     */
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
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
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
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
                    static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities
            .maxImageExtent.width);
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
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
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
    bool isDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices = findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
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
    bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
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
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        /*
         * We need to find at least one queue family that supports VK_QUEUE_GRAPHICS_BIT
         */
        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) {
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
    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        /*
         * VK_EXT_DEBUG_UTILS_EXTENSION_NAME macro here which is equal to the literal string "VK_EXT_debug_utils".
         * Using this macro lets you avoid typos.
         */
        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    /**
     * @return true or false depending on if all the requested layers are available
     */
    bool checkValidationLayerSupport() {
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
        for (const char *layerName: validationLayers) {
            bool layerFound = false;

            for (const auto &layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
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
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }
};

/**
 * Main code that is compiled and run
 * @return exit code
 */
int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}