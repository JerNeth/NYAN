module;

//#include <expected>

export module NYANRenderer:Graph; 
import std;
import NYANVulkan;
import :Error;

export namespace nyan::renderer
{
	class Node
	{
		//Either Pass or Ressource
	};

	class Pass : Node
	{

	};

	class Ressource : Node
	{

	};

	class Graph
	{
	public:

		void reads(Pass pass, Ressource ressource) noexcept;
		void writes(Pass pass, Ressource ressource) noexcept;
		[[nodiscard("must handle potential error")]] static std::expected<Graph, GraphError> create() noexcept;
	private:
		Graph() noexcept;
	};


}
