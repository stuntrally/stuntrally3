#version ogre_glsl_ver_330

vulkan_layout( ogre_t0 ) uniform texture2D depthTexture;
vulkan_layout( ogre_t1 ) uniform texture2D sceneTexture;
vulkan( layout( ogre_s0 ) uniform sampler texSampler );

vulkan_layout( location = 0 )
in block
{
	vec2 uv0;
} inPs;

vulkan_layout( location = 0 )
out vec4 fragColour;

void main()
{
	fragColour = texture( vkSampler2D( sceneTexture, texSampler ), inPs.uv0 );
	// fragColour.z = texture( vkSampler2D( depthTexture, texSampler ), inPs.uv0 ).x * 1000;
	// fragColour.xy = inPs.uv0.xy;
}
