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
const int gPixelOffset;

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
	float4 finalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
	// Step 1: find the dimensions of the texture (the texture has a method for that)	
	gTexture.GetDimensions(width,height);

	// Step 2: calculate dx and dy (UV space for 1 pixel)	
	float du = (1.f / width) * gPixelOffset;
	float dv = (1.f / height) * gPixelOffset;
	// Step 3: Create a double for loop (5 iterations each)
	//		   Inside the loop, calculate the offset in each direction. Make sure not to take every pixel but move by 2 pixels each time
	//			Do a texture lookup using your previously calculated uv coordinates + the offset, and add to the final color
	int count = 0;
	const uint loops = 5;
	const uint offsetCorrection = loops / 2;

	for (uint i = 0; i < loops; i++)
	{
		for (uint j = 0; j < loops; j++)
		{
			float2 offset = float2(du * (i - offsetCorrection), dv * (j - offsetCorrection));
			float2 lookupUv = float2(input.TexCoord.x + offset.x, input.TexCoord.y + offset.y);

			if (lookupUv.x <= 1 && lookupUv.x >= 0 && lookupUv.y <= 1 && lookupUv.y >= 0)
			{
				finalColor += gTexture.Sample(samPoint, lookupUv);
				++count;
			}
		}
	}
	// Step 4: Divide the final color by the number of passes (in this case 5*5)	
	finalColor /= count;
	finalColor.w = 1.f;
	// Step 5: return the final color
	return finalColor;
}


//TECHNIQUE
//---------
technique11 Blur
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