//Copyright (c) 2019 Embark Studios
//
//Permission is hereby granted, free of charge, to any
//person obtaining a copy of this software and associated
//documentation files (the "Software"), to deal in the
//Software without restriction, including without
//limitation the rights to use, copy, modify, merge,
//publish, distribute, sublicense, and/or sell copies of
//the Software, and to permit persons to whom the Software
//is furnished to do so, subject to the following
//conditions:
//
//The above copyright notice and this permission notice
//shall be included in all copies or substantial portions
//of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
//ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
//TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
//PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
//SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
//IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//DEALINGS IN THE SOFTWARE.

#ifndef HASH_GLSL
#define HASH_GLSL

// Jenkins hash function. TODO: check if we need something better.
uint hash1(uint x) {
	x += (x << 10u);
	x ^= (x >>  6u);
	x += (x <<  3u);
	x ^= (x >> 11u);
	x += (x << 15u);
	return x;
}

uint hash1_mut(inout uint h) {
    uint res = h;
    h = hash1(h);
    return res;
}

uint hash_combine2(uint x, uint y) {
    const uint M = 1664525u, C = 1013904223u;
    uint seed = (x * M + y + C) * M;

    // Tempering (from Matsumoto)
    seed ^= (seed >> 11u);
    seed ^= (seed << 7u) & 0x9d2c5680u;
    seed ^= (seed << 15u) & 0xefc60000u;
    seed ^= (seed >> 18u);
    return seed;
}

uint hash2(uvec2 v) {
	return hash_combine2(v.x, hash1(v.y));
}

uint hash3(uvec3 v) {
	return hash_combine2(v.x, hash2(v.yz));
}

uint hash4(uvec4 v) {
	return hash_combine2(v.x, hash3(v.yzw));
}

float uint_to_u01_float(uint h) {
	const uint mantissaMask = 0x007FFFFFu;
	const uint one = floatBitsToUint(1.f);

	h &= mantissaMask;
	h |= one;

	float  r2 = uintBitsToFloat( h );
	return r2 - 1.0;
}

float interleaved_gradient_noise(uvec2 px) {
    return fract(52.9829189 * fract(0.06711056f * float(px.x) + 0.00583715f * float(px.y)));
}


#endif