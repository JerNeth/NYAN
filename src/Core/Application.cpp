#include "Application.h"
#include "VulkanWrapper/Surface.hpp"
#include "VulkanWrapper/PhysicalDevice.hpp"
#include "Utility/Exceptions.h"
#include <chrono>
#include <codecvt>

nyan::Application::Application(std::string name):
	m_name(std::move(name)),
	m_settings(std::filesystem::current_path() / std::string("general.ini")) 
{
	vulkan::Instance::Validation validation{
		//.enabled {false},
		//.createCallback {false},
		.enabled {debug},
		.createCallback {debug},
		.callBackVerbose { true },
		.callBackInfo { true },
		.callBackWarning { true },
		.callBackError { true },
		.callBackGeneral { true },
		.callBackValidation { true },
		.callBackPerformance { true },
		.callBackDeviceAddressBinding { true },
		.gpuAssisted { false },
		.gpuAssistedReserveBindingSlot { false },
		.bestPractices { false },
		.debugPrintf { false },
		.synchronizationValidation { false },
		.disableAll { false },
		.disableShaders { false },
		.disableThreadSafety { false },
		.disableAPIParameters { false },
		.disableObjectLifetimes { false },
		.disableCoreChecks { false },
		.disableUniqueHandles { false },
		.disableShaderValidationCache { false },
	};
	if (!setup_glfw())
		throw InitializationError("Could not initialize GLFW");
	if (!setup_vulkan_instance(validation))//OpenGL fallback maybe?
		throw InitializationError("Could not instantiate Vulkan, maybe no vulkan support or outdated drivers?");
	if (!setup_glfw_window())
		throw InitializationError("Could not create GLFW window");
	if (!setup_glfw_input())
		throw InitializationError("Could not create GLFW window");
	if (!setup_vulkan_surface())
		throw InitializationError("Could not create Vulkan Surface");
	if (!setup_vulkan_device())//OpenGL fallback maybe?
		throw InitializationError("Could not create Vulkan Device, maybe update driver?");
}

nyan::Application::~Application()
{
}

vulkan::LogicalDevice& nyan::Application::get_device()
{
	assert(m_vulkanDevice);
	return *m_vulkanDevice;
}

glfww::Window& nyan::Application::get_window()
{
	assert(m_window);
	return *m_window;
}

nyan::Input& nyan::Application::get_input()
{
	assert(m_input);
	return *m_input;
}

void nyan::Application::next_frame()
{
	m_windowSystemInterface->begin_frame();
	for (const auto& beginFrameFunction : m_beginFrameFunctions) {
		beginFrameFunction();
	}
}

void nyan::Application::end_frame()
{
	for (const auto& endFrameFunction : m_endFrameFunctions) {
		endFrameFunction();
	}
	m_windowSystemInterface->end_frame();
	m_frameCount++;
}

void nyan::Application::main_loop()
{
	m_lastUpdate = std::chrono::steady_clock::now();
	while (!m_window->should_close() && (!m_maxFrameCount || (m_frameCount < m_maxFrameCount)))
	{
		update();
		if (!m_window->is_iconified()) {

			next_frame();


			end_frame();
		}
	}
}

void nyan::Application::each_frame_begin(std::function<void()> beginFrame)
{
	m_beginFrameFunctions.push_back(beginFrame);
}

void nyan::Application::each_frame_end(std::function<void()> endFrame)
{
	m_endFrameFunctions.push_back(endFrame);
}

void nyan::Application::each_update(std::function<void(std::chrono::nanoseconds)> update)
{
	m_updateFunctions.push_back(update);
}

void nyan::Application::update()
{
	auto now = std::chrono::steady_clock::now();
	auto delta = now - m_lastUpdate;
	m_lastUpdate = now;
	glfwPollEvents();
	m_input->update();
	
	for (const auto& update : m_updateFunctions) {
		update(delta);
	}
	m_tickCount++;
}

void nyan::Application::quit_after(uint64_t numFrames)
{
	m_maxFrameCount = numFrames;
}

bool nyan::Application::setup_glfw()
{
	try {
		m_glfwLibrary = std::make_unique<glfww::Library>();
	}
	catch (const std::runtime_error& error) {
		std::cerr << error.what() << std::endl;
		return false;
	}
	return true;
}

