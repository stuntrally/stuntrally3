#version ogre_glsl_ver_330

vulkan_layout( ogre_t0 ) uniform texture2D depthTexture;
vulkan_layout( ogre_t1 ) uniform texture2D sceneTexture;
vulkan( layout( ogre_s0 ) uniform sampler texSampler );

vulkan_layout( location = 0 )
in block
{
	vec2 uv0;
} inPs;

vulkan( layout( ogre_P0 ) uniform Params { )
	uniform vec2 uvSunPos;
	//.. par fog clr
vulkan( }; )

vulkan_layout( location = 0 )
out vec4 fragColour;


//#define DITHER  // todo, dithering, tex 8x8
#define QUALITY  3  //par hq..

// #define DECAY     0.974
// #define EXPOSURE  0.24
#define DECAY     0.964
#define EXPOSURE  0.124

#if (QUALITY==3)  // high
	#define SAMPLES  256
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


void main()
{
	vec2 uv = inPs.uv0;

	#define dep(uv, val)  ( (texture( vkSampler2D( depthTexture, texSampler ), uv).x * 1000.0) < 0.01 ? val : 0.0 )
   	
	//  light, objects
	// float occ = 0.0;
	float occ = dep(uv, 1.0);
    // #ifdef DITHER
    	// float dither = texture(ditherTexture, fragCoord/iChannelResolution[1].xy).r;  //?-
    // #endif

    vec2 uvStep = (uv - uvSunPos) * (1.0 / float(SAMPLES) * DENSITY);
    float illumDecay = 1.;
    
	for (int i=0; i < SAMPLES; i++)
    {
        uv -= uvStep;
        // #ifdef DITHER
        	// float s = texture(depth, uv + (uvStep * dither)).x;
        // #else
			float s = dep(uv, 0.2);
        // #endif
        s *= illumDecay * WEIGHT;
        occ += s;
        illumDecay *= DECAY;
    }
        
	fragColour = texture( vkSampler2D( sceneTexture, texSampler ), inPs.uv0 );

	//  par clr sun,fog
	vec3 rays = vec3(1.0, 0.8, 0.6) * occ * EXPOSURE;
	fragColour.xyz += rays;  
}
