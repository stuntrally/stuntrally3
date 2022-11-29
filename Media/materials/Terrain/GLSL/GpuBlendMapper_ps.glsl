// Our terrain has the following pattern:
//
//	 1N  10   11
//		o-o-o
//		|/|/|
//	 0N	o-+-o 01
//		|/|/|
//		o-o-o
//	 NN  N0   N1
//
// We need to calculate the normal of the vertex in
// the center '+', which is shared by 6 triangles.

#version ogre_glsl_ver_330

#ifndef USE_UINT
	vulkan_layout( ogre_t0 ) uniform texture2D heightMap;
	vulkan_layout( ogre_t0 ) uniform texture2D normalMap1;
#else
	vulkan_layout( ogre_t0 ) uniform utexture2D heightMap;
	vulkan_layout( ogre_t0 ) uniform utexture2D normalMap1;
#endif

vulkan( layout( ogre_P0 ) uniform Params { )
	uniform vec2 heightMapResolution;
	uniform vec3 vScale;
	uniform float terrainWorldSize;
	// uniform vec4 vLayers;

	//  blendmap params for 4 layers
	uniform vec4 Hmin;
	uniform vec4 Hmax;
	uniform vec4 Hsmt;
														 
	uniform vec4 Amin;
	uniform vec4 Amax;
	uniform vec4 Asmt;
	uniform vec4 Nonly;  // bool

	//  noise mul
	uniform vec3 Nnext;
	uniform vec3 Nprev;
	uniform vec2 Nnext2;
	//  noise +1,-1 pars
	uniform vec3 Nfreq;
	uniform vec3 Noct;  // uint8
	uniform vec3 Npers;
	uniform vec3 Npow;
	//  noise +2 pars
	uniform vec2 Nfreq2;
	uniform vec2 Noct2;
	uniform vec2 Npers2;
	uniform vec2 Npow2;
vulkan( }; )

vulkan_layout( location = 0 )
out vec4 fragColour;

vulkan_layout( location = 0 )
in block
{
	vec2 uv0;
} inPs;


//--------------------------------------------------------------------------------------------------
// #include "Noise.glsl"  // todo: ?

//  Array and textureless 2D simplex noise function
//  Author:  Ian McEwan, Ashima Arts
//  License: MIT  https://github.com/ashima/webgl-noise

vec3 mod289(vec3 x) {	return x - floor(x * (1.0 / 289.0)) * 289.0;  }
vec2 mod289(vec2 x) {	return x - floor(x * (1.0 / 289.0)) * 289.0;  }

vec3 permute(vec3 x) {	return mod289( ((x*34.0) + 1.0) * x);  }

float snoise1(vec2 v)
{
	const vec4 C = vec4(
		0.211324865405187,  // (3.0-sqrt(3.0))/6.0
		0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
		-0.577350269189626, // -1.0 + 2.0 * C.x
		0.024390243902439); // 1.0 / 41.0

	//  First corner
	vec2 i = floor(v + dot(v, C.yy));
	vec2 x0 = v - i + dot(i, C.xx);

	//  Other corners
	vec2 i1;
	i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
	// x0 = x0 - 0.0 + 0.0 * C.xx;
	// x1 = x0 - i1  + 1.0 * C.xx;
	// x2 = x0 - 1.0 + 2.0 * C.xx;
	vec4 x12 = x0.xyxy + C.xxzz;
	x12.xy -= i1;

	//  Permutations
	i = mod289(i);  // Avoid truncation effects
	vec3 p = permute( permute(
		i.y + vec3(0.0, i1.y, 1.0)) +
		i.x + vec3(0.0, i1.x, 1.0));

	vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
	m = m*m;  m = m*m;

	//  Gradients: 41 points uniformly over a line, mapped onto a diamond.
	//  The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)
	vec3 x = 2.0 * fract(p * C.www) - 1.0;
	vec3 h = abs(x) - 0.5;
	vec3 ox = floor(x + 0.5);
	vec3 a0 = x - ox;

	//  Normalise gradients implicitly by scaling m
	//  Approximation of: m *= inversesqrt( a0*a0 + h*h );
	m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

	//  Compute final noise value at P
	vec3 g;
	g.x = a0.x * x0.x + h.x * x0.y;
	g.yz = a0.yz * x12.xz + h.yz * x12.yw;
	//return 130.0 * dot(m, g);  // -1..1
	return 0.5 + 0.5 * 130.0 * dot(m, g);  // 0..1
}

