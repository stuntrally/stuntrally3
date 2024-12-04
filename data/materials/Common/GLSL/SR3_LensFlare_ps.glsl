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
	uniform vec4 efxClrSun;
	//.. var
vulkan( }; )

vulkan_layout( location = 0 )
out vec4 fragColour;
// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 


//  adapted from: https://www.shadertoy.com/view/4sX3Rs

// float noise(float t)
// {
// 	return texture(iChannel0,vec2(t,.0)/iChannelResolution[0].xy).x;
// }
// float noise(vec2 t)
// {
// 	return texture(iChannel0,t/iChannelResolution[0].xy).x;
// }

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
vec3 lensflare(vec2 uv, vec2 pos)
{
	vec2 main = uv - pos;
	vec2 uvd = uv*(length(uv));

	float dist = length(main), distUV = dist;  dist = pow(dist, 0.1);
	float ang = atan(main.x,main.y);
	// float n = noise(vec2(ang*16.0,dist*32.0));  // todo: tex

	float f0 = 1.0 / (distUV*64.0+1.0);  //*16  sun  *
	f0 = f0 + f0*(sin(sin(ang*2.+pos.x)*4.0 - cos(ang*3.+pos.y)*16.)*.1 + dist*.1 + .8);
	// f0 = f0 + f0*(sin(noise(sin(ang*2.+pos.x)*4.0 - cos(ang*3.+pos.y))*16.)*.1 + dist*.1 + .8);

	// float fc0 = max(0.0, 1.0/(length(-uv+pos)*62.0-6.0));  // circle o
	// fc0 = pow(fc0, 8.0);
	// fc0 = fc0 + fc0*(sin((ang+1.0/18.0)*12.0)*.1+dist*.1+.8);

	float f1 = max(0.01-pow(length(uv+1.2*pos),1.9),.0)*7.0;

	float f2  = max(1.0/(1.0+32.0*pow(length(uvd+0.8 *pos),2.0)),.0)*0.25;
	float f22 = max(1.0/(1.0+32.0*pow(length(uvd+0.85*pos),2.0)),.0)*0.23;
	float f23 = max(1.0/(1.0+32.0*pow(length(uvd+0.9 *pos),2.0)),.0)*0.21;

	vec2 uvx = mix(uv,uvd,-0.5);

	float f4  = max(0.01-pow(length(uvx+0.4 *pos),2.4),.0)*6.0;
	float f42 = max(0.01-pow(length(uvx+0.45*pos),2.4),.0)*5.0;
	float f43 = max(0.01-pow(length(uvx+0.5 *pos),2.4),.0)*3.0;

	uvx = mix(uv,uvd,-.4);

	float f5  = max(0.01-pow(length(uvx+0.2*pos),5.5),.0)*2.0;
	float f52 = max(0.01-pow(length(uvx+0.4*pos),5.5),.0)*2.0;
	float f53 = max(0.01-pow(length(uvx+0.6*pos),5.5),.0)*2.0;

	uvx = mix(uv,uvd,-0.5);

	float f6  = max(0.01-pow(length(uvx-0.3 *pos),1.6),.0)*6.0;
	float f62 = max(0.01-pow(length(uvx-0.325*pos),1.6),.0)*3.0;
	float f63 = max(0.01-pow(length(uvx-0.35 *pos),1.6),.0)*5.0;

	vec3 c = vec3(.0);

	c.r += f2+f4+f5+f6;
	c.g += f22+f42+f52+f62;
	c.b += f23+f43+f53+f63;

	c = c*1.3 - vec3(length(uvd)*.05);
	c += vec3(f0);
	// c += vec3(f0)*0.1 + vec3(fc0);  // test o

	// return c;
	return c * c;
	// return vec3(pow(c.x,3.1),pow(c.y,3.1),pow(c.z,3.1));
}

vec3 mod_clr(vec3 color, float factor,float factor2)  // color modifier
{
	float w = color.x+color.y+color.z;
	return mix(color, vec3(w)*factor, w*factor2);
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 
void main()
{
	vec2 uvSun = uvSunPos_Fade.xy;
	vec2 uv = inPs.uv0 - vec2(0.5, 0.5);
	uvSun.x *= uvSunPos_Fade.w;  // wnd x/y aspect
	uv.x *= uvSunPos_Fade.w;
	
	//  meh-  sample depth around sun for dim factor
	// todo: not here for every pixel, once in a rtt pass

	vec2 uvSun2 = uvSunPos_Fade.xy + vec2(0.5, 0.5);
	#define getDepth(uv)  ( (texture( vkSampler2D( depthTexture, depthSampler ), uv).x * 1000.0) < 0.01 ? 0.0 : 1.0 )
#if 0
	float dim = 1.0 - getDepth(uvSun2);
#else
	float sum =  // HQ smooth
		getDepth( uvSun2 ) +  // todo pixel size-
		getDepth( uvSun2 +vec2(-0.0008, 0.00 ) ) +
		getDepth( uvSun2 +vec2( 0.0008, 0.00 ) ) +
		getDepth( uvSun2 +vec2( 0.00  ,-0.0008) ) +
		getDepth( uvSun2 +vec2( 0.00  , 0.0008) );
	float dim = max(0.0, 1.0 - sum / 5.0);
#endif

	// if (uv.x > 1.5 || uv.y > 1.5 ||
	// 	uv.x <-0.5 || uv.y <-0.5)
	// 	dim = 0.0;  // out-

	vec3 color = efxClrSun.xyz * lensflare(uv, uvSun );
	// color -= noise(inPs.uv0.xy) * 0.015;
	color = mod_clr(color, 0.5, 0.1)
		* dim * uvSunPos_Fade.z;  // fade
	
	fragColour = texture( vkSampler2D( sceneTexture, texSampler ), inPs.uv0 );  // org scene
	fragColour.xyz += color;  // add
}
