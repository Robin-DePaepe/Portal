#pragma once
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <DirectXPackedVector.h>
#include <DirectXCollision.h>
#include <VertexHelper.h>

struct SkinnedVertex
{
	SkinnedVertex(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 normal, DirectX::XMFLOAT4 color, float blendWeight0 = 1.f, float blendWeight1 = 0.f):
		TransformedVertex(position, normal, color),
		OriginalVertex(position, normal, color),
		 BlendWeight0{ blendWeight0 },
		 BlendWeight1{ blendWeight1 }
	{}
	VertexPosNormCol TransformedVertex;
	VertexPosNormCol OriginalVertex;
	float BlendWeight0, BlendWeight1;
};
