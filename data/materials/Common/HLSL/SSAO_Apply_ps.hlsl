struct PS_INPUT
{
	float2 uv0			: TEXCOORD0;

};

Texture2D<float> ssaoTexture	: register(t0);
Texture2D<float4> sceneTexture	: register(t1);
Texture2D<float> fogTexture		: register(t2);

SamplerState samplerState0		: register(s0);

uniform float powerScale;

float4 main
(
	PS_INPUT inPs
) : SV_Target
{
	float ssao = ssaoTexture.Sample(samplerState0, inPs.uv0);

	ssao = saturate( pow(ssao, powerScale) );

	float4 col = sceneTexture.Sample(samplerState0, inPs.uv0);
	float fog = fogTexture.Sample(samplerState0, inPs.uv0);

	//return float4(ssao, ssao, ssao, col.w);  // test
	//return float4(fog, ssao, ssao, col.w);  // test

	return float4( lerp(col.xyz, col.xyz * ssao, fog), col.w );
}
