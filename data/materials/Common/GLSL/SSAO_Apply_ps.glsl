#version ogre_glsl_ver_330

vulkan_layout( ogre_t0 ) uniform texture2D ssaoTexture;
vulkan_layout( ogre_t1 ) uniform texture2D sceneTexture;
vulkan_layout( ogre_t2 ) uniform texture2D fogTexture;

vulkan( layout( ogre_s0 ) uniform sampler samplerState0 );

vulkan_layout( location = 0 )
in block
{
	vec2 uv0;
} inPs;

vulkan_layout( location = 0 )
out vec4 fragColour;

vulkan( layout( ogre_P0 ) uniform Params { )
	uniform float powerScale;
vulkan( }; )

void main()
{
	float ssao = texture( vkSampler2D( ssaoTexture, samplerState0 ), inPs.uv0 ).x;

	ssao = clamp( pow(ssao, powerScale), 0.0, 1.0);

	vec4 col = texture( vkSampler2D( sceneTexture, samplerState0 ), inPs.uv0 );
	float fog = texture( vkSampler2D( fogTexture, samplerState0 ), inPs.uv0 ).x;

	// fragColour = vec4( ssao, ssao, ssao, 1.0 );  // test ssao only
	// fragColour = vec4( ssao, ssao, fog, 1.0 );  // test fog, ssao  yellow-blue

	fragColour = vec4( mix(col.xyz, col.xyz * ssao, fog), col.a );
}
