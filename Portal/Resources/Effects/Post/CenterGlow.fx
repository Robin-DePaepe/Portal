//=============================================================================
//// Shader uses position and texture
//=============================================================================
SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Mirror;
	AddressV = Mirror;
};

Texture2D gTexture;
float gGlowSizeRatio;
float gOpacity;
bool gUseCube;

/// Create Depth Stencil State (ENABLE DEPTH WRITING)
DepthStencilState EnableDepth
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
};
/// Create Rasterizer State (Backface culling) 
RasterizerState backCulling
{
	CullMode = BACK;
};

//IN/OUT STRUCTS
//--------------
struct VS_INPUT
{
	float3 Position : POSITION;
	float2 TexCoord : TEXCOORD0;

};

struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD1;
};


//VERTEX SHADER
//-------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;

	output.Position = float4(input.Position, 1);
	output.TexCoord = input.TexCoord;

	return output;
}


//PIXEL SHADER
//------------
float4 PS(PS_INPUT input) : SV_Target
{
	uint  width, height;

	gTexture.GetDimensions(width,height);

	float2 screenCenter = float2(width / 2.f, height / 2.f);

	const float widthSize = width * gGlowSizeRatio;
	const float heightSize = height * gGlowSizeRatio;
	const float size = (heightSize + widthSize) / 2.f;

	const float x = input.TexCoord.x * width;
	const float y = input.TexCoord.y * height;

	float4 finalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

	if (gUseCube)
	{
		if (x > screenCenter.x - size && x < screenCenter.x + size && y > screenCenter.y - size && y < screenCenter.y + size)
		{
			finalColor = float4(1.f, 1.f, 1.f, gOpacity);
		}
	}
	else
	{
		if (sqrt(pow(x - screenCenter.x, 2) + pow(y - screenCenter.y, 2)) <= size)
		{
		finalColor = float4(1.f, 1.f, 1.f, gOpacity);
		}
	}

		return finalColor;
}


//TECHNIQUE
//---------
technique11 CenterGlow
{
	pass P0
	{
		// Set states...
		SetRasterizerState(backCulling);
		SetDepthStencilState(EnableDepth, 0);

		SetVertexShader(CompileShader(vs_4_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PS()));
	}
}