float snoise(vec2 v, float zoom, int octaves, float persistence)
{
    float total = 0.0;
    for (int i=0; i < 5; ++i)  // const loop
    {
        float frequency = pow(2.f, float(i));
        float amplitude = pow(persistence, float(i));
        float nval = snoise1(v * frequency * zoom) * amplitude;
        total += i < octaves ? nval : 0.f;
    }
    //return total;
    float m = (persistence - 0.1f) * -0.83f + 1.f;
    return total * m;
    // pers = 0.7, mul = 0.5  pers = 0.1, mul = 1
}

//     xa  xb
//1    .___.
//0__./     \.___
//   xa-s    xb+s   // val, min, max, smooth range
float linRange(float x, float xa, float xb, float s)
{
	float lr = x < xa ? (x-xa)/s+1.f : (xb-x)/s+1.f;
	return clamp( lr, 0.0,1.0);
}


//--------------------------------------------------------------------------------------------------
void main()
{
	//  get normal
	// float normal00 = float( texelFetch( normalMap1, ivec2(  iCoord.x, iCoord.y ), 0 ).x );  // todo: ? ..
	ivec2 iCoord = ivec2( inPs.uv0 * heightMapResolution );

	ivec3 xN01;
	xN01.x = max( iCoord.x - 1, 0 );
	xN01.y = iCoord.x;
	xN01.z = min( iCoord.x + 1, int(heightMapResolution.x) );
	ivec3 yN01;
	yN01.x = max( iCoord.y - 1, 0 );
	yN01.y = iCoord.y;
	yN01.z = min( iCoord.y + 1, int(heightMapResolution.y) );

	// It's height XY, but texelFetch uses YX.
	float heightNN = float( texelFetch( heightMap, ivec2( xN01.x, yN01.x ), 0 ).x ) * vScale.y;
	float heightN0 = float( texelFetch( heightMap, ivec2( xN01.y, yN01.x ), 0 ).x ) * vScale.y;

	float height0N = float( texelFetch( heightMap, ivec2( xN01.x, yN01.y ), 0 ).x ) * vScale.y;
	float height00 = float( texelFetch( heightMap, ivec2( xN01.y, yN01.y ), 0 ).x ) * vScale.y;
	float height01 = float( texelFetch( heightMap, ivec2( xN01.z, yN01.y ), 0 ).x ) * vScale.y;

	float height10 = float( texelFetch( heightMap, ivec2( xN01.y, yN01.z ), 0 ).x ) * vScale.y;
	float height11 = float( texelFetch( heightMap, ivec2( xN01.z, yN01.z ), 0 ).x ) * vScale.y;

	vec3 vNN = vec3( -vScale.x, heightNN, -vScale.z );
	vec3 vN0 = vec3( -vScale.x, heightN0,  0 );
	vec3 v0N = vec3(  0, height0N, -vScale.z );
	vec3 v00 = vec3(  0, height00,  0 );
	vec3 v01 = vec3(  0, height01,  vScale.z );
	vec3 v10 = vec3(  vScale.x, height10,  0 );
	vec3 v11 = vec3(  vScale.x, height11,  vScale.z );

	vec3 vNormal = vec3( 0, 0, 0 );
	vNormal += cross( (v01 - v00), (v11 - v00) );
	vNormal += cross( (v11 - v00), (v10 - v00) );
	vNormal += cross( (v10 - v00), (v0N - v00) );
	vNormal += cross( (v0N - v00), (vNN - v00) );
	vNormal += cross( (vNN - v00), (vN0 - v00) );
	vNormal += cross( (vN0 - v00), (v01 - v00) );
	vNormal = normalize( vNormal );

	
	//  for blendmap
	float a = max(0.0, acos(vNormal.y) * 180.0 / 3.1415926536);  // get angle in degrees
	// float a = normal00 * 240;  // test
	// float a = max(0.0, acos(normal00 * 2.0 - 1.0) * 180.0 / 3.1415926536);

	float h = float( texelFetch( heightMap, ivec2( iCoord.x, iCoord.y ), 0 ).x );
	vec2 uv1 = vec2(inPs.uv0.x, 1.0 - inPs.uv0.y);
	vec2 tuv = uv1 * terrainWorldSize / 512.f;


	//  blendmap  ----

	//  ter ang,h ranges  (turned off if noise only)
	float l0a = Nonly.x < 0.1f ? 0.f : linRange(a, Amin.x, Amax.x, Asmt.x) * linRange(h, Hmin.x, Hmax.x, Hsmt.x), l0 = l0a;
	float l1a = Nonly.y < 0.1f ? 0.f : linRange(a, Amin.y, Amax.y, Asmt.y) * linRange(h, Hmin.y, Hmax.y, Hsmt.y), l1 = l1a;
	float l2a = Nonly.z < 0.1f ? 0.f : linRange(a, Amin.z, Amax.z, Asmt.z) * linRange(h, Hmin.z, Hmax.z, Hsmt.z), l2 = l2a;
	float l3a = Nonly.w < 0.1f ? 0.f : linRange(a, Amin.w, Amax.w, Asmt.w) * linRange(h, Hmin.w, Hmax.w, Hsmt.w), l3 = l3a;
	
	//  noise par
	float n0 = Nnext.x < 0.01f ? 0.f : Nnext.x * pow( snoise(tuv, Nfreq.x, int(Noct.x), Npers.x), Npow.x);
	float n1 = Nnext.y < 0.01f ? 0.f : Nnext.y * pow( snoise(tuv, Nfreq.y, int(Noct.y), Npers.y), Npow.y);
	float n2 = Nnext.z < 0.01f ? 0.f : Nnext.z * pow( snoise(tuv, Nfreq.z, int(Noct.z), Npers.z), Npow.z);

	float p1 = Nprev.x < 0.01f ? 0.f : Nprev.x * pow( snoise(tuv, Nfreq.x+3.f, int(Noct.x), Npers.x), Npow.x);
	float p2 = Nprev.y < 0.01f ? 0.f : Nprev.y * pow( snoise(tuv, Nfreq.y+3.f, int(Noct.y), Npers.y), Npow.y);
	float p3 = Nprev.z < 0.01f ? 0.f : Nprev.z * pow( snoise(tuv, Nfreq.z+3.f, int(Noct.z), Npers.z), Npow.z);

	float m0 = Nnext2.x < 0.01f ? 0.f : Nnext2.x * pow( snoise(tuv, Nfreq2.x, int(Noct2.x), Npers2.x), Npow2.x);
	float m1 = Nnext2.y < 0.01f ? 0.f : Nnext2.y * pow( snoise(tuv, Nfreq2.y, int(Noct2.y), Npers2.y), Npow2.y);

	//  add noise
	//  +1, to next layer
	l1 += l0a * n0;  l0 *= 1.f-n0;
	l2 += l1a * n1;  l1 *= 1.f-n1;
	l3 += l2a * n2;  l2 *= 1.f-n2;
	//  -1, to prev
	l0 += l1a * p1;  l1 *= 1.f-p1;
	l1 += l2a * p2;  l2 *= 1.f-p2;
	l2 += l3a * p3;  l3 *= 1.f-p3;
	//  +2
	l2 += l0a * m0;  l0 *= 1.f-m0;
	l3 += l1a * m1;  l1 *= 1.f-m1;
	
	//  normalize  (sum = 1)
	l0 = clamp(l0, 0.0,1.0);
	l1 = clamp(l1, 0.0,1.0);
	l2 = clamp(l2, 0.0,1.0);
	l3 = clamp(l3, 0.0,1.0);  // fix white dots
	float ll = l0+l1+l2+l3;
	if (ll < 0.01f)  {  l0 = 1.f;  ll = l0+l1+l2+l3;  }  // fix black dots
	
	ll = 1/ll;  l0 *= ll;  l1 *= ll;  l2 *= ll;  l3 *= ll;  // norm
	fragColour = vec4(l0, l1, l2, l3);
}
