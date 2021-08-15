#shader VERTEX


#version 330 core

layout (location = 0) in vec2 pos_in;


uniform ivec2 targetSize;



vec4 toNDC(vec4 positiveNorm) {
	return (positiveNorm * 2) - 1;
}

uniform vec2 outSize;


void main() {
	
	// convert to NDC
	vertex.lt_rb = toNDC(lt_rb_in / vec4(targetSize.xy, targetSize.xy));
	vertex.lt_rb_abs = vec4(
		lt_rb_in.x,
		targetSize.y - lt_rb_in.y,
		lt_rb_in.z,
		targetSize.y - lt_rb_in.w
	);
	
	gl_Position = toNDC()
}






#shader FRAGMENT

#version 330


layout(location = 0) out float out_Color;

in vec3 gs_tex;
flat in float gs_opacity;
flat in vec4 gs_clip; 

uniform float magnitude;
uniform vec2 rawSize;
uniform sampler2D rawTex;


void main(void) {

	else if(gs_guiType == 1) { // text
		
		float dd;
		float d = dd = texture(fontTex, gs_tex).r;
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

		a = smoothstep(0.35, 0.9, abs(d));
// 		a = step(0.65, abs(d));
		
		if(a < 0.01) {
//  			out_Color = vec4(gs_tex.xy, 0, 1);
// 			return; // show the overdraw
			discard;
		};
		
		//if(dd < .35) discard;
		out_Color = vec4(gs_fg_color.rgb, a); 
// 		out_Color = vec4(.9,.9,.9, a); 
		return;
	}
	else if(gs_guiType == 2) { // simple image
		out_Color = texture(atlasTex, gs_tex);
		return;
	}

	// gradients
	// right triangles
	// diamonds or general rotation
	
	
	// error fallthrough debug value
	out_Color = vec4(1,.1,.1, .4);
	
	
	
	
	
}
