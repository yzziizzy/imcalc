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
uniform float magnitude;
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

/*
 uint8_t sdfEncode(float d, int inside, float maxDist) {
	int o;
	d = sqrt(d);
	float norm = d / maxDist;
	if(inside) norm = -norm;
	
	o = (norm * 192) + 64;
	
	return o < 0 ? 0 : (o > 255 ? 255 : o);
}
*/





void main(void) {
	


	ivec2 center = ivec2(ivec2(gl_FragCoord.xy) * oversample);


	// value right under the center of the pixel, to determine if we are inside
	// or outside the glyph
	vec4 v = texelFetch(rawTex, ivec2(ivec2(gl_FragCoord.xy) * outSize.xy), 0);
	
//	int v = input[sx + (sy * dw)];
	
	
	float d = 99999999.99;
	float e = 0.0;
	
	ivec2 tp;
	for(tp.y = -int(searchSize); tp.y < int(searchSize); tp.y++) {
		for(tp.x = -int(searchSize); tp.x < int(searchSize); tp.x++) {
			
			//length(tp);
			//int off = boundedOffset(sx, sy, ox, oy, dw, dh);
			float off = texelFetch(rawTex, center + tp, 0).r;
			
			e = max(off, e);
			
//			if(off >= 0 && in[off] != v) 
//				d = min(length(tp), d);
		}
	}
	
	//int q = sdfEncode(d, v, maxDist);
	//printf("%d,%d = %d (%f)\n",x,y,q,d);
	
//	output[x + (y * fg->sdfGlyphSize.x)] = q;

	out_Color = vec4(e,e,e,1);
//	out_Color = vec4(d,d,d,1);




/*

	else if(gs_guiType == 1) { // text
		
		float dd;
		float d = dd = texture(fontTex, gs_tex).r;
/*		
		out_Color = vec4(d,d,d, 1.0); 
		return;
		* /
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
	
	*/
	// error fallthrough debug value
	//out_Color = vec4(1,.1,.1, .4);
//	out_Color = v; //ec4(v,.1,.1, 1);
	
	
	
	
	
}
