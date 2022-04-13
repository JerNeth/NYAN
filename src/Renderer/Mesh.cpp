#include "Renderer/Mesh.h"

//void nyan::StaticMesh::render(vulkan::CommandBufferHandle& cmd)
//{
//	assert(vertexBuffer);
//	assert(indexBuffer);
//	//cmd->bind_index_buffer(vulkan::IndexState{ .buffer = indexBuffer->get_handle(), .offset = indexOffset,.indexType = indexType });
//	//cmd->set_vertex_attribute(0, 0, vulkan::get_format<Math::vec3>());
//	//cmd->set_vertex_attribute(1, 0, vulkan::get_format<Math::vec2>());
//	//cmd->set_vertex_attribute(2, 0, vulkan::get_format<Math::ubvec4>());
//	//if (tangentSpace) {
//	//	cmd->set_vertex_attribute(3, 0, vulkan::get_format<Math::bvec4>());
//	//	cmd->set_vertex_attribute(4, 0, vulkan::get_format<Math::bvec4>());
//	//}
//	//cmd->bind_vertex_buffer(0, *vertexBuffer, vertexOffset, VK_VERTEX_INPUT_RATE_VERTEX);
//	//cmd->push_constants(&transform.transform, 0, sizeof(Transform));
//	//cmd->draw_indexed(indexCount, 1, 0, 0, 0);
//}
//
//void nyan::SkinnedMesh::render(vulkan::CommandBufferHandle& cmd)
//{
//	assert(vertexBuffer);
//	assert(indexBuffer);
//	assert(skeleton);
//	//skeleton->bind(cmd);
//	//cmd->bind_index_buffer(vulkan::IndexState{ .buffer = indexBuffer->get_handle(), .offset = indexOffset,.indexType = indexType });
//	//cmd->set_vertex_attribute(0, 0, vulkan::get_format<Math::vec3>());
//	//cmd->set_vertex_attribute(1, 0, vulkan::get_format<Math::vec2>());
//	//cmd->set_vertex_attribute(2, 0, VK_FORMAT_R8G8B8A8_UINT);
//	//cmd->set_vertex_attribute(3, 0, vulkan::get_format<Math::usvec4>());
//	//if (tangentSpace) {
//	//	cmd->set_vertex_attribute(4, 0, vulkan::get_format<Math::bvec4>());
//	//	cmd->set_vertex_attribute(5, 0, vulkan::get_format<Math::bvec4>());
//	//}
//	//if (blendshapeCount != 0) {
//	//	assert(blendshape);
//	//	cmd->set_vertex_attribute(7, 1, vulkan::get_format<Math::ubvec3>());
//	//	cmd->bind_vertex_buffer(1, *blendshape->vertexBuffer, blendshape->vertexOffset, VK_VERTEX_INPUT_RATE_VERTEX);
//	//	//TODO bind ubo/push constant for blendweight
//	//}
//	//cmd->bind_vertex_buffer(0, *vertexBuffer, vertexOffset, VK_VERTEX_INPUT_RATE_VERTEX);
//	//cmd->push_constants(&transform.transform, 0, sizeof(Transform));
//	//cmd->draw_indexed(indexCount, 1, 0, 0, 0);
//}