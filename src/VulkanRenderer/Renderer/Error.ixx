module;


export module NYANVulkanRenderer:Error;


export namespace nyan::vulkan::renderer
{
	class Error
	{
	public:

	private:
		Error() noexcept;
	};
	class RendererCreationError
	{
	public:
		enum class Type
		{
			InstanceCreationError,
			NoValidDevices,
			UnknownError
		};
	public:
		RendererCreationError(Type type) noexcept;
	private:
	};
}
