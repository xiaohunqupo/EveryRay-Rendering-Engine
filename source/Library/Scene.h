#pragma once
#include "Common.h"
#include "GameComponent.h"
#include "Camera.h"
#include "Material.h"
#include "ER_Material.h"
#include "ModelMaterial.h"
#include "Effect.h"

#include "..\JsonCpp\include\json\json.h"

namespace Rendering
{
	class RenderingObject;
}

namespace Library
{
	class Scene : public GameComponent
	{
	public:
		Scene(Game& pGame, Camera& pCamera, const std::string& path);
		~Scene();

		void SaveRenderingObjectsTransforms();

		ER_Material* GetMaterialByName(const std::string& matName, const MaterialShaderEntries& entries, bool instanced);
		Camera& GetCamera() { return mCamera; }
		const XMFLOAT3& GetLightProbesVolumeMinBounds() const { return mLightProbesVolumeMinBounds; }
		const XMFLOAT3& GetLightProbesVolumeMaxBounds() const { return mLightProbesVolumeMaxBounds; }
		bool HasLightProbesSupport() { return mHasLightProbes; }

		std::map<std::string, Rendering::RenderingObject*> objects;

		//TODO remove to private and make public methods
		std::string skyboxPath;
		XMFLOAT3 cameraPosition;
		XMFLOAT3 cameraDirection;
		XMFLOAT3 sunDirection;
		XMFLOAT3 sunColor;
		XMFLOAT3 ambientColor;

	private:
		void LoadRenderingObjectData(Rendering::RenderingObject* aObject);
		void LoadRenderingObjectInstancedData(Rendering::RenderingObject* aObject);
		std::tuple<Material*, Effect*, std::string> CreateMaterialData(const std::string& materialName, const std::string& effectName, const std::string& techniqueName);
		Json::Value root;
		Camera& mCamera;
		std::string mScenePath;

		bool mHasLightProbes = true;
		XMFLOAT3 mLightProbesVolumeMinBounds = { 0,0,0 };
		XMFLOAT3 mLightProbesVolumeMaxBounds = { 0,0,0 };
	};
}