bool nyan::Application::setup_glfw_window()
{
	try {
		auto monitors = glfww::Monitor::get_monitors();
		auto monitorName = m_settings.get_or_default<std::string>(Settings::Setting::Monitor, "");
		//const auto monitorIdx = m_settings.get_or_default<size_t>(Settings::Setting::Monitor, 0ull);
		for (const auto& monitor : monitors) {
			if (monitor.get_identifier() == monitorName) {
				m_monitor = std::make_unique<glfww::Monitor>(monitor);
				break;
			}
		}
		if(!m_monitor)
			m_monitor = std::make_unique<glfww::Monitor>();
		//if(monitorIdx < monitors.size())
		//	m_monitor = std::make_unique<glfww::Monitor>(monitors[monitorIdx]);
		//else
		//	m_monitor = std::make_unique<glfww::Monitor>();
		auto width = m_settings.get_or_default(Settings::Setting::Width, 1920);
		auto height = m_settings.get_or_default(Settings::Setting::Height, 1080);
		glfww::WindowMode windowMode = m_settings.get_or_default(Settings::Setting::WindowMode, glfww::WindowMode::Windowed);

		m_window = std::make_unique<glfww::Window>(width, height, *m_monitor, windowMode, m_name);
	}
	catch (const std::runtime_error& error) {
		std::cerr << error.what() << std::endl;
		return false;
	}
	return true;
}
bool nyan::Application::setup_glfw_input()
{
	try {
		m_input = std::make_unique<nyan::Input>(*m_window);
	}
	catch (const std::runtime_error& error) {
		std::cerr << error.what() << std::endl;
		return false;
	}
	return true;
}

bool nyan::Application::setup_vulkan_instance(const vulkan::Instance::Validation& validation)
{
	try {

		auto requiredExtensions = m_glfwLibrary->get_required_extensions();
		//VK_KHR_SURFACE_EXTENSION_NAME;

		//VK_KHR_WIN32_SURFACE_EXTENSION_NAME;  //VK_USE_PLATFORM_WIN32_KHR
		//VK_KHR_XLIB_SURFACE_EXTENSION_NAME;  //VK_USE_PLATFORM_XLIB_KHR
		//VK_KHR_XCB_SURFACE_EXTENSION_NAME;  //VK_USE_PLATFORM_XCB_KHR
		//VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;  //VK_USE_PLATFORM_WAYLAND_KHR
		//VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;  //VK_USE_PLATFORM_ANDROID_KHR
		//VK_EXT_DIRECTFB_SURFACE_EXTENSION_NAME;  //VK_USE_PLATFORM_DIRECTFB_EXT
		//VK_EXT_ACQUIRE_XLIB_DISPLAY_EXTENSION_NAME;  //VK_USE_PLATFORM_XLIB_XRANDR_EXT
		//VK_MVK_IOS_SURFACE_EXTENSION_NAME;  //VK_USE_PLATFORM_IOS_MVK
		//VK_MVK_MACOS_SURACE_EXTENSION_NAME;  //VK_USE_PLATFORM_MACOS_MVK
		//VK_EXT_METAL_SURFACE_EXTENSION_NAME; //VK_USE_PLATFORM_METAL_EXT

		std::vector optionalExtensions{
			//VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME, //Don't actually want that, it's for HMDs, exculsive display access
			//VK_KHR_DISPLAY_EXTENSION_NAME, //Poor support on windows
			//VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME, //Depends on KHR_DISPLAY
			VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
			//VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME, //Not available yet
			//VK_KHR_get_physical_device_properties2, //1.1 core
			VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME
		};
		if (validation.enabled) {
			optionalExtensions.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
			optionalExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		m_vulkanInstance = std::make_unique<vulkan::Instance>(validation, requiredExtensions, optionalExtensions, m_name, m_engineName);
	}
	catch (const Utility::VulkanException& error) {
		Utility::log_error(error.what());
		return false;
	}
	return true;
}

bool nyan::Application::setup_vulkan_device()
{
	try {
		std::vector requiredExtensions {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		};
		std::vector optionalExtensions{
			VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
			VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
			VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME,
			//VK_EXT_DEBUG_MARKER_EXTENSION_NAME, //Deprecated by debug utils
			VK_KHR_RAY_QUERY_EXTENSION_NAME,
			VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
			VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
			//VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME, //Crashes nsight captures for some reason
			//VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, 1.2 Core
			//VK_KHR_MAINTENANCE_4_EXTENSION_NAME, 1.3 Core
			//VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME, Not widely supported idk
			VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME,
			//VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME, //barely supported on desktop, only some linux configs
			//VK_KHR_FRAGMENT_SHADER_BARYCENTRIC_EXTENSION_NAME, //TODO
			//VK_EXT_SWAPCHAIN_MAINTENANCE_1_EXTENSION_NAME, //Not really supported on windows, with nvidia hardware
			VK_KHR_PRESENT_ID_EXTENSION_NAME,
			//VK_KHR_PRESENT_WAIT_EXTENSION_NAME, //Device creation stalling with this extension for some reason.
		};
		const auto deviceId = m_settings.get_value<size_t>(Settings::Setting::DeviceIdx);
		size_t deviceIdx = 0;
		auto physicalDevices = m_vulkanInstance->get_physical_devices();
		std::vector<size_t> validDevices;
		for (size_t idx = 0; idx < physicalDevices.size(); ++idx) {
			auto& dev = physicalDevices[idx];
			const auto& prop = dev.get_properties();
			const auto id = (static_cast<uint64_t>(prop.vendorID) << 32ull) | prop.deviceID;
			bool required = true;
			for (const auto& extension : requiredExtensions)
				required &= dev.use_extension(extension);
			if (!required)
				continue;
			//uint32_t optional = 0;
			for (const auto& extension : optionalExtensions)
				dev.use_extension(extension);
				//optional += dev.use_extension(extension);
			validDevices.push_back(idx);
			if (deviceId && id == *deviceId) {
				deviceIdx = validDevices.size() - 1;
				break;
			}
		}
		if (validDevices.empty())
			return false;
		auto& physicalDevice = physicalDevices[validDevices[deviceIdx]];
		const auto& prop = physicalDevice.get_properties();
		m_settings.set_value(Settings::Setting::DeviceIdx, (static_cast<uint64_t>(prop.vendorID) << 32ull) | prop.deviceID);
		
		auto init_priorities = [](auto n, auto numPriorities)
		{
			std::vector<float> priorities(n);
			for (size_t idx = 0; idx < priorities.size(); ++idx)
				priorities[idx] = 1.f - (idx / Math::max(numPriorities, 1.f));
			return priorities;
		};
		const auto discreteQueuePriorities = prop.limits.discreteQueuePriorities;
		const auto genericQueuePriorities = init_priorities(1, discreteQueuePriorities);
		const auto computeQueuePriorities = init_priorities(1, discreteQueuePriorities);
		const auto transferQueuePriorities = init_priorities(1, discreteQueuePriorities);
		//m_vulkanDevice = physicalDevice.create_logical_device(*m_vulkanInstance, genericQueuePriorities, computeQueuePriorities, transferQueuePriorities);
		constexpr uint32_t a = VK_API_VERSION_1_3; 
		constexpr uint32_t b = VK_VERSION_MAJOR(4206828);
		constexpr uint32_t c = VK_VERSION_MINOR(4206828);
		constexpr uint32_t d = VK_VERSION_PATCH(4206828);
		m_vulkanDevice = physicalDevice.create_logical_device(*m_vulkanInstance);
		m_windowSystemInterface = std::make_unique<vulkan::WindowSystemInterface>(*m_vulkanDevice, *m_vulkanInstance);
		m_vulkanDevice->create_pipeline_cache("pipeline.cache");
	}
	catch (const Utility::VulkanException& error) {
		Utility::log_error(error.what());
		return false;
	}
	return true;
}

bool nyan::Application::setup_vulkan_surface()
{
	try {
		#ifdef WIN32
		m_vulkanInstance->setup_win32_surface(m_window->get_win32_window(), GetModuleHandle(nullptr));
		#elif X_PROTOCOL
		m_vulkanInstance->setup_x11_surface(m_window->get_x11_window(),m_window->get_x11_display());
		#endif
		//assert(m_vulkanDevice);
		//for(uint32_t queueType = 0; queueType < static_cast<uint32_t>(vulkan::Queue::Type::Size); ++queueType)
		//{
		//	for(auto& queue : m_vulkanDevice->get_queues(static_cast<vulkan::Queue::Type>(queueType)))
		//	{
		//		queue.set_present_capable(m_vulkanDevice->get_physical_device().supports_surface(m_vulkanInstance->get_surface(), queue.get_family_index()));
		//	}
		//}
	}
	catch (const Utility::VulkanException& error) {
		Utility::log_error(error.what());
		return false;
	}
	return true;
}
