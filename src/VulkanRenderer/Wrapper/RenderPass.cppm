module;

//#include <array>
//#include <bit>
//#include <expected>
//#include <span>

#include "magic_enum.hpp"

#include "volk.h"

export module NYANVulkan:RenderPass;
import std;

import NYANData;

import :Common;
import :DeletionQueue;
import :Image;
import :Error;
import :Object;

export namespace nyan::vulkan
{
	class LogicalDevice;
	class RenderPass : public Object<VkRenderPass>
	{
	protected:
		struct Data {
			nyan::StaticVector<uint32_t, 16> numColorAttachments;
			nyan::StaticVector<ClearValue, 128> clearValues;
		};
	public:
		[[nodiscard]] static std::expected<RenderPass, Error> create(LogicalDevice& device) noexcept;

		RenderPass(RenderPass&) = delete;
		RenderPass(RenderPass&& other) noexcept;

		RenderPass& operator=(RenderPass& other) = delete;
		RenderPass& operator=(RenderPass&& other) noexcept;

		~RenderPass() noexcept;
		[[nodiscard]] uint32_t get_num_color_attachments(uint32_t subpass) const noexcept;
		[[nodiscard]] std::span<const ClearValue> get_clear_values() const noexcept;
	protected:
		RenderPass(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, VkRenderPass handle, Data data) noexcept;

		DeletionQueue& r_deletionQueue;
		Data m_data;
	};
	class SingleRenderPass : public RenderPass
	{
	public:
		struct Attachment {
			LoadOp load {LoadOp::Clear};
			StoreOp store {StoreOp::Store};
			Format format;
			Samples samples{Samples::C1};
			Layout initialLayout{Layout::Undefined};
			Layout finalLayout{Layout::Undefined};
			ClearValue clearValue{};
		};
		struct Params {
			nyan::StaticVector<Attachment, maxOutputAttachments + 1> attachments; // Include depthStencil	
			//enum class DepthMode : uint8_t {
			//	Attachment,
			//	Read,
			//} depthStencil;
		};
	public:
		[[nodiscard]] static std::expected<SingleRenderPass, Error> create(LogicalDevice& device, Params params) noexcept;

	private:
		SingleRenderPass(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, VkRenderPass handle, Data data) noexcept;
	};
}