#include "Application.h"
#include <chrono>
nyan::Application::Application(const std::string& name): m_name(name) , m_settings("general.ini") 
{
	OPTICK_EVENT("")
	if (!setup_glfw())
		throw InitializationError("Could not initialize GLFW");
	if (!setup_vulkan_instance())//OpenGL fallback maybe?
		throw InitializationError("Could not instantiate Vulkan, maybe no vulkan support or outdated drivers?");
	if (!setup_glfw_window())
		throw InitializationError("Could not create GLFW window");
	if (!setup_vulkan_surface())
		throw InitializationError("Could not create Vulkan Surface");
	if (!setup_vulkan_device())//OpenGL fallback maybe?
		throw InitializationError("Could not create Vulkan Device, maybe update driver?");
	
}

vulkan::LogicalDevice& nyan::Application::get_device()
{
	return *m_vulkanDevice;
}

glfww::Window& nyan::Application::get_window()
{
	return *m_window;
}

int nyan::Application::get_width()
{
	return m_vulkanDevice->get_swapchain_width();
}

int nyan::Application::get_height()
{
	return m_vulkanDevice->get_swapchain_height();
}

void nyan::Application::next_frame()
{
	m_windowSystemInterface->begin_frame();
	for (auto* renderer : m_renderer) {
		renderer->next_frame();
	}
}

void nyan::Application::end_frame()
{
	for (auto renderer = m_renderer.rbegin(); renderer != m_renderer.rend(); renderer++) {
		(*renderer)->end_frame();
	}
	m_windowSystemInterface->end_frame();
}

void nyan::Application::add_renderer(nyan::Renderer* renderer)
{
	if(const auto& res = std::find(m_renderer.cbegin(), m_renderer.cend(), renderer); res == m_renderer.cend())
		m_renderer.push_back(renderer);
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
		auto monitors = glfww::get_monitors();
		auto monitorName = m_settings.get_or_default<std::string>(Settings::Setting::Monitor, "");
		for (const auto& monitor : monitors) {
			if (monitor.get_identifier() == monitorName) {
				m_monitor = std::make_unique<glfww::Monitor>(monitor);
				break;
			}
		}
		auto width = m_settings.get_or_default(Settings::Setting::Width, 1920);
		auto height = m_settings.get_or_default(Settings::Setting::Height, 1080);
		glfww::WindowMode windowMode = m_settings.get_or_default(Settings::Setting::WindowMode,glfww::WindowMode::Windowed);
		m_monitor = std::make_unique<glfww::Monitor>();
		m_window = std::make_unique<glfww::Window>(width, height, *m_monitor, windowMode, m_name);
	}
	catch (const std::runtime_error& error) {
		std::cerr << error.what() << std::endl;
		return false;
	}
	return true;
}
bool nyan::Application::setup_vulkan_instance()
{
	try {
		auto instanceExtensions = m_glfwLibrary->get_required_extensions();
		m_vulkanInstance = std::make_unique<vulkan::Instance>(instanceExtensions.data(), static_cast<uint32_t>(instanceExtensions.size()), m_name, m_engineName);
	}
	catch (const std::runtime_error& error) {
		std::cerr << error.what() << std::endl;
		return false;
	}
	return true;
}

bool nyan::Application::setup_vulkan_device()
{
	try {
		std::vector<const char*> requiredExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		};
		std::vector<const char*> optionalExtensions = {
			VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
			VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
			VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME,
			VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
			VK_KHR_RAY_QUERY_EXTENSION_NAME,
			VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
			VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
			VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME
		};
		m_vulkanDevice = m_vulkanInstance->setup_device(requiredExtensions, optionalExtensions);
		m_windowSystemInterface = std::make_unique<vulkan::WindowSystemInterface>(*m_vulkanDevice, *m_vulkanInstance);
	}
	catch (const std::runtime_error& error) {
		std::cerr << error.what() << std::endl;
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
	catch (const std::runtime_error& error) {
		std::cerr << error.what() << std::endl;
		return false;
	}
	return true;
}
