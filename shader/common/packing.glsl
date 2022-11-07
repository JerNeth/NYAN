#ifndef PACKING_GLSL
#define PACKING_GLSL

#define RGB9E5_EXPONENT_BITS          5
#define RGB9E5_MANTISSA_BITS          9
#define RGB9E5_EXP_BIAS               15
#define RGB9E5_MAX_VALID_BIASED_EXP   31

#define MAX_RGB9E5_EXP               (RGB9E5_MAX_VALID_BIASED_EXP - RGB9E5_EXP_BIAS)
#define RGB9E5_MANTISSA_VALUES       (1<<RGB9E5_MANTISSA_BITS)
#define MAX_RGB9E5_MANTISSA          (RGB9E5_MANTISSA_VALUES-1)
#define MAX_RGB9E5                   ((float(MAX_RGB9E5_MANTISSA))/RGB9E5_MANTISSA_VALUES * (1<<MAX_RGB9E5_EXP))
#define EPSILON_RGB9E5               ((1.0/RGB9E5_MANTISSA_VALUES) / (1<<RGB9E5_EXP_BIAS))


float clamp_range_for_rgb9e5(float x) {
    return clamp(x, 0.0, MAX_RGB9E5);
}

int floor_log2(float x) {
    uint f = floatBitsToUint(x);
    uint biasedexponent = (f & 0x7F800000u) >> 23;
    return int(biasedexponent) - 127;
}

// https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_texture_shared_exponent.txt
uint vec3_to_rgb9e5(vec3 rgb) {
    float rc = clamp_range_for_rgb9e5(rgb.x);
    float gc = clamp_range_for_rgb9e5(rgb.y);
    float bc = clamp_range_for_rgb9e5(rgb.z);

    float maxrgb = max(rc, max(gc, bc));
    int exp_shared = max(-RGB9E5_EXP_BIAS-1, floor_log2(maxrgb)) + 1 + RGB9E5_EXP_BIAS;
    float denom = exp2(float(exp_shared - RGB9E5_EXP_BIAS - RGB9E5_MANTISSA_BITS));

    int maxm = int(floor(maxrgb / denom + 0.5));
    if (maxm == MAX_RGB9E5_MANTISSA + 1) {
        denom *= 2;
        exp_shared += 1;
    }

    int rm = int(floor(rc / denom + 0.5));
    int gm = int(floor(gc / denom + 0.5));
    int bm = int(floor(bc / denom + 0.5));
    
//    return (uint(rm) << (32 - 9))
//        | (uint(gm) << (32 - 9 * 2))
//        | (uint(bm) << (32 - 9 * 3))
//        | uint(exp_shared);
    return (uint(rm) << (0))
        | (uint(gm) << (RGB9E5_MANTISSA_BITS * 1))
        | (uint(bm) << ( RGB9E5_MANTISSA_BITS * 2))
        | (uint(exp_shared)<< (RGB9E5_MANTISSA_BITS * 3));
}


uint bitfield_extract(uint value, uint offset, uint bits) {
    uint mask = (1u << bits) - 1u;
    return (value >> offset) & mask;
}

vec3 rgb9e5_to_vec3(uint v) {
    int exponent =
        //int(bitfield_extract(v, 0, RGB9E5_EXPONENT_BITS)) - RGB9E5_EXP_BIAS - RGB9E5_MANTISSA_BITS;
        int(bitfield_extract(v, RGB9E5_MANTISSA_BITS * 3, RGB9E5_EXPONENT_BITS)) - RGB9E5_EXP_BIAS - RGB9E5_MANTISSA_BITS;
    float scale = exp2(float(exponent));

    return vec3(
//        float(bitfield_extract(v, 32 - RGB9E5_MANTISSA_BITS, RGB9E5_MANTISSA_BITS)) * scale,
//        float(bitfield_extract(v, 32 - RGB9E5_MANTISSA_BITS * 2, RGB9E5_MANTISSA_BITS)) * scale,
//        float(bitfield_extract(v, 32 - RGB9E5_MANTISSA_BITS * 3, RGB9E5_MANTISSA_BITS)) * scale
        float(bitfield_extract(v, 0, RGB9E5_MANTISSA_BITS)) * scale,
        float(bitfield_extract(v, RGB9E5_MANTISSA_BITS * 1, RGB9E5_MANTISSA_BITS)) * scale,
        float(bitfield_extract(v, RGB9E5_MANTISSA_BITS * 2, RGB9E5_MANTISSA_BITS)) * scale
    );
}

#undef RGB9E5_EXPONENT_BITS
#undef RGB9E5_MANTISSA_BITS
#undef RGB9E5_EXP_BIAS
#undef RGB9E5_MAX_VALID_BIASED_EXP
#undef MAX_RGB9E5_EXP
#undef RGB9E5_MANTISSA_VALUES
#undef MAX_RGB9E5_MANTISSA
#undef MAX_RGB9E5
#undef EPSILON_RGB9E5


#endif