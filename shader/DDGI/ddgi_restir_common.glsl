#ifndef DDGIRESTIR_COMMON_GLSL
#define DDGIRESTIR_COMMON_GLSL

struct DDGI_ReSTIR_Reservoir {
	// Selected sample: direction
	vec3 direction;

	// Selected sample: irradiance
	vec3 irradiance;
	
	// Selected sample: target PDF
	float targetPdf;

	//RIS weight sum during streaming,
	//the reservoir weight (inverse PDF) after Resampling
	float weightSum;

	// Number of samples considered for this reservoir (pairwise MIS makes this a float)
	float M;

	//uint age;
	//uint packedVisibility
	
	// Cannonical weight when using pairwise MIS;
	float canonicalWeight;

}

#endif