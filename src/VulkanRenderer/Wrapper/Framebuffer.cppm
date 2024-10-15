module;

//#include <bit>
//#include <expected>
//#include <span>

#include "magic_enum.hpp"

#include "volk.h"

export module NYANVulkan:Framebuffer;
import std;
import NYANData;
import :DeletionQueue;
import :Error;
import :Formats;
import :ImageView;
import :Object;

export namespace nyan::vulkan
{
	class LogicalDevice;
	class RenderPass;
	class Framebuffer : public Object<VkFramebuffer>
	{
	public:
		struct Params {
			RenderPass& renderPass;
			nyan::StaticVector<ImageView, 128> attachments;
			uint16_t width;
			uint16_t height;
		};
	private:
		struct Data {
			uint16_t width;
			uint16_t height;
		};
	public:
		Framebuffer(Framebuffer&) = delete;
		Framebuffer(Framebuffer&& other) noexcept;

		Framebuffer& operator=(Framebuffer&) = delete;
		Framebuffer& operator=(Framebuffer&& other) noexcept;


		~Framebuffer() noexcept; 
		[[nodiscard]] static std::expected<Framebuffer, Error> create(LogicalDevice& device, const Params& params) noexcept;
		[[nodiscard]] uint16_t get_width() const noexcept;
		[[nodiscard]] uint16_t get_height() const noexcept;
	protected:
		Framebuffer(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, VkFramebuffer handle, Data data) noexcept;

		Data m_data;
	private:
		DeletionQueue& r_deletionQueue;
	};
	class ImagelessFramebuffer : public Framebuffer
	{
	public:
		[[nodiscard]] static std::expected<ImagelessFramebuffer, Error> create(LogicalDevice& device) noexcept;
	private:
		ImagelessFramebuffer(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, VkFramebuffer handle) noexcept;
	};
}