#version ogre_glsl_ver_330

vulkan_layout( ogre_t0 ) uniform texture2D tex1;
vulkan_layout( ogre_t1 ) uniform texture2D tex2;
vulkan_layout( ogre_t2 ) uniform texture2D tex3;

vulkan( layout( ogre_s0 ) uniform sampler sampler1 );
vulkan( layout( ogre_s1 ) uniform sampler sampler2 );
vulkan( layout( ogre_s2 ) uniform sampler sampler3 );

vulkan_layout( location = 0 )
in block
{
	vec2 uv0;
} inPs;

vulkan_layout( location = 0 )
out vec4 fragColour;

void main()
{
	vec2 uv = inPs.uv0,
		uv1 = uv, uv2 = uv,
		          uv3 = uv;

	               uv1.x *= 2.0;
	uv2.x -= 0.5;  uv2.x *= 2.0;                 uv2.y *= 2.0;
	uv3.x -= 0.5;  uv3.x *= 2.0;  uv3.y -= 0.5;  uv3.y *= 2.0;

	fragColour =
		uv.x < 0.5
		? texture( vkSampler2D( tex1, sampler1 ), uv1 ) :
		(uv.y < 0.5
		? texture( vkSampler2D( tex2, sampler2 ), uv2 )
		: texture( vkSampler2D( tex3, sampler3 ), uv3 ));
}
