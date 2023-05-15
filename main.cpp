/**
 * Vulkan header from the LunarG SDK, provides the functions, structures, and
 * enumerations.
 */
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <optional>

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

    bool isComplete() {
        return graphicsFamily.has_value();
    }
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

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device; // logical device

    VkQueue graphicsQueue;

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
        pickPhysicalDevice();
        createLogicalDevice();
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
        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

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
          * This structure describes number of queues we want for a single queue family
          */
         VkDeviceQueueCreateInfo queueCreateInfo{};
         queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
         queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
         queueCreateInfo.queueCount = 1;

         float queuePriority = 1.0f;
         queueCreateInfo.pQueuePriorities = &queuePriority;

         VkPhysicalDeviceFeatures deviceFeatures{};

         /*
          * Logical device info struct
          */
         VkDeviceCreateInfo createInfo{};
         createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

         createInfo.pQueueCreateInfos = &queueCreateInfo;
         createInfo.queueCreateInfoCount = 1;

         createInfo.pEnabledFeatures = &deviceFeatures;

         createInfo.enabledExtensionCount = 0;

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
          * Retrieve queue handles for each queue family
          */
         vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
     }

     /**
      * Checks if a given device is suitable to be used as GPU
      * @param device is a VkPhysicalDevice object
      * @return boolean of if the given device has the support of the queue families
      */
     bool isDeviceSuitable(VkPhysicalDevice device) {
         QueueFamilyIndices indices = findQueueFamilies(device);

         return indices.isComplete();
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

            for (const auto &layerProperties: availableLayers) {
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