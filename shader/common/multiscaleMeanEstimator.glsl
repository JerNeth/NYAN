#ifndef MULTISCALEMEANESTIMATOR_GLSL
#define MULTISCALEMEANESTIMATOR_GLSL
struct MultiscaleMeanEstimatorData
{
	vec3 mean;
	vec3 shortMean;
	float vbbr;
	vec3 variance;
	float inconsistency;
};

float MultiscaleMeanEstimator(vec3 y,
	inout MultiscaleMeanEstimatorData data,
	in float shortWindowBlend)
{
	vec3 mean = data.mean;
	vec3 shortMean = data.shortMean;
	float vbbr = data.vbbr;
	vec3 variance = data.variance;
	float inconsistency = data.inconsistency;
  
	// Suppress fireflies.
	{
		vec3 dev = sqrt(max(1e-5f.xxx, variance));
		vec3 highThreshold = 0.1 + shortMean + dev * 8;
		vec3 overflow = max(0.f.xxx, y - highThreshold);
		y -= overflow;
	}
  
	vec3 delta = y - shortMean;
	shortMean = mix(shortMean, y, shortWindowBlend);
	vec3 delta2 = y - shortMean;
  
	// This should be a longer window than shortWindowBlend to avoid bias
	// from the variance getting smaller when the short-term mean does.
	float varianceBlend = shortWindowBlend * 0.5;
	variance = mix(variance, delta * delta2, varianceBlend);
	vec3 dev = sqrt(max(1e-5f.xxx, variance));
  
	vec3 shortDiff = mean - shortMean;
  
	float relativeDiff = luminance(abs(shortDiff) / max(1e-5f.xxx, dev));
	inconsistency = mix(inconsistency, relativeDiff, 0.08f);
  
	float varianceBasedBlendReduction =
		clamp( luminance(0.5 * shortMean / max(1e-5f.xxx, dev) ), 1.0/32.f, 1.f );
  
	float catchUpBlend = clamp(smoothstep(0.f, 1.f,
		relativeDiff * max(0.02f, inconsistency - 0.2)), 1.0f/256.f, 1.f);
	catchUpBlend *= vbbr;
  
	vbbr = mix(vbbr, varianceBasedBlendReduction, 0.1);
	float blendFactor = clamp(catchUpBlend, 6e-5f, 1.f);
	mean = mix(mean, y, blendFactor);
  
	// Output
	data.mean = mean;
	data.shortMean = shortMean;
	data.vbbr = vbbr;
	data.variance = variance;
	data.inconsistency = inconsistency;
	return blendFactor;
}

struct MultiscaleLuminanceMeanEstimatorData
{
	vec3 mean;
	float shortMean;
	float vbbr;
	float variance;
	float inconsistency;
};


float MultiscaleLuminanceMeanEstimator(vec3 y,
	  inout MultiscaleLuminanceMeanEstimatorData data,
	  in float shortWindowBlend)
{
	vec3 mean = data.mean;
	float shortMean = data.shortMean;
	float vbbr = data.vbbr;
	float variance = data.variance;
	float inconsistency = data.inconsistency;
  
	// Suppress fireflies.
//	{
//		vec3 dev = sqrt(max(1e-5f.xxx, variance));
//		vec3 highThreshold = 0.1 + shortMean + dev * 8;
//		vec3 overflow = max(0.f.xxx, y - highThreshold);
//		y -= overflow;
//	}
//  
	float delta = luminance(y) - shortMean;
	shortMean = mix(shortMean, luminance(y), shortWindowBlend);
	float delta2 = luminance(y) - shortMean;
  
	// This should be a longer window than shortWindowBlend to avoid bias
	// from the variance getting smaller when the short-term mean does.
	float varianceBlend = shortWindowBlend * 0.5;
	variance = mix(variance, delta * delta2, varianceBlend);
	float dev = sqrt(max(1e-5f, variance));
  
	float shortDiff = luminance(mean) - shortMean;
  
	float relativeDiff = abs(shortDiff) / max(1e-5f, dev);
	inconsistency = mix(inconsistency, relativeDiff, 0.08f);
  
	float varianceBasedBlendReduction =
		clamp( 0.5 * shortMean / max(1e-5f, dev), 1.0/32.f, 1.f );
  
	float catchUpBlend = clamp(smoothstep(0.f, 1.f,
		relativeDiff * max(0.02f, inconsistency - 0.2)), 1.0f/256.f, 1.f);
	catchUpBlend *= vbbr;
  
	vbbr = mix(vbbr, varianceBasedBlendReduction, 0.1);
	float blendFactor = clamp(catchUpBlend, 6e-5f, 1.f);
	mean = mix(mean, y, blendFactor);
  
	// Output
	data.mean = mean;
	data.shortMean = shortMean;
	data.vbbr = vbbr;
	data.variance = variance;
	data.inconsistency = inconsistency;
	return blendFactor;
}
#endif //MULTISCALEMEANESTIMATOR_GLSL