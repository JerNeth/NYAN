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
void Utility::FBXReader::parse_meshes(std::string fbxFile, std::vector<nyan::MeshData>& retMeshes, std::vector<nyan::MaterialData>& retMats)
{
	fbxsdk::FbxImporter* importer = FbxImporter::Create(sdkManager, "");

	if (!importer->Initialize(fbxFile.c_str(), -1, sdkManager->GetIOSettings()))
		return;

	fbxsdk::FbxScene* scene = ::FbxScene::Create(sdkManager, "ImportScene");

	importer->Import(scene);

	importer->Destroy();
	fbxsdk::FbxNode* rootNode = scene->GetRootNode();
	auto texCount = scene->GetTextureCount();
	for (int matIdx = 0; matIdx < scene->GetMaterialCount(); matIdx++) {
		fbxsdk::FbxSurfaceMaterial* mat = scene->GetMaterial(matIdx);
		//std::cout<< mat->GetName() << ' ';
		auto s = mat->ShadingModel.Get();
		if (s == "Phong") {
			auto phong = reinterpret_cast<fbxsdk::FbxSurfacePhong*>(mat);
			
			nyan::MaterialData material
			{
				.name = mat->GetName(),
				.ambientColor = Math::vec3{phong->Ambient.Get()[0], phong->Ambient.Get()[1], phong->Ambient.Get()[2]},
				.diffuseColor = Math::vec3{phong->Diffuse.Get()[0], phong->Diffuse.Get()[1], phong->Diffuse.Get()[2]},
				.ambientFactor {static_cast<float>(phong->AmbientFactor.Get())},
				.diffuseFactor {static_cast<float>(phong->DiffuseFactor.Get())},
				.shininessFacor {static_cast<float>(phong->Shininess.Get())},
			};
			for (int i = 0; i < phong->Diffuse.GetSrcObjectCount(); i++) {
				auto* obj = phong->Diffuse.GetSrcObject(i);
				auto* tex = static_cast<FbxFileTexture*>(obj);
				material.diffuseTex = tex->GetRelativeFileName();
				std::cout << "Diffuse: " << tex->GetRelativeFileName() << "\n";
			}
			for (int i = 0; i < phong->Ambient.GetSrcObjectCount(); i++) {
				auto* obj = phong->Ambient.GetSrcObject(i);
				auto* tex = static_cast<FbxFileTexture*>(obj);
				std::cout << "Ambient: " << tex->GetRelativeFileName() << "\n";
			}
			for (int i = 0; i < phong->Bump.GetSrcObjectCount(); i++) {
				auto* obj = phong->Bump.GetSrcObject(i);
				auto* tex = static_cast<FbxFileTexture*>(obj);
				std::cout << "Bump: " << tex->GetRelativeFileName() << "\n";
			}
			for (int i = 0; i < phong->Emissive.GetSrcObjectCount(); i++) {
				auto* obj = phong->Emissive.GetSrcObject(i);
				auto* tex = static_cast<FbxFileTexture*>(obj);
				std::cout << "Emissive: " << tex->GetRelativeFileName() << "\n";
			}
			for (int i = 0; i < phong->NormalMap.GetSrcObjectCount(); i++) {
				auto* obj = phong->NormalMap.GetSrcObject(i);
				auto* tex = static_cast<FbxFileTexture*>(obj);
				material.normalTex = tex->GetRelativeFileName();
				std::cout << "NormalMap: " << tex->GetRelativeFileName() << "\n";
			}
			for (int i = 0; i < phong->Reflection.GetSrcObjectCount(); i++) {
				auto* obj = phong->Reflection.GetSrcObject(i);
				auto* tex = static_cast<FbxFileTexture*>(obj);
				std::cout << "Reflection: " << tex->GetRelativeFileName() << "\n";
			}
			for (int i = 0; i < phong->Shininess.GetSrcObjectCount(); i++) {
				auto* obj = phong->Shininess.GetSrcObject(i);
				auto* tex = static_cast<FbxFileTexture*>(obj);
				std::cout << "Shininess: " << tex->GetRelativeFileName() << "\n";
			}
			retMats.push_back(material);
		}
	}
	//std::cout << rootNode->GetChildCount() << '\n';
	for (int child = 0, endChildren = rootNode->GetChildCount(true); child < endChildren; child++) {
		//nyan::MeshData ret;
		auto node = rootNode->GetChild(child);
		auto translation = node->LclTranslation.Get();
		auto rotation = node->LclRotation.Get();
		auto scale = node->LclScaling.Get();

		auto mesh = node->GetMesh();
		fbxsdk::FbxVector4* lControlPoints = mesh->GetControlPoints();
		mesh->GenerateTangentsDataForAllUVSets();
		mesh->GenerateNormals();
		assert(mesh->GetElementNormalCount());
		assert(mesh->GetElementUVCount());
		assert(mesh->GetElementTangentCount());

		auto normals = mesh->GetElementNormal(0);
		auto uvs = mesh->GetElementUV(0);
		auto tangents = mesh->GetElementTangent(0);
		auto materials = mesh->GetElementMaterial(0);

		//ret.name = 
		//std::cout << node->GetName() << ' ';
		//std::cout << mesh->GetName() << ' ';

		auto beginRetVec = retMeshes.size();
		auto& materialArray = materials->FbxGeometryElementMaterial::ParentClass::GetDirectArray();
		for (int i = 0; i < materialArray.GetCount(); i++) {
			fbxsdk::FbxSurfaceMaterial* mat= materialArray.GetAt(i);
			retMeshes.push_back(nyan::MeshData
			{
				.name = node->GetName() + std::string(mat->GetName()),
				.material = mat->GetName(),
				.translate = Math::vec3{translation.mData[0],translation.mData[1], translation.mData[2] },
				.rotate = Math::vec3{rotation.mData[0],rotation.mData[1], rotation.mData[2]},
				.scale = Math::vec3{scale.mData[0],scale.mData[1], scale.mData[2]},
			});
		}
		
		//materials->
		int vertexCounter = 0;

		for (int poly = 0, polyCount = mesh->GetPolygonCount(); poly < polyCount; poly++) {
			int idx = poly;
			//assert(materials->GetMappingMode() == FbxGeometryElement::eByPolygon);
			if (materials->GetReferenceMode() == FbxGeometryElement::eIndexToDirect) {
				idx = materials->GetIndexArray()[idx];
			}

			auto& retVal = retMeshes[beginRetVec + idx];
			size_t firstVertex = retVal.positions.size();
			auto polySize = mesh->GetPolygonSize(poly);
			for (int posInPoly = 0; posInPoly < polySize; posInPoly++, vertexCounter++) {
				int ctrlPointIndex = mesh->GetPolygonVertex(poly, posInPoly);
				auto ctrlPoint = lControlPoints[ctrlPointIndex];
				idx = 0;
				if (normals->GetMappingMode() == FbxGeometryElement::eByControlPoint) {
					idx = ctrlPointIndex;
				}
				else if (normals->GetMappingMode() == FbxGeometryElement::eByPolygonVertex) {
					idx = vertexCounter;
				}
				if (normals->GetReferenceMode() == FbxGeometryElement::eIndexToDirect) {
					idx = normals->GetIndexArray()[idx];
				}
				auto normal = normals->GetDirectArray()[idx];

				if (uvs->GetMappingMode() == FbxGeometryElement::eByControlPoint) {
					idx = ctrlPointIndex;
				}
				else if (uvs->GetMappingMode() == FbxGeometryElement::eByPolygonVertex) {
					idx = vertexCounter;
				}
				if (uvs->GetReferenceMode() == FbxGeometryElement::eIndexToDirect) {
					idx = uvs->GetIndexArray()[idx];
				}
				auto uv = uvs->GetDirectArray()[idx];

				if (tangents->GetMappingMode() == FbxGeometryElement::eByControlPoint) {
					idx = ctrlPointIndex;
				}
				else if (tangents->GetMappingMode() == FbxGeometryElement::eByPolygonVertex) {
					idx = vertexCounter;
				}
				if (tangents->GetReferenceMode() == FbxGeometryElement::eIndexToDirect) {
					idx = tangents->GetIndexArray()[idx];
				}
				auto tangent = tangents->GetDirectArray()[idx];
				



				retVal.positions.push_back(Math::vec3{
						static_cast<float>(ctrlPoint.mData[0]),
						static_cast<float>(ctrlPoint.mData[1]),
						static_cast<float>(ctrlPoint.mData[2]),
					});
				retVal.normals.push_back(Math::vec3{
						static_cast<float>(normal.mData[0]),
						static_cast<float>(normal.mData[1]),
						static_cast<float>(normal.mData[2]),
					});
				retVal.tangents.push_back(Math::vec3{
						static_cast<float>(tangent.mData[0]),
						static_cast<float>(tangent.mData[1]),
						static_cast<float>(tangent.mData[2]),
					});
				retVal.uvs.push_back(Math::vec2{
						static_cast<float>(uv.mData[0]),
						static_cast<float>(uv.mData[1]),
					});
			}
			for (size_t l = 2; l < polySize; l++) {
				retVal.indices.push_back(firstVertex);
				retVal.indices.push_back(firstVertex + l - 1ull);
				retVal.indices.push_back(firstVertex + l);
			}
		}
	}

	scene->Destroy(true);
}
