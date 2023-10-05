#include "Renderer/RendererError.hpp"

renderer::Error::Error(Type type, const std::source_location& location) noexcept :
	m_type (type)
{
}
