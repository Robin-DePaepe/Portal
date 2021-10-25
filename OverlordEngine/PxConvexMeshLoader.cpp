#include "stdafx.h"
#include "PxConvexMeshLoader.h"
#include "PhysxManager.h"

physx::PxConvexMesh* PxConvexMeshLoader::LoadContent(const std::wstring& assetFile)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::string buffer = converter.to_bytes(assetFile);
	//std::string buffer = std::string(assetFile.begin(), assetFile.end());
	auto inputStream  = physx::PxDefaultFileInputData(buffer.c_str());
	return PhysxManager::GetInstance()->GetPhysics()->createConvexMesh(inputStream);
}

void PxConvexMeshLoader::Destroy(physx::PxConvexMesh* ) {}
