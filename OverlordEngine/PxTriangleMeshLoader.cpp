#include "stdafx.h"
#include "PxTriangleMeshLoader.h"
#include "PhysxManager.h"

physx::PxTriangleMesh* PxTriangleMeshLoader::LoadContent(const std::wstring& assetFile)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::string buffer = converter.to_bytes(assetFile);
	//std::string buffer =std::string(assetFile.begin(), assetFile.end());
	auto inputStream  = physx::PxDefaultFileInputData(buffer.c_str());
	return PhysxManager::GetInstance()->GetPhysics()->createTriangleMesh(inputStream);
}

void PxTriangleMeshLoader::Destroy(physx::PxTriangleMesh*) {}
