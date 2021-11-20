#include "Utility/FBXReader.h"
#include "Renderer/Mesh.h"

Utility::FBXReader::FBXReader()
{
	sdkManager = FbxManager::Create();

	ios = FbxIOSettings::Create(sdkManager, IOSROOT);
	sdkManager->SetIOSettings(ios);
}

Utility::FBXReader::~FBXReader()
{
	ios->Destroy();
	sdkManager->Destroy();
}

std::vector<nyan::MeshData> Utility::FBXReader::parse_meshes(std::string fbxFile, bool tangentSpace)
{
	fbxsdk::FbxImporter* importer = FbxImporter::Create(sdkManager, "");

	if (!importer->Initialize(fbxFile.c_str(), -1, sdkManager->GetIOSettings()))
		return {};

	fbxsdk::FbxScene* scene = ::FbxScene::Create(sdkManager, "ImportScene");

	importer->Import(scene);

	importer->Destroy();
	fbxsdk::FbxNode* rootNode = scene->GetRootNode();

	//std::cout << rootNode->GetChildCount() << '\n';
	std::vector<nyan::MeshData> retVec;
	for (int child = 0; child < rootNode->GetChildCount(); child++) {
		nyan::MeshData ret;
		auto node = rootNode->GetChild(child);
		auto mesh = node->GetMesh();
		ret.name = node->GetName();

		//for (int i = 0; i < node->GetMaterialCount(); i++) {
		//	fbxsdk::FbxSurfaceMaterial* mat= node->GetMaterial(i);
		//	std::cout<< mat->GetName() << ' ';
		//	auto s = mat->ShadingModel.Get();
		//	assert(s == "Phong");
		//	auto phong = reinterpret_cast<fbxsdk::FbxSurfacePhong*>(mat);
		//	std::cout << phong->ShadingModel.Get() << ' ';
		//	//std::cout << phong->Diffuse. << '\n';
		//}

		fbxsdk::FbxVector4* lControlPoints = mesh->GetControlPoints();
		auto* findices = mesh->GetPolygonVertices();



		
		auto lambda = [&]<typename V, typename T>(std::vector<V>& vertices, T & indices) {
			constexpr bool usesTangentSpace = std::is_same_v<V, nyan::SkinnedMesh::TangentVertex> || std::is_same_v < V, nyan::StaticMesh::TangentVertex>;

			vertices.reserve(mesh->GetControlPointsCount());
			for (int i = 0; i < mesh->GetControlPointsCount(); i++) {
				V vertex{};
				vertex.uv.x() = 0;
				vertex.uv.y() = 0;
				for (int j = 0; j < 3; j++)
					vertex.pos[j] = lControlPoints[i].mData[j];
				vertices.push_back(vertex);
			}
			std::vector <Math::vec3> normals{};
			std::vector <Math::vec3> tangents{};
			if constexpr (usesTangentSpace) {
				normals.resize(vertices.size());
				tangents.resize(vertices.size());
			}
			indices.reserve(mesh->GetPolygonVertexCount());
			std::unordered_map<HashValue, uint32_t> vertexIndex;
			std::unordered_multimap<uint32_t, uint32_t> splitVertices;
			
			vertexIndex.reserve(vertices.size());
			fbxsdk::FbxLayerElementArrayTemplate<fbxsdk::FbxVector2>* uvs;
			mesh->GetTextureUV(&uvs);
			for (int i = 0; i < mesh->GetPolygonCount(); i++) {
				auto polyIndex = mesh->GetPolygonVertexIndex(i);
				int k = mesh->GetPolygonSize(i);
				std::vector<uint32_t> tmpIndices;
				std::vector<uint32_t> uvIndices;
				uvIndices.reserve(k);
				tmpIndices.reserve(k);
				for (int j = 0; j < k; j++) {
					auto uvIdx = mesh->GetTextureUVIndex(i, j);
					uvIndices.push_back(uvIdx);
					auto uv = uvs->operator[](uvIdx);
					auto index = findices[polyIndex + j];
					auto& vertex = vertices[index];
					float tarX = uv[0];// *UINT16_MAX;
					float tarY = (1.0f - uv[1]);// *UINT16_MAX;
					Hasher h;
					h(tarX);
					h(tarY);
					h(vertex.pos);
					if (vertexIndex.contains(h())) {
						tmpIndices.push_back(vertexIndex[h()]);
					}
					else {
						if (0 == vertex.uv.x() && 0 == vertex.uv.y()) {
							vertexIndex.emplace(h(), index);
							tmpIndices.push_back(index);
							vertex.uv.x() = tarX;
							vertex.uv.y() = tarY;
							if constexpr (usesTangentSpace)
								splitVertices.emplace(index, index);
						}
						else {
							auto newIndex = vertices.size();
							vertexIndex.emplace(h(), newIndex);
							if constexpr (usesTangentSpace)
								splitVertices.emplace(index, newIndex);
							tmpIndices.push_back(newIndex);
							V newVertex = vertex;
							newVertex.uv.x() = tarX;
							newVertex.uv.y() = tarY;
							vertices.push_back(newVertex);
						}
					}
				}
				if constexpr (usesTangentSpace) {
					for (int j = 0; j < k - 3; j++) {
						auto uv0 = uvs->operator[](uvIndices[j + 0]);
						auto uv1 = uvs->operator[](uvIndices[(j + 1) % k]);
						auto uv2 = uvs->operator[](uvIndices[(j + 2) % k]);
						auto& v0 = vertices[tmpIndices[j + 0]];
						auto& v1 = vertices[tmpIndices[(j + 1) % k]];
						auto& v2 = vertices[tmpIndices[(j + 2) % k]];
						auto q1 = v1.pos - v0.pos;
						auto q2 = v2.pos - v0.pos;
						float s1 = -uv0[0];
						float s2 = s1;
						s1 += uv1[0];
						s2 += uv2[0];
						//float t1 = -(1.0f - uv0[1]);
						//float t2 = t1;
						//t1 += (1.0f - uv1[1]);
						//t2 += (1.0f - uv2[1]);
						float t1 = -(uv0[1]);
						float t2 = t1;
						t1 += (uv1[1]);
						t2 += (uv2[1]);

						auto TS = 1.0f / (s1 * t2 - s2 * t1) * Math::mat22(t2, -t1, -s2, s1) * Math::mat32(q1.x(), q1.y(), q1.z(), q2.x(), q2.y(), q2.z());

						auto normal = TS.row(0).cross(TS.row(1));
						normals[findices[polyIndex + j + 0]] += normal;
						normals[findices[polyIndex + (j + 1) % k]] += normal;
						normals[findices[polyIndex + (j + 2) % k]] += normal;
						tangents[findices[polyIndex + j + 0]] += TS.row(0);
						tangents[findices[polyIndex + (j + 1) % k]] += TS.row(0);
						tangents[findices[polyIndex + (j + 2) % k]] += TS.row(0);
					}
				}
				assert(k == 4 || k == 3);
				//TODO Triangulate non-square polygons
				indices.push_back(tmpIndices[0]);
				indices.push_back(tmpIndices[1]);
				indices.push_back(tmpIndices[2]);
				if (k == 4) {
					indices.push_back(tmpIndices[0]);
					indices.push_back(tmpIndices[2]);
					indices.push_back(tmpIndices[3]);
				}
			}
			if constexpr (usesTangentSpace) {
				for (int i = 0; i < normals.size(); i++) {
					auto& normal = normals[i].normalize();
					auto& tangent = tangents[i].normalize();
					auto snormal = Math::snormVec<int8_t>(normal);
					auto stangent = Math::snormVec<int8_t>(tangent);
					auto range = splitVertices.equal_range(i);
					vertices[i].normal = snormal;
					vertices[i].tangent = stangent;
					for (auto b = range.first; b != range.second; b++) {
						vertices[b->second].normal = snormal;
						vertices[b->second].tangent = stangent;
					}
				}
			}
		};
		if (tangentSpace) {
			ret.vertices = std::vector<nyan::StaticMesh::TangentVertex>();
			auto& vertices = ret.get_static_tangent_vertices();
			if (mesh->GetControlPointsCount() >= UINT16_MAX) {
				ret.indices = std::vector<uint32_t>();
				auto& indices = ret.get_indices32();
				lambda(vertices, indices);
			}
			else {
				ret.indices = std::vector<uint16_t>();
				auto& indices = ret.get_indices16();
				lambda(vertices, indices);
			}
		}
		else {
			ret.vertices = std::vector<nyan::StaticMesh::Vertex>();
			auto& vertices = ret.get_static_vertices();
			if (mesh->GetControlPointsCount() >= UINT16_MAX) {
				ret.indices = std::vector<uint32_t>();
				auto& indices = ret.get_indices32();
				lambda(vertices, indices);
			}
			else {
				ret.indices = std::vector<uint16_t>();
				auto& indices = ret.get_indices16();
				lambda(vertices, indices);
			}
		}
		

		retVec.push_back(ret);
	}


	return retVec;
}
