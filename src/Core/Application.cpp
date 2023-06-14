#include "Application.h"
#include "VulkanWrapper/Surface.hpp"
#include "Utility/Exceptions.h"
#include <chrono>
nyan::Application::Application(std::string name): m_name(std::move(name)) , m_settings("general.ini") 
{
	vulkan::Instance::Validation validation{
		.enabled {false},
		.createCallback {false},
		//.enabled {debug},
		//.createCallback {debug},
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

Application::~Application()
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
		for (const auto& monitor : monitors) {
			if (monitor.get_identifier() == monitorName) {
				m_monitor = std::make_unique<glfww::Monitor>(monitor);
				break;
			}
		}
		auto width = m_settings.get_or_default(Settings::Setting::Width, 1920);
		auto height = m_settings.get_or_default(Settings::Setting::Height, 1080);
		glfww::WindowMode windowMode = m_settings.get_or_default(Settings::Setting::WindowMode, glfww::WindowMode::Windowed);
		m_monitor = std::make_unique<glfww::Monitor>();

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

		std::vector optionalExtensions{
			//VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME, //Don't actually want that, it's vor HMDs, exculsive display access
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
		m_vulkanDevice = m_vulkanInstance->setup_device(requiredExtensions, optionalExtensions);
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
		#ifdef _WIN32
		m_vulkanInstance->setup_win32_surface(m_window->get_win32_window(), GetModuleHandle(nullptr));
		#elif X_PROTOCOL
		m_vulkanInstance->setup_x11_surface(m_window->get_x11_window(),m_window->get_x11_display());
		#endif
	}
	catch (const Utility::VulkanException& error) {
		Utility::log_error(error.what());
		return false;
	}
	return true;
}
