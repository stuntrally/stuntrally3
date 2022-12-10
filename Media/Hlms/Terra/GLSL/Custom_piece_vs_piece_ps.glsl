// @piece( custom_VStoPS )
	// @property( triplanar )
		// vec3 worldPos : TEXCOORD@counter(texcoord);
	// @end
// @end
// 
// 
// @piece( custom_vs_posExecution )
	// @property( triplanar )
		// outVs.worldPos = worldPos;
	// @end
// @end
// 
// 
// @piece( triplanar_swizzle0 )yz@end
// @piece( triplanar_swizzle1 )xz@end
// @piece( triplanar_swizzle2 )xy@end
