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

/// Create Depth Stencil State (ENABLE DEPTH WRITING)
/// Create Rasterizer State (Backface culling) 

DepthStencilState EnableDepthWriting
{
	DepthEnable = true;
	DepthWriteMask = ALL;
};

RasterizerState BackCulling
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

	output.TexCoord = input.TexCoord;
	output.Position = float4(input.Position, 1.f);

	return output;
}


//PIXEL SHADER
//------------
float4 PS(PS_INPUT input) : SV_Target
{
	// Step 1: sample the texture
	float4 sampleColor = gTexture.Sample(samPoint, input.TexCoord);

	// Step 2: calculate the mean value
	float mean = (sampleColor.r + sampleColor.g + sampleColor.b) / 3.f;

	// Step 3: return the color
	return float4(mean, mean, mean, 1.0f);
}

BlendState gBlendState
{
	BlendEnable[0] = false;
};


//TECHNIQUE
//---------
technique11 Grayscale
{
	pass P0
	{
		// Set states...
		SetVertexShader(CompileShader(vs_4_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PS()));

		SetRasterizerState(BackCulling);
		SetDepthStencilState(EnableDepthWriting, 0);
		SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
	}
}

