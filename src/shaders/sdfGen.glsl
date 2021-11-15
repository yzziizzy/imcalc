#shader VERTEX


#version 330 core

layout (location = 0) in vec2 pos_in;
out vec2 texCoords;

uniform ivec2 rawSize;
uniform ivec2 outSize;


vec4 toNDC(vec4 positiveNorm) {
	return (positiveNorm * 2) - 1;
}



void main() {
	
	gl_Position = vec4(pos_in * outSize, 0, 1);
	texCoords = (pos_in * .5) + .5;
}






#shader FRAGMENT

#version 330


layout(location = 0) out vec4 out_Color;

in vec3 gs_tex;
flat in float gs_opacity;
flat in vec4 gs_clip; 

uniform float oversample;
//uniform float magnitude;
uniform float searchSize;
uniform ivec2 rawSize;
uniform ivec2 outSize;
uniform sampler2D rawTex;





int boundedOffset(int x, int y, int ox, int oy, int w, int h) {
	int x1 = x + ox;
	int y1 = y + oy;
	if(x1 < 0 || y1 < 0 || x1 >= w || y1 >= h) return -1;
	return x1 + (w * y1);
}


float sdfEncode(float d, float inside, float maxDist) {
	float o;
	d = sqrt(d);
	float norm = d / maxDist;
	if(inside == 1.0) norm = -norm;
	
	o = (norm * 192.0) + 64.0;
	
	return clamp(o, 0.0, 255.0) / 255.0;
}






void main(void) {
	
	ivec2 center = ivec2(ivec2(gl_FragCoord.xy) * oversample) + ivec2(searchSize, searchSize);

	// value right under the center of the pixel, to determine if we are inside
	// or outside the glyph
	float v = texelFetch(rawTex, center, 0).r;
	
	
	float d = 99999999.99;
	
	ivec2 tp;
	for(tp.y = -int(searchSize); tp.y < int(searchSize); tp.y++) {
		for(tp.x = -int(searchSize); tp.x < int(searchSize); tp.x++) {
			
			float samp = texelFetch(rawTex, center + tp, 0).r;
			
			if(samp != v) {
				d = min(dot(tp, tp), d);
			}
		}
	}
	
	float q = sdfEncode(d, v, searchSize);

	out_Color = vec4(q,0,0,1);
}
