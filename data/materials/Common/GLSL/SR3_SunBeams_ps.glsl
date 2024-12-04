#version ogre_glsl_ver_330

vulkan_layout( ogre_t0 ) uniform texture2D depthTexture;
vulkan_layout( ogre_t1 ) uniform texture2D sceneTexture;
vulkan( layout( ogre_s0 ) uniform sampler depthSampler );
vulkan( layout( ogre_s1 ) uniform sampler texSampler );

vulkan_layout( location = 0 )
in block
{
	vec2 uv0;
} inPs;

vulkan( layout( ogre_P0 ) uniform Params { )
	uniform vec4 uvSunPos_Fade;  // .w aspect
	uniform vec4 efxClrRays;
	//.. par
vulkan( }; )

vulkan_layout( location = 0 )
out vec4 fragColour;
// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 


//  inspired by:
// https://www.shadertoy.com/view/4dyXWy
// https://www.shadertoy.com/view/MlccW4

//#define DITHER  // todo, dithering, tex 8x8 or more noise
#define QUALITY  2  //par hq..

#define DECAY  0.974
#define SCALE  0.24
// #define DECAY  0.984
// #define SCALE  0.124

#if (QUALITY==3)  // high
	#define SAMPLES  128  //256
	#define DENSITY  0.98
	#define WEIGHT   0.25
#endif
#if (QUALITY==2)  // med
	#define SAMPLES  64
	#define DENSITY  0.97
	#define WEIGHT   0.25
#endif
#if (QUALITY==1)  // low
	#define SAMPLES  32
	#define DENSITY  0.95
	#define WEIGHT   0.25
#endif
#if (QUALITY==0)  // lowest
	#define SAMPLES  16
	#define DENSITY  0.93
	#define WEIGHT   0.36
#endif

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
void main()
{
	vec2 uv = inPs.uv0;
	vec2 uvSun = uvSunPos_Fade.xy + vec2(0.5, 0.5);
	float fade = pow(uvSunPos_Fade.z, 3.0);  //par

	// if (fade > 0.01f)  // skip?
	#define getDepth(uv, val)  ( (texture( vkSampler2D( depthTexture, depthSampler ), uv).x * 1000.0) < 0.01 ? val : 0.0 )

	//  light, objects
	// float occ = 0.0;
	float sum = getDepth(uv, 1.0);
	// #ifdef DITHER
		// float dither = texture(ditherTexture, fragCoord/iChannelResolution[1].xy).r;  //?-
	// #endif

	vec2 uvStep = (uv - uvSun) * (1.0 / float(SAMPLES) * DENSITY);
	float illumDecay = 1.;

	for (int i=0; i < SAMPLES; i++)
	{
		uv -= uvStep;
		// #ifdef DITHER
			// float s = texture(depth, uv + (uvStep * dither)).x;
		// #else
			float s = getDepth(uv, 0.2);
		// #endif
		s *= illumDecay * WEIGHT;
		sum += s;
		illumDecay *= DECAY;
	}
		
	fragColour = texture( vkSampler2D( sceneTexture, texSampler ), inPs.uv0 );  // org scene

	vec3 rays = efxClrRays.xyz * sum * SCALE * fade;
	fragColour.xyz += rays;  // add
}
