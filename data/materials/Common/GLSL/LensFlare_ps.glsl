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


/* This is free and unencumbered software released into the public domain. https://unlicense.org/
Trying to get some interesting looking lens flares, seems like it worked.
See https://www.shadertoy.com/view/lsBGDK for a more avanced, generalized solution
If you find this useful send me an email at peterekepeter at gmail dot com,
I've seen this shader pop up before in other works, but I'm curious where it ends up.
If you want to use it, feel free to do so, there is no need to credit but it is appreciated.
*/

// float noise(float t)
// {
// 	return texture(iChannel0,vec2(t,.0)/iChannelResolution[0].xy).x;
// }
// float noise(vec2 t)
// {
// 	return texture(iChannel0,t/iChannelResolution[0].xy).x;
// }

vec3 lensflare(vec2 uv, vec2 pos)
{
	vec2 main = uv-pos;
	vec2 uvd = uv*(length(uv));

	float ang = atan(main.x,main.y);
	float dist = length(main);  dist = pow(dist, 0.1);
	// float n = noise(vec2(ang*16.0,dist*32.0));

	float f0 = 1.0/(length(uv-pos)*116.0+1.0);  //*16

	// f0 = f0 + f0*(sin(noise(sin(ang*2.+pos.x)*4.0 - cos(ang*3.+pos.y))*16.)*.1 + dist*.1 + .8);

	float f1 = max(0.01-pow(length(uv+1.2*pos),1.9),.0)*7.0;

	float f2  = max(1.0/(1.0+32.0*pow(length(uvd+0.8*pos),2.0)),.0)*00.25;
	float f22 = max(1.0/(1.0+32.0*pow(length(uvd+0.85*pos),2.0)),.0)*00.23;
	float f23 = max(1.0/(1.0+32.0*pow(length(uvd+0.9*pos),2.0)),.0)*00.21;

	vec2 uvx = mix(uv,uvd,-0.5);

	float f4  = max(0.01-pow(length(uvx+0.4*pos),2.4),.0)*6.0;
	float f42 = max(0.01-pow(length(uvx+0.45*pos),2.4),.0)*5.0;
	float f43 = max(0.01-pow(length(uvx+0.5*pos),2.4),.0)*3.0;

	uvx = mix(uv,uvd,-.4);

	float f5  = max(0.01-pow(length(uvx+0.2*pos),5.5),.0)*2.0;
	float f52 = max(0.01-pow(length(uvx+0.4*pos),5.5),.0)*2.0;
	float f53 = max(0.01-pow(length(uvx+0.6*pos),5.5),.0)*2.0;

	uvx = mix(uv,uvd,-0.5);

	float f6  = max(0.01-pow(length(uvx-0.3*pos),1.6),.0)*6.0;
	float f62 = max(0.01-pow(length(uvx-0.325*pos),1.6),.0)*3.0;
	float f63 = max(0.01-pow(length(uvx-0.35*pos),1.6),.0)*5.0;

	vec3 c = vec3(.0);

	c.r += f2+f4+f5+f6; c.g+=f22+f42+f52+f62; c.b+=f23+f43+f53+f63;
	c = c*1.3 - vec3(length(uvd)*.05);
	c += vec3(f0);

	return c;
}

vec3 cc(vec3 color, float factor,float factor2)  // color modifier
{
	float w = color.x+color.y+color.z;
	return mix(color,vec3(w)*factor,w*factor2);
}

/*
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	vec2 uv = fragCoord.xy / iResolution.xy - 0.5;
	uv.x *= iResolution.x/iResolution.y; //fix aspect ratio
	vec3 mouse = vec3(iMouse.xy/iResolution.xy - 0.5,iMouse.z-.5);
	mouse.x *= iResolution.x/iResolution.y; //fix aspect ratio
	if (iMouse.z<.5)
	{
		mouse.x=sin(iTime)*.5;
		mouse.y=sin(iTime*.913)*.5;
	}

	vec3 color = vec3(1.4,1.2,1.0)*lensflare(uv,mouse.xy);
	color -= noise(fragCoord.xy)*.015;
	color = cc(color,.5,.1);
	fragColor = vec4(color,1.0);
}
*/

void main()
{
	fragColour = texture( vkSampler2D( sceneTexture, texSampler ), inPs.uv0 );

	vec2 uv = inPs.uv0 - vec2(0.5, 0.5);
	float asp = 1920.0 / 1200.0;  //iResolution.x/iResolution.y;  // todo: fix aspect ratio..
	uv.x *= asp;

	vec2 mouse = vec2(-0.4, -0.4);  // todo: par sun pos in screen space
	mouse.x *= asp;

	// fragColour.xyz +=
	vec3 color = vec3(1.0,1.05,1.1) * lensflare(uv, mouse);
	color = cc(color, 0.5, 0.1);
	fragColour.xyz += color; // * color;

	// fragColour.z = texture( vkSampler2D( depthTexture, texSampler ), inPs.uv0 ).x * 1000;
	// fragColour.xy = inPs.uv0.xy;
}
