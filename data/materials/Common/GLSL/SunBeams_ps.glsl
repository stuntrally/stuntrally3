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
	//.. par
vulkan( }; )

vulkan_layout( location = 0 )
out vec4 fragColour;


//#define DITHER  // dithering toggle, tex
#define QUALITY  3  //par hq..

// #define DECAY     0.974
// #define EXPOSURE  0.24
#define DECAY     0.964
#define EXPOSURE  0.124

#if (QUALITY==3)  // high
	#define SAMPLES  256
	#define DENSITY  0.98
	#define WEIGHT   0.25
#else
#if (QUALITY==2)  // med
	#define SAMPLES  64
	#define DENSITY  0.97
	#define WEIGHT   0.25
#else
#if (QUALITY==1)  // low
	#define SAMPLES  32
	#define DENSITY  0.95
	#define WEIGHT   0.25
#else  // lowest
	#define SAMPLES  16
	#define DENSITY  0.93
	#define WEIGHT   0.36
#endif
#endif
#endif


void main()
{
    vec2 lightpos = uvSunPos;
	vec2 uv = inPs.uv0;
   	
	//  light, objects
	float occ = 0.0;  //texture( vkSampler2D( sceneTexture, texSampler ), inPs.uv0 ).r;
	float obj = texture( vkSampler2D( depthTexture, texSampler ), uv ).r * 1090.0 < 0.01f ? 1.0 : 0.0;
    // #ifdef DITHER
    	// float dither = texture(iChannel1, fragCoord/iChannelResolution[1].xy).r;  //?-
    // #endif

    vec2 dtc = (uv - lightpos) * (1.0 / float(SAMPLES) * DENSITY);
    float illumDecay = 1.;
    
	for (int i=0; i < SAMPLES; i++)
    {
        uv -= dtc;
        // #ifdef DITHER
        	// float s = texture(iChannel0, uv+(dtc*dither)).x;
        // #else
        	// float s = texture(iChannel0, uv).x;
			float s = texture( vkSampler2D( depthTexture, texSampler ), uv ).r * 1090.0 < 0.01f ? 0.2 : 0.0;  // obj
        // #endif
        s *= illumDecay * WEIGHT;
        occ += s;
        illumDecay *= DECAY;
    }
        
	fragColour = texture( vkSampler2D( sceneTexture, texSampler ), uv ).xyz;

	vec3 rays = vec3(0.0, 0.0, obj * 0.333) + occ*EXPOSURE;
	fragColour += rays;
}
