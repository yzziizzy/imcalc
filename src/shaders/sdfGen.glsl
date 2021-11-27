#shader VERTEX


#version 330 core

layout (location = 0) in vec2 pos_in;

uniform ivec2 outSize;


void main() {	
	gl_Position = vec4(pos_in * outSize, 0, 1);
}






#shader FRAGMENT

#version 330


layout(location = 0) out vec4 out_Color;

uniform float searchSize;
uniform sampler2D rawTex;



float sdfEncode(float d, float inside, float maxDist) {
	float o;
	d = sqrt(d);
	float norm = d / maxDist;
	if(inside == 1.0) norm = -norm;
	
	o = (norm * 192.0) + 64.0;
	
	return clamp(o, 0.0, 255.0) / 255.0;
}




void main(void) {
	
	ivec2 center = ivec2(ivec2(gl_FragCoord.xy)) + ivec2(searchSize, searchSize);

	// value right under the center of the pixel, to determine if we are inside
	// or outside the glyph
	float v = texelFetch(rawTex, center, 0).r;
	
	
	float d = 99999999.99;
	
	ivec2 tp;
	for(tp.y = center.y - int(searchSize); tp.y < center.y + int(searchSize); tp.y+=2) {
		for(tp.x = center.x - int(searchSize); tp.x < center.x + int(searchSize); tp.x+=2) {
			
			
			
			float s1 = texelFetch(rawTex, tp, 0).r;
			if(s1 != v) {
				ivec2 a = tp - center;
				d = min(dot(a, a), d);
			}
			
			#define check(off) \
			{ \
			float s2 = texelFetchOffset(rawTex, tp, 0, off).r; \
			if(s2 != v) { \
				ivec2 a = tp - center + off; \
				d = min(dot(a, a), d); \
			}}
			
			check(ivec2(0,1))
			check(ivec2(1,0))
			check(ivec2(1,1))

		}
	}
	
	float q = sdfEncode(d, v, searchSize);

	out_Color = vec4(q,0,0,1);
}
