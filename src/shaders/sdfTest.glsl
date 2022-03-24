#shader VERTEX


#version 330 core

layout (location = 0) in vec2 pos_in;
layout (location = 1) in vec3 tex_in;

uniform ivec2 outSize;
out vec3 vs_tex;

void main() {	
	gl_Position = vec4(pos_in * outSize, 0, 1);
	vs_tex = tex_in;
}






#shader FRAGMENT

#version 330

	
layout(location = 0) out vec4 out_Color;

in vec3 vs_tex;

uniform float stepLow; // used for non-smooth step
uniform float stepHigh;
uniform int stepUseSmooth;
uniform float alphaCutoff;
uniform vec3 fgColor;

uniform sampler2DArray fontTex;


void main(void) {

	float dd;
	float d = dd = texture(fontTex, vs_tex).r;
/*		
	out_Color = vec4(d,d,d, 1.0); 
	return;
	*/
	float a;
	
	
	if(d > .75) {
		d = 1;// (d - .75) * -4;
	}
	else {
		d = (d / 3) * 4;
	}
	d = 1 - d;



	if(stepUseSmooth == 1) {
		a = smoothstep(stepLow, stepHigh, abs(d));
	}
	else {
		a = step(stepLow, abs(d));
	}
	
	if(a < alphaCutoff) {
		discard;
	};
	
	//if(dd < .35) discard;
	out_Color = vec4(fgColor.rgb, a); 
// 	out_Color = vec4(.9,.9,.9, a); 
	
}
