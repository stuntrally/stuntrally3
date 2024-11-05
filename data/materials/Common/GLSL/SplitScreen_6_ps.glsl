#version ogre_glsl_ver_330

vulkan_layout( ogre_t0 ) uniform texture2D tex1;
vulkan_layout( ogre_t1 ) uniform texture2D tex2;
vulkan_layout( ogre_t2 ) uniform texture2D tex3;
vulkan_layout( ogre_t3 ) uniform texture2D tex4;
vulkan_layout( ogre_t4 ) uniform texture2D tex5;
vulkan_layout( ogre_t5 ) uniform texture2D tex6;

vulkan( layout( ogre_s0 ) uniform sampler sampler1 );
vulkan( layout( ogre_s1 ) uniform sampler sampler2 );
vulkan( layout( ogre_s2 ) uniform sampler sampler3 );
vulkan( layout( ogre_s3 ) uniform sampler sampler4 );
vulkan( layout( ogre_s4 ) uniform sampler sampler5 );
vulkan( layout( ogre_s5 ) uniform sampler sampler6 );

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
		uv1 = uv, uv2 = uv, uv3 = uv,
		uv4 = uv, uv5 = uv, uv6 = uv;
	float t1 = 1.0 / 3.0, t2 = 2.0 / 3.0;

	              uv1.x *= 3.0;
	uv2.x -= t1;  uv2.x *= 3.0;
	uv3.x -= t2;  uv3.x *= 3.0;
	              uv4.x *= 3.0;
	uv5.x -= t1;  uv5.x *= 3.0;
	uv6.x -= t2;  uv6.x *= 3.0;

	uv1.y *= 2.0;  uv2.y *= 2.0;  uv3.y *= 2.0;
	uv4.y -= 0.5;  uv5.y -= 0.5;  uv6.y -= 0.5;
	uv4.y *= 2.0;  uv5.y *= 2.0;  uv6.y *= 2.0;

	fragColour =
		uv.y < 0.5 ?
		(uv.x < t1 ? texture( vkSampler2D( tex1, sampler1 ), uv1 ) :
		 uv.x < t2 ? texture( vkSampler2D( tex2, sampler2 ), uv2 ) :
		             texture( vkSampler2D( tex3, sampler3 ), uv3 )) :
		(uv.x < t1 ? texture( vkSampler2D( tex4, sampler4 ), uv4 ) :
		 uv.x < t2 ? texture( vkSampler2D( tex5, sampler5 ), uv5 ) :
		             texture( vkSampler2D( tex6, sampler6 ), uv6 ));
}
