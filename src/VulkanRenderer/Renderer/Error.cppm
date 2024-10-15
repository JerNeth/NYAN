module;


export module NYANRenderer:Error;


export namespace nyan::renderer
{
	class Error
	{
	public:

	private:
		Error() noexcept;
	};
	class ResourceError
	{
	public:
		enum class Type
		{
			InstanceCreationError,
			NoValidDevices,
			LogicalDeviceCreationError,
			UnknownError
		};
	public:
		ResourceError(Type type) noexcept;
	private:
	};
	class WSIError
	{
	public:
		enum class Type
		{
			SurfaceLost,
			UnknownError
		};
	public:
		WSIError() noexcept {};
		WSIError(Type type) noexcept;
	private:
	};
	class GraphError
	{
	public:
		enum class Type
		{
			UnknownError
		};
	public:
		GraphError() noexcept {};
		GraphError(Type type) noexcept;
	private:
	};

	class SettingsError
	{
	public:
		enum class Type
		{
			UnknownError
		};
	public:
		SettingsError() noexcept {};
		SettingsError(Type type) noexcept;
	private:
	};

	WSIError::WSIError(Type type) noexcept
	{
	}

	GraphError::GraphError(Type type) noexcept
	{
	}
	ResourceError::ResourceError(Type type) noexcept
	{
	}
}
