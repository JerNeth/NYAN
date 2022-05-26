
mat4x3 fetchRowMatrix(vec4 row1, vec4 row2, vec4 row3) {
	return mat4x3(row1.x, row2.x, row3.x,
						row1.y, row2.y, row3.y,
						row1.z, row2.z, row3.z,
						row1.w, row2.w, row3.w);
}
mat4x3 fetchTransformMatrix(Instance instance) {
    return fetchRowMatrix(instance.modelRow1, 
                       instance.modelRow2,
                        instance.modelRow3);
}
