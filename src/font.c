



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <dlfcn.h>
#include <pthread.h>
#include <sys/sysinfo.h>

// FontConfig
#include "fcfg.h"

// for sdf debugging
#include "dumpImage.h"


#include "utilities.h"
#include "font.h"


static FT_Error (*_FT_Init_FreeType)(FT_Library *alibrary);
static FT_Error (*_FT_Set_Pixel_Sizes)(FT_Face face, FT_UInt pixel_width, FT_UInt pixel_height);
static FT_Error (*_FT_Load_Char)(FT_Face face, FT_ULong char_code, FT_Int32 load_flags);
static FT_Error (*_FT_New_Face)(FT_Library library, const char* filepathname, FT_Long face_index, FT_Face *aface);


static FontGen* addChar(int magnitude, FT_Face* ff, int code, int fontSize, char bold, char italic);


// temp
static void addFont(FontManager* fm, char* name);


void sdfgen_new(FontGen* fg);



/*


Magnitude is the distance in pixels for the distance field to be calculated
   around the actual glyph. It is the width of the 'glow' in the sdf.

Oversample is the ratio of the raw raster image size to the desired output sdf.








*/


FontManager* FontManager_alloc(GUI_GlobalSettings* gs) {
	FontManager* fm;
	
	pcalloc(fm);
	HT_init(&fm->fonts, 4);
	fm->magnitude = 8;
	fm->maxAtlasSize = 512;

// BUG: split out gl init operations now
	//FontManager_init(fm, gs);
	
	return fm;
}


void FontManager_init(FontManager* fm, GUI_GlobalSettings* gs) {
	int i = 0;
	int atlas_dirty = 0;
	char* atlas_path = "./fonts.atlas";
	GUIFont* font;

	FontManager_loadAtlas(fm, atlas_path);

	while(gs->fontList[i] != NULL) {
		// printf("checking font: %s\n", gs->Buffer_fontList[i]);
		if(HT_get(&fm->fonts, gs->fontList[i], &font)) {
			atlas_dirty = 1;
			break;
		}
		i++;
	}


	if(1 || atlas_dirty) {
		i = 0;
		while(gs->fontList[i] != NULL) {
			// printf("building font: %s\n", gs->Buffer_fontList[i]);
			FontManager_addFont(fm, gs->fontList[i], 16);
			i++;
		}
		FontManager_finalize(fm);

		FontManager_createAtlas(fm);
		FontManager_saveAtlas(fm, atlas_path);
	}

	HT_get(&fm->fonts, "Arial", &fm->helv);
}




GUIFont* FontManager_findFont(FontManager* fm, char* name) {
	GUIFont* f;
	
	if(HT_get(&fm->fonts, name, &f)) {
		return fm->helv; // fallback
	}
	
	return f;
}







// new font rendering info
//static char* defaultCharset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 `~!@#$%^&*()_+|-=\\{}[]:;<>?,./'\"";
//static char* defaultCharset = "Q";
static char* defaultCharset = "0123456789.+-()";

// 16.16 fixed point to float conversion
static float f2f(int32_t i) {
	return ((double)(i) / 65536.0);
}

// 26.6 fixed point to float conversion
static float f2f26_6(int32_t i) {
	return ((double)(i) / 64.0);
}


static void blit(
	int src_x, int src_y, int dst_x, int dst_y, int w, int h,
	int src_w, int dst_w, unsigned char* src, unsigned char* dst) {
	
	
	int y, x, s, d;
	
	// this may be upside down...
	for(y = 0; y < h; y++) {
		for(x = 0; x < w; x++) {
			s = ((y + src_y) * src_w) + src_x + x;
			d = ((y + dst_y) * dst_w) + dst_x + x;
			
			dst[d] = src[s];
		}
	}
}

static FT_Library ftLib = NULL;

static void checkFTlib() {
	FT_Error err;
	void* lib;
	char* liberr = NULL;
	
	dlerror();
	lib = dlopen("libfreetype.so", RTLD_LAZY | RTLD_GLOBAL);
	liberr = dlerror();
	if(liberr) {
		fprintf(stderr, "Could not load libfreetype: %s\n", liberr);
		exit(1);
	}
	
	_FT_Init_FreeType = dlsym(lib, "FT_Init_FreeType");
	_FT_Set_Pixel_Sizes = dlsym(lib, "FT_Set_Pixel_Sizes");
	_FT_Load_Char = dlsym(lib, "FT_Load_Char");
	_FT_New_Face = dlsym(lib, "FT_New_Face");
	
	if(!ftLib) {
		err = _FT_Init_FreeType(&ftLib);
		if(err) {
			fprintf(stderr, "Could not initialize FreeType library.\n");
			return;
		}
	}
}


typedef struct fb_info {
	GLuint fb_id, tex_id;
} fb_info;

typedef struct quad_info {
	GLuint vao, vbo;
} quad_info;


typedef struct {
	GLuint vao, vbo;
	GLuint fb_tex_id;
	GLuint fb_id;
	ShaderProgram* shader;	
} gpu_calc_info;


// returns fbo name 
// leaves the new framebuffer bound
void mk_fbo(fb_info* fb, int size_x, int size_y, GLuint format, GLuint fmt_width) {

	GLenum status;
	GLuint fbid;
	GLuint texid;
	
	// backing texture	
	glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_2D, texid);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	// allocate storage
	glTexImage2D(
		GL_TEXTURE_2D, 0, format, 
		size_x, size_y, 
		0, format, fmt_width, NULL
	);
	
	
	// the fbo itself
	glGenFramebuffers(1, &fbid);
	glBindFramebuffer(GL_FRAMEBUFFER, fbid);
	glexit("sdf fbo creation");
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texid, 0);
	glexit("");
	
	GLenum db = GL_COLOR_ATTACHMENT0;
	glDrawBuffers(1, &db);
	glexit("");
	
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE) {
		printf("fbo status invalid\n");
		exit(1);
	}
	
	fb->fb_id = fbid;
	fb->tex_id = texid;
}

void mk_quad(quad_info* qd) {

	glexit("");

	// quad
	float vertices[] = {
		-1.0, -1.0,
		-1.0, 1.0,
		1.0, -1.0,
		1.0, 1.0
	};
	
	glGenVertexArrays(1, &qd->vao);
	glBindVertexArray(qd->vao);
	glexit("");
	
	glGenBuffers(1, &qd->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, qd->vbo);
	
	glEnableVertexAttribArray(0);
	glexit("");
	
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8, 0);
	glexit("");
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glexit("");
}


void init_gpu_sdf(gpu_calc_info* gpu, int magnitude, int sz_x, int sz_y) {
	
	if(0 >= sz_x * sz_y) {
		fprintf(stderr, "cannot generate font: zero raw size\n");
		exit(1);
	} 
	
	glexit("");

	quad_info qd;
	mk_quad(&qd);
	gpu->vao = qd.vao;
	gpu->vbo = qd.vbo;
	
	fb_info fb;
	mk_fbo(&fb, sz_x, sz_y, GL_RED, GL_UNSIGNED_BYTE);
	gpu->fb_id = fb.fb_id;
	gpu->fb_tex_id = fb.tex_id;
	
	// shader
	gpu->shader = loadCombinedProgram("sdfGen");
	glUseProgram(gpu->shader->id);
	glexit("shading prog");

	glexit("");
	glUniform1f(glGetUniformLocation(gpu->shader->id, "searchSize"), magnitude);
	printf("searchSize/mag: %d\n", magnitude);
	
	// other properties
	glActiveTexture(GL_TEXTURE0 + 0);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glexit(""); 
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glexit("");
	
	// set to handle the largest texture
	glViewport(0, 0, sz_x, sz_y);
	glexit("");
	// everything left bound on purpose
}

void destroy_gpu_sdf(gpu_calc_info* gpu) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1,  &gpu->fb_id);
	glDeleteTextures(1,  &gpu->fb_tex_id);
	
	glDeleteBuffers(1, &gpu->vbo);
	glDeleteVertexArrays(1, &gpu->vao);
}


void CalcSDF_GPU(gpu_calc_info* gpu, FontGen* fg) {
	
	glClear(GL_COLOR_BUFFER_BIT);
	glexit("");
	// input data texture of raw glyph
	GLuint rawID;
	glexit("");
	
	glGenTextures(1, &rawID);
	glBindTexture(GL_TEXTURE_2D, rawID);
	
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glexit("");
	
	glTexImage2D(GL_TEXTURE_2D, // target
		0,  // level, 0 = base, no minimap,
		GL_RED, // internalformat
		fg->rawGlyphSize.x,
		fg->rawGlyphSize.y,
		0,  // border
		GL_RED,  // format
		GL_UNSIGNED_BYTE, // input type
		fg->rawGlyph);
	
	glexit("");
	
		
	glUniform2iv(glGetUniformLocation(gpu->shader->id, "outSize"), 1, (int*)&fg->sdfGlyphSize);
	printf("outSize: %d,%d\n", fg->sdfGlyphSize.x, fg->sdfGlyphSize.y);
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glexit("quad draw");
	
	// fetch the results. this call will flush the pipeline implicitly	
	fg->sdfGlyph = malloc(fg->sdfGlyphSize.x * fg->sdfGlyphSize.y * sizeof(*fg->sdfGlyph));
	glBindBuffer( GL_PIXEL_PACK_BUFFER, 0);
	glReadPixels(0,0,	
		fg->sdfGlyphSize.x,
		fg->sdfGlyphSize.y,
		GL_RED,
		GL_UNSIGNED_BYTE,
		fg->sdfGlyph
	);
	
	printf("code: %d, sz: %d,%d\n", fg->code, fg->sdfGlyphSize.x, fg->sdfGlyphSize.y);
	
	/*
	{
		static char buf[200];
		sprintf(buf, "raw-test-%d.png", fg->code);
		writePNG(buf, 1, fg->rawGlyph, fg->rawGlyphSize.x, fg->rawGlyphSize.y);
	}
	{
		static char buf[200];
		sprintf(buf, "sdf-test-%d.png", fg->code);
		writePNG(buf, 1, fg->sdfGlyph, fg->sdfGlyphSize.x, fg->sdfGlyphSize.y);
	}
	*/
	
	glDeleteTextures(1, &rawID);
	
	int x, y;
	
	// find the bounds of the sdf data
	// first rows
	for(y = 0; y < fg->sdfGlyphSize.y; y++) {
		int hasData = 0;
		for(x = 0; x < fg->sdfGlyphSize.x; x++) {
			hasData += fg->sdfGlyph[x + (y * fg->sdfGlyphSize.x)];
		}
		
		if(hasData / fg->sdfGlyphSize.x < 255) {
			fg->sdfBounds.min.y = y;
			break;
		}
	}
	for(y = fg->sdfGlyphSize.y - 1; y >= 0; y--) {
		int hasData = 0;
		for(x = 0; x < fg->sdfGlyphSize.x; x++) {
			hasData += fg->sdfGlyph[x + (y * fg->sdfGlyphSize.x)];
		}
		
		if(hasData / fg->sdfGlyphSize.x < 255) {
			fg->sdfBounds.max.y = y + 1;
			break;
		}
	}

	for(x = 0; x < fg->sdfGlyphSize.x; x++) {
		int hasData = 0;
		for(y = 0; y < fg->sdfGlyphSize.y; y++) {
			hasData += fg->sdfGlyph[x + (y * fg->sdfGlyphSize.x)];
		}
		
		if(hasData / fg->sdfGlyphSize.y < 255) {
			fg->sdfBounds.min.x = x;
			break;
		}
	}
	for(x = fg->sdfGlyphSize.x - 1; x >= 0; x++) {
		int hasData = 0;
		for(y = 0; y < fg->sdfGlyphSize.y; y++) {
			hasData += fg->sdfGlyph[x + (y * fg->sdfGlyphSize.x)];
		}
		
		if(hasData / fg->sdfGlyphSize.y < 255) {
			fg->sdfBounds.max.x = x + 1;
			break;
		}
	}
	
	fg->sdfDataSize.x = fg->sdfBounds.max.x - fg->sdfBounds.min.x;
	fg->sdfDataSize.y = fg->sdfBounds.max.y - fg->sdfBounds.min.y;
	
	free(fg->rawGlyph);
	
}




void gen_sdf_test_samples(char* fontName, int code) {
	printf("\n\n-------------------\nGenerating font samples\n-------------------\n\n");
				
	checkFTlib();
	
	ShaderProgram* test_shader = loadCombinedProgram("sdfTest");
	
	quad_info qd;
	mk_quad(&qd);			
	
	for(int size = 32; size <= 32; size += 4) {
		for(int bold = 0; bold <= 0; bold++) {
			for(int italic = 0; italic <= 0; italic++) {
		
				FT_Error err;
				FT_Face fontFace;
				
				char* fontPath = getFontFile2(fontName, bold, italic);
				if(!fontPath) {
					fprintf(stderr, "Could not load font '%s'\n", fontName);
					return;
				}
				printf("font path: %s: %s\n", fontName, fontPath);
				
				err = _FT_New_Face(ftLib, fontPath, 0, &fontFace);
				if(err) {
					fprintf(stderr, "Could not access font '%s' at '%s'.\n", fontName, fontPath);
					return;
				}
				
			
				
			//	f->ascender = fontFace->ascender >> 6;
			//	f->descender = fontFace->descender >> 6;
			//	f->height = fontFace->height >> 6;
				
				
				for(int magnitude = 2; magnitude < 8; magnitude += 2) {
					
					//
					// generate the single sdf image
					//
			
					gpu_calc_info gpu;
					
					
	//     		 printf("calc: '%s':%d:%d %c\n", name, bold, italic, defaultCharset[i]);
	//	  		gensize is the desired native font size 
					FontGen* fg = addChar(magnitude , &fontFace, code, size, bold, italic);
					//fg->font = f;
					
					{
						static char buf[200];
						sprintf(buf, "raw-glyph-%d-[%d]-%s%s.png", fg->code, size, bold?"b":"", italic?"i":"");
						writePNG(buf, 1, fg->rawGlyph, fg->rawGlyphSize.x, fg->rawGlyphSize.y);
					}
					
					
					
					sdfgen_new(fg);
					exit(2);
					
						
					init_gpu_sdf(&gpu, magnitude, fg->rawGlyphSize.x, fg->rawGlyphSize.y);
					
					printf("gpucalc: '%s':%d:%d %c\n", fontName, fg->bold, fg->italic, fg->code);
					CalcSDF_GPU(&gpu, fg);
					
					{
						static char buf[200];
						sprintf(buf, "sdf-test-%d-[%d-%d-%s%s].png", fg->code, size, magnitude, bold?"b":"", italic?"i":"");
						writePNG(buf, 1, fg->sdfGlyph, fg->sdfGlyphSize.x, fg->sdfGlyphSize.y);
					}
					
					destroy_gpu_sdf(&gpu);
					
					
					//
					// render some samples
					//
					
					// load the trimmed sdf texture
					GLuint rawID;
					glexit("");
					
					glGenTextures(1, &rawID);
					glBindTexture(GL_TEXTURE_2D, rawID);
					
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					glexit("");
					
					glTexImage2D(GL_TEXTURE_2D, // target
						0,  // level, 0 = base, no minimap,
						GL_RED, // internalformat
						fg->rawGlyphSize.x,
						fg->rawGlyphSize.y,
						0,  // border
						GL_RED,  // format
						GL_UNSIGNED_BYTE, // input type
						fg->rawGlyph);
	
					
					for(int sampleSize = 8; sampleSize <= 8; sampleSize += 2) {
						for(int useSmooth = 0; useSmooth <= 1; useSmooth++) {
							for(float stepLow = 0.4; stepLow <= .7; stepLow += 0.1) {
								for(float stepHigh = 0.4; stepHigh <= .7; stepHigh += 0.1) {
						
									// the fbo
									fb_info fb;
									mk_fbo(&fb, fg->sdfGlyphSize.x, fg->sdfGlyphSize.y, GL_RED, GL_UNSIGNED_BYTE);
									
									// the shader
									glUseProgram(test_shader->id);
									glexit("test shading prog");
									
									// the quad
									glBindVertexArray(qd.vao);
									glBindBuffer(GL_ARRAY_BUFFER, qd.vbo);
									glEnableVertexAttribArray(0);
									glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8, 0);
									glexit("");
									
									glClear(GL_COLOR_BUFFER_BIT);
									glexit("");
									
								
									
									
									glUniform2iv(glGetUniformLocation(gpu.shader->id, "outSize"), 1, (int*)&fg->sdfGlyphSize);
									printf("outSize: %d,%d\n", fg->sdfGlyphSize.x, fg->sdfGlyphSize.y);
									
									glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
									glexit("quad draw");
									
									// fetch the results. this call will flush the pipeline implicitly	
									fg->sdfGlyph = malloc(fg->sdfGlyphSize.x * fg->sdfGlyphSize.y * sizeof(*fg->sdfGlyph));
									glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
									glReadPixels(0,0,	
										fg->sdfGlyphSize.x,
										fg->sdfGlyphSize.y,
										GL_RED,
										GL_UNSIGNED_BYTE,
										fg->sdfGlyph
									);
											
					
									{
										static char buf[200];
										sprintf(buf, "render-%d-[%d-%d-%s%s]-%0.1f,%0.1f,%s.png", 
											fg->code, size, magnitude, bold?"b":"", italic?"i":"", 
											stepLow, stepHigh, useSmooth?"sm":"");
										writePNG(buf, 1, fg->sdfGlyph, fg->sdfGlyphSize.x, fg->sdfGlyphSize.y);
									}
					
								} // stepHigh
							} // stepLow
						} // useSmooth
					} // sampleSize
							
							
				} // magnitude
			} // italic
		} // bold
	} // size
	
}












static float dist(int a, int b) {
	return a*a + b*b;
}
static float dmin(int a, int b, float d) {
	return fmin(dist(a, b), d);
}

static int boundedOffset(int x, int y, int ox, int oy, int w, int h) {
	int x1 = x + ox;
	int y1 = y + oy;
	if(x1 < 0 || y1 < 0 || x1 >= w || y1 >= h) return -1;
	return x1 + (w * y1);
}

static uint8_t sdfEncode(float d, int inside, float maxDist) {
	int o;
	d = sqrt(d);
	float norm = d / maxDist;
	if(inside) norm = -norm;
	
	o = (norm * 192) + 64;
	
	return o < 0 ? 0 : (o > 255 ? 255 : o);
}

void CalcSDF_Software_(FontGen* fg) {
	
	int searchSize;
	int x, y, ox, oy, sx, sy;
	int dw, dh;
	
	uint8_t* input;
	uint8_t* output;
	
	float d, maxDist;
	
	searchSize = fg->magnitude;
	maxDist = 0.5 * searchSize;
	dw = fg->rawGlyphSize.x;
	dh = fg->rawGlyphSize.y;
	
	// this is wrong(?)
	fg->sdfGlyphSize.x = dw;//floor(((float)(dw/* + (2*fg->magnitude)*/) / (float)()) + .5);
	fg->sdfGlyphSize.y = dh;//floor(((float)(dh/* + (2*fg->magnitude)*/) / (float)(fg->oversample)) + .5); 
	
	fg->sdfGlyph = output = malloc(fg->sdfGlyphSize.x * fg->sdfGlyphSize.y * sizeof(uint8_t));
	input = fg->rawGlyph;
	
	fg->sdfBounds.min.x = 0;
	fg->sdfBounds.max.y = fg->sdfGlyphSize.y;
	fg->sdfBounds.max.x = fg->sdfGlyphSize.x; 
	fg->sdfBounds.min.y = 0;
	
	// calculate the sdf 
	for(y = 0; y < fg->sdfGlyphSize.y; y++) {
		for(x = 0; x < fg->sdfGlyphSize.x; x++) {
			int sx = x;
			int sy = y;
			//printf(".");
			// value right under the center of the pixel, to determine if we are inside
			// or outside the glyph
			int v = input[sx + (sy * dw)];
			
			d = 999999.99999;
			
			
			for(oy = -searchSize / 2; oy < searchSize; oy++) {
				for(ox = -searchSize / 2; ox < searchSize; ox++) {
					int off = boundedOffset(sx, sy, ox, oy, dw, dh);
					if(off >= 0 && input[off] != v) 
						d = dmin(ox, oy, d);
				}
			}
			
			int q = sdfEncode(d, v, maxDist);
			//printf("%d,%d = %d (%f)\n",x,y,q,d);
			
			output[x + (y * fg->sdfGlyphSize.x)] = q;
		}
	}
	
	
	// find the bounds of the sdf data
	// first rows
	for(y = 0; y < fg->sdfGlyphSize.y; y++) {
		int hasData = 0;
		for(x = 0; x < fg->sdfGlyphSize.x; x++) {
			hasData += output[x + (y * fg->sdfGlyphSize.x)];
		}
		
		if(hasData / fg->sdfGlyphSize.x < 255) {
			fg->sdfBounds.min.y = y;
			break;
		}
	}
	for(y = fg->sdfGlyphSize.y - 1; y >= 0; y--) {
		int hasData = 0;
		for(x = 0; x < fg->sdfGlyphSize.x; x++) {
			hasData += output[x + (y * fg->sdfGlyphSize.x)];
		}
		
		if(hasData / fg->sdfGlyphSize.x < 255) {
			fg->sdfBounds.max.y = y + 1;
			break;
		}
	}

	for(x = 0; x < fg->sdfGlyphSize.x; x++) {
		int hasData = 0;
		for(y = 0; y < fg->sdfGlyphSize.y; y++) {
			hasData += output[x + (y * fg->sdfGlyphSize.x)];
		}
		
		if(hasData / fg->sdfGlyphSize.y < 255) {
			fg->sdfBounds.min.x = x;
			break;
		}
	}
	for(x = fg->sdfGlyphSize.x - 1; x >= 0; x++) {
		int hasData = 0;
		for(y = 0; y < fg->sdfGlyphSize.y; y++) {
			hasData += output[x + (y * fg->sdfGlyphSize.x)];
		}
		
		if(hasData / fg->sdfGlyphSize.y < 255) {
			fg->sdfBounds.max.x = x + 1;
			break;
		}
	}
	
	fg->sdfDataSize.x = fg->sdfBounds.max.x - fg->sdfBounds.min.x;
	fg->sdfDataSize.y = fg->sdfBounds.max.y - fg->sdfBounds.min.y;
	
	free(fg->rawGlyph);
}




static FontGen* addChar(int magnitude, FT_Face* ff, int code, int fontSize, char bold, char italic) {
	FontGen* fg;
	FT_Error err;
	FT_GlyphSlot slot;
	
	pcalloc(fg);
	fg->code = code;
	fg->italic = italic;
	fg->bold = bold;
	fg->magnitude = magnitude;
	
//	int rawSize = fontSize * fm->oversample;
	
	
	err = _FT_Set_Pixel_Sizes(*ff, 0, fontSize);
	if(err) {
		fprintf(stderr, "Could not set pixel size to %dpx.\n", fontSize);
		free(fg);
		return NULL;
	}
	
	
	err = _FT_Load_Char(*ff, code, FT_LOAD_DEFAULT | FT_LOAD_MONOCHROME);
	
	//f2f(slot->metrics.horiBearingY);
	
	// draw character to freetype's internal buffer and copy it here
	_FT_Load_Char(*ff, code, FT_LOAD_RENDER);
	// slot is a pointer
	slot = (*ff)->glyph;
	
	// typographic metrics for later. has nothing to do with sdf generation
	fg->rawAdvance = f2f(slot->linearHoriAdvance); 
	fg->rawBearing.x = f2f26_6(slot->metrics.horiBearingX); 
	fg->rawBearing.y = f2f26_6(slot->metrics.horiBearingY); 
	
	// back to sdf generation
	Vector2i rawImgSz = {(slot->metrics.width >> 6), (slot->metrics.height >> 6)};
	
	fg->rawGlyphSize.x = (slot->metrics.width >> 6) + (fg->magnitude); 
	fg->rawGlyphSize.y = (slot->metrics.height >> 6) + (fg->magnitude); 
	
	// the raw glyph is copied to the middle of a larger buffer to make the sdf algorithm simpler 
	fg->rawGlyph = calloc(1, sizeof(*fg->rawGlyph) * fg->rawGlyphSize.x * fg->rawGlyphSize.y);
	
	blit(
		0, 0, // src x and y offset for the image
		 (fg->magnitude * .5), (fg->magnitude * .5), // dst offset
		rawImgSz.x, rawImgSz.y, // width and height
		slot->bitmap.pitch, fg->rawGlyphSize.x, // src and dst row widths
		slot->bitmap.buffer, // source
		fg->rawGlyph); // destination
	
	int dw = fg->rawGlyphSize.x;
	int dh = fg->rawGlyphSize.y;
	
	fg->sdfGlyphSize.x = dw;//floor(((float)dw / (float)fg->oversample) + .5);
	fg->sdfGlyphSize.y = dh;//floor(((float)dh / (float)fg->oversample) + .5); 
	
	
	/*
	/// TODO move to multithreaded pass
	CalcSDF_Software_(fg);
	
	// done with the raw data
	free(fg->rawGlyph);
	*/
	
	/*
	
	printf("raw size: %d, %d\n", fg->rawGlyphSize.x, fg->rawGlyphSize.y);
	printf("sdf size: %d, %d\n", fg->sdfGlyphSize.x, fg->sdfGlyphSize.y);
	printf("bounds: %f,%f, %f,%f\n",
		fg->sdfBounds.min.x,
		fg->sdfBounds.min.y,
		fg->sdfBounds.max.x,
		fg->sdfBounds.max.y
		   
	);

	writePNG("sdf-raw.png", 1, fg->rawGlyph, fg->rawGlyphSize.x, fg->rawGlyphSize.y);
	writePNG("sdf-pct.png", 1, fg->sdfGlyph, fg->sdfGlyphSize.x, fg->sdfGlyphSize.y);
	*/
	
	return fg;
}


void* sdf_thread(void* _fm) {
	FontManager* fm = (FontManager*)_fm;
	
	int i = 0;
	while(1) {
		i = atomic_fetch_add(&fm->genCounter, 1);
		
		if(i >= VEC_LEN(&fm->gen)) break;
		
		FontGen* fg = VEC_ITEM(&fm->gen, i);
		printf("calc: '%s':%d:%d %c\n", fg->font->name, fg->bold, fg->italic, fg->code);
		CalcSDF_Software_(fg);
		
	}
	
	
	pthread_exit(NULL);
}

void FontManager_finalize(FontManager* fm) {
	
	
	if(0) { // cpu calculation
		int maxThreads = get_nprocs();
		pthread_t threads[maxThreads];
		
		for(int i = 0; i < maxThreads; i++) {
			int ret = pthread_create(&threads[i], NULL, sdf_thread, fm);
			if(ret) {
				printf("failed to spawn thread in FontManager\n");
				exit(1);
			}
		}
		
		// wait for the work to get done
		for(int i = 0; i < maxThreads; i++) {
			pthread_join(threads[i], NULL);
		}
	}
	else {
		// gpu calculation
		
		gpu_calc_info gpu;
		
		init_gpu_sdf(&gpu, fm->magnitude, fm->maxRawSize.x, fm->maxRawSize.y);	
	
		VEC_EACH(&fm->gen, i, fg) {
		
			printf("gpucalc: '%s':%d:%d %c\n", fg->font->name, fg->bold, fg->italic, fg->code);
			CalcSDF_GPU(&gpu, fg);
		}
		
		destroy_gpu_sdf(&gpu);
	}
}


// sorting function for fontgen array
static int gen_comp(const void* aa, const void * bb) {
	FontGen* a = *((FontGen**)aa);
	FontGen* b = *((FontGen**)bb);
	
	if(a->sdfDataSize.y == b->sdfDataSize.y) {
		return b->sdfDataSize.x - a->sdfDataSize.x;
	}
	else {
		return b->sdfDataSize.y - a->sdfDataSize.y;
	}
}



GUIFont* GUIFont_alloc(char* name) {
	GUIFont* f;
	
	pcalloc(f);
	
	f->name = strdup(name);
	f->charsLen = 128;
	f->regular = calloc(1, sizeof(*f->regular) * f->charsLen);
	f->bold = calloc(1, sizeof(*f->bold) * f->charsLen);
	f->italic= calloc(1, sizeof(*f->italic) * f->charsLen);
	f->boldItalic = calloc(1, sizeof(*f->boldItalic) * f->charsLen);
	
	return f;
}



void FontManager_addFont2(FontManager* fm, char* name, char bold, char italic, int genSize) {
	GUIFont* f;
	FT_Error err;
	FT_Face fontFace;
	
	//defaultCharset = "I";
	
	int len = strlen(defaultCharset);
	
	//int fontSize = 32; // pixels
	
	checkFTlib();
	
	// TODO: load font
	char* fontPath = getFontFile2(name, bold, italic);
	if(!fontPath) {
		fprintf(stderr, "Could not load font '%s'\n", name);
		return;
	}
	printf("font path: %s: %s\n", name, fontPath);

	err = _FT_New_Face(ftLib, fontPath, 0, &fontFace);
	if(err) {
		fprintf(stderr, "Could not access font '%s' at '%s'.\n", name, fontPath);
		return;
	}
	
	if(HT_get(&fm->fonts, name, &f)) {
		f = GUIFont_alloc(name);
		HT_set(&fm->fonts, name, f);
	}
	
	f->ascender = fontFace->ascender >> 6;
	f->descender = fontFace->descender >> 6;
	f->height = fontFace->height >> 6;
	
	for(int i = 0; i < len; i++) {
// 		printf("calc: '%s':%d:%d %c\n", name, bold, italic, defaultCharset[i]);
		FontGen* fg = addChar(fm->magnitude, &fontFace, defaultCharset[i], genSize, bold, italic);
		fg->font = f;
		
		fm->maxRawSize.x = MAX(fm->maxRawSize.x, fg->rawGlyphSize.x);
		fm->maxRawSize.y = MAX(fm->maxRawSize.y, fg->rawGlyphSize.y);
		
		VEC_PUSH(&fm->gen, fg);
	}
	
	

}

void FontManager_addFont(FontManager* fm, char* name, int genSize) {
	FontManager_addFont2(fm, name, 0, 0, genSize);
// 	FontManager_addFont2(fm, name, 1, 0); // DEBUG: temporarily disabled for testing metrics
// 	FontManager_addFont2(fm, name, 0, 1);
// 	FontManager_addFont2(fm, name, 1, 1);
}

void FontManager_createAtlas(FontManager* fm) {
	char buf[32];
	
	// order the characters by height then width, tallest and widest first.
	VEC_SORT(&fm->gen, gen_comp);
	
	int totalWidth = 0;
	VEC_EACH(&fm->gen, ind, gen) {
//		printf("%c: h: %d, w: %d \n", gen->code, gen->sdfDataSize.y, gen->sdfDataSize.x);
		totalWidth += gen->sdfDataSize.x;
	}
	
	int maxHeight = VEC_ITEM(&fm->gen, 0)->sdfDataSize.y;
	int naiveSize = ceil(sqrt(maxHeight * totalWidth));
	int pot = nextPOT(naiveSize);
	int pot2 = naiveSize / 2;
	
	printf("naive min tex size: %d -> %d (%d)\n", naiveSize, pot, totalWidth);
	
	pot = MIN(pot, fm->maxAtlasSize);
	
	
	// test the packing
	int row = 0;
	int hext = maxHeight;
	int rowWidth = 0;
	
	// copy the chars into the atlas, cleaning as we go
	uint8_t* texData = malloc(sizeof(*texData) * pot * pot);
	memset(texData, 255, sizeof(*texData) * pot * pot);
	fm->atlasSize = pot;
	
	
	row = 0;
	hext = 0;
	int prevhext = maxHeight;
	rowWidth = 0;
	VEC_EACH(&fm->gen, ind, gen) {
		
		if(rowWidth + gen->sdfDataSize.x > pot) {
			row++;
			rowWidth = 0;
			hext += prevhext;
			prevhext = gen->sdfDataSize.y;
			
			// next texture
			if(hext + prevhext > pot) { 
				VEC_PUSH(&fm->atlas, texData);
				
				// disabled for debug
				//sprintf(buf, "sdf-comp-%ld.png", VEC_LEN(&fm->atlas));
				//writePNG(buf, 1, texData, pot, pot);
				
				
				texData = malloc(sizeof(*texData) * pot * pot);
				// make everything white, the "empty" value
				memset(texData, 255, sizeof(*texData) * pot * pot);
				
				hext = 0;
			}
		}
		
		// blit the sdf bitmap data
		blit(
			gen->sdfBounds.min.x, gen->sdfBounds.min.y, // src x and y offset for the image
			rowWidth, hext, // dst offset
			gen->sdfDataSize.x, gen->sdfDataSize.y, // width and height
			gen->sdfGlyphSize.x, pot, // src and dst row widths
			gen->sdfGlyph, // source
			texData); // destination
		
		
		// copy info over to font
		struct charInfo* c;
		if(gen->bold && gen->italic) c = &gen->font->boldItalic[gen->code];
		else if(gen->bold) c = &gen->font->bold[gen->code];
		else if(gen->italic) c = &gen->font->italic[gen->code];
		else c = &gen->font->regular[gen->code];
		
		c->texIndex = VEC_LEN(&fm->atlas);
		c->texelOffset.x = rowWidth;
		c->texelOffset.y = hext;
		c->texelSize = gen->sdfDataSize;
		c->texNormOffset.x = (float)rowWidth / (float)pot;
		c->texNormOffset.y = (float)hext / (float)pot;
		c->texNormSize.x = (float)gen->sdfDataSize.x / (float)pot;
		c->texNormSize.y = (float)gen->sdfDataSize.y / (float)pot;
		
		// BUG: wrong? needs magnitude?
		c->advance = gen->rawAdvance;
		c->topLeftOffset.x = (gen->rawBearing.x);// + (float)gen->sdfBounds.min.x;
		c->topLeftOffset.y = (gen->rawBearing.y);// - (float)gen->sdfBounds.min.y;
		
		c->size = gen->genSize;
//		c->genSize.y = gen->sdfDataSize.y;
		
//		printf("toff: %f, %f \n", c->texNormOffset.x, c->texNormOffset.y);
//		printf("tsize: %f, %f \n", c->texNormSize.x, c->texNormSize.y);
//		printf("ltoff: %f, %f \n", c->topLeftOffset.x, c->topLeftOffset.y);
		
		// advance the write offset
		rowWidth += gen->sdfDataSize.x;
		
		// clean up the FontGen struct
		free(gen->sdfGlyph);
		free(gen);
	}
	
	
	VEC_PUSH(&fm->atlas, texData);
	
	// disabled for debugging
	//sprintf(buf, "sdf-comp-%ld.png", VEC_LEN(&fm->atlas));
	//writePNG(buf, 1, texData, pot, pot);
	
	VEC_FREE(&fm->gen);
	
	
// 	writePNG("sdf-comp.png", 1, texData, pot, pot);

	
	//exit(1);
	
	
}


// bump on format changes. there is no backward compatibility. saving is for caching only.
static uint16_t GUIFONT_ATLAS_FILE_VERSION = 3;

void FontManager_saveAtlas(FontManager* fm, char* path) {
	FILE* f;
	uint16_t u16;
	
	f = fopen(path, "wb");
	if(!f) {
		fprintf(stderr, "Could not save font atlas to '%s'\n", path);
		return;
	}
	
	// write the file version
	fwrite(&GUIFONT_ATLAS_FILE_VERSION, 1, 2, f);
	
	HT_EACH(&fm->fonts, fName, GUIFont*, font) {
		// save the font
		// font identifier
		fwrite("F", 1, 1, f);
		
		// name length
		uint16_t nlen = strlen(font->name); 
		fwrite(&nlen, 1, 2, f);
		
		//name 
		fwrite(font->name, 1, nlen, f);
		
		// global metrics
		fwrite(&font->ascender, 1, 4, f);
		fwrite(&font->descender, 1, 4, f);
		fwrite(&font->height, 1, 4, f);
		
		// number of charInfo structs
		uint32_t clen = font->charsLen;
		fwrite(&clen, 1, 4, f);
		
		// the charInfo structs
		fwrite(font->regular, 1, clen * sizeof(*font->regular), f);
		fwrite(font->bold, 1, clen * sizeof(*font->bold), f);
		fwrite(font->italic, 1, clen * sizeof(*font->italic), f);
		fwrite(font->boldItalic, 1, clen * sizeof(*font->boldItalic), f);
	}
	
	// atlas identifier
	fwrite("A", 1, 1, f);
	
	// max atlas size
	fwrite(&fm->maxAtlasSize, 1, 4, f);
	
	// number of atlas layers
	u16 = VEC_LEN(&fm->atlas);
	fwrite(&u16, 1, 2, f);
	
	// atlas dimension (always square)
	fwrite(&fm->atlasSize, 1, 4, f);
	
	// atlas data
	VEC_EACH(&fm->atlas, ind, at) {
		fwrite(at, 1, fm->atlasSize * fm->atlasSize * sizeof(*at), f);
	}
	
	
	// done
	fclose(f);
}

int FontManager_loadAtlas(FontManager* fm, char* path) {
	FILE* f;
	
	f = fopen(path, "rb");
	if(!f) {
		fprintf(stderr, "Could not open font atlas '%s'\n", path);
		return 1;
	}
	
	
	uint8_t u8;
	uint16_t u16;
	uint32_t u32;
	
	// check the file version
	int r = fread(&u16, 1, 2, f);
	if(u16 != GUIFONT_ATLAS_FILE_VERSION) {
		printf("Font atlas file version mismatch. %d != %d, %d, '%s' \n", (int)u16, GUIFONT_ATLAS_FILE_VERSION, r, path);
		fclose(f);
		return 1;
	}
	
	while(!feof(f)) {
		// check the sigil sigil
		fread(&u8, 1, 1, f);
		
		if(u8 == 'F') {
			
			GUIFont* gf = calloc(1, sizeof(*gf)); 
			
			// name length and name string
			fread(&u16, 1, 2, f);
			gf->name = malloc(u16 + 1);
			fread(gf->name, 1, u16, f);
			gf->name[u16] = 0;
			
			HT_set(&fm->fonts, gf->name, gf);
			
			// global metrics
			fread(&gf->ascender, 1, 4, f);
			fread(&gf->descender, 1, 4, f);
			fread(&gf->height, 1, 4, f);
			
			// charInfo array length
			fread(&u32, 1, 4, f);
			gf->charsLen = u32;
			gf->regular = malloc(u32 * sizeof(*gf->regular));
			gf->bold = malloc(u32 * sizeof(*gf->bold));
			gf->italic = malloc(u32 * sizeof(*gf->italic));
			gf->boldItalic = malloc(u32 * sizeof(*gf->boldItalic));
			
			// charInfo structs
			fread(gf->regular, 1, u32 * sizeof(*gf->regular), f);
			fread(gf->bold, 1, u32 * sizeof(*gf->bold), f);
			fread(gf->italic, 1, u32 * sizeof(*gf->italic), f);
			fread(gf->boldItalic, 1, u32 * sizeof(*gf->boldItalic), f);
			
		}
		else if(u8 == 'A') { // atlas
			
			// max atlas size
			fread(&u32, 1, 4, f);
			fm->maxAtlasSize = u32;
			
			// number of layers
			fread(&u16, 1, 2, f);
			int layerNum = u16;
			
			// atlas dimension
			fread(&u32, 1, 4, f);
			fm->atlasSize = u32;
			
			printf("atlas size: %d\n", u32);

			// atlas data
			for(int i = 0; i < layerNum; i++) {
				uint8_t* at;
				
				at = malloc(u32 * u32 * sizeof(*at));
				fread(at, 1, u32 * u32 * 1, f);
				VEC_PUSH(&fm->atlas, at);
			}
			
			break;
		}
	}
	
	fclose(f);
	
	
	HT_get(&fm->fonts, "Arial", &fm->helv);
	
	return 0;
}






void sdfgen_new(FontGen* fg) {

	char* input;
	int in_size_x = fg->rawGlyphSize.x, in_size_y = fg->rawGlyphSize.y; // size of the input image, in input pixels
	printf("input size: %d,%d\n", in_size_x, in_size_y);

	// out_mag is the size the field is calculated away from the true edge,
	//   in output pixels
	int out_mag = 8;

	

	//// ----- Init the Output -----

	// number of input pixels inside each output pixel
	int io_ratio = floor(192.0 / (float)out_mag);
	printf("io_ratio: %d\n", io_ratio);
	
	
	// how big the core of the output needs to be, in real-valued output pixels
	float out_needed_xf = (float)in_size_x / io_ratio; 
	float out_needed_yf = (float)in_size_y / io_ratio; 
	printf("out needed: %.2f,%.2f\n", out_needed_xf, out_needed_yf);
	
	int out_padding = out_mag;
	
	// size of the output image, in output pixels, including padding
	int out_size_x = ceil(out_needed_xf) + (out_padding * 2);
	int out_size_y = ceil(out_needed_yf) + (out_padding * 2);
	printf("outsize: %d,%d\n", out_size_x, out_size_y);
	
	char* output = malloc(out_size_x * out_size_y * sizeof(output));
	
	
	//// ----- Geometric Utilities -----
	
	// offset to the center of an output pixel from the TL corner
	float io_center_off = (float)io_ratio / 2.0;
	
	#define INPX(x, y)  input[(x + (y * in_size_x))]
	#define OUTLOC(x, y)  (x + (y * out_size_x))
	
	
	
	//// ----- Output Color Cache -----
	
	char* out_center_cache = calloc(1, out_size_x * out_size_y * sizeof(*out_center_cache));
	
	// TODO: only the core, non-padded area
	for(int oy = 0; oy < out_size_y; oy++) {
	for(int ox = 0; ox < out_size_x; ox++) {
		
		// very wrong
		out_center_cache[OUTLOC(ox, oy)] = INPX(ox, oy);
		
	}}
	
	
	{
		static char buf[200];
		sprintf(buf, "output-center-cache-%d.png", fg->code);
		writePNG(buf, 1, out_center_cache, out_size_x, out_size_y);
	}
	
	
	//// ----- SDF Generation -----
	
	// Scan each input line looking for black pixels that touch white pixels.
	// When found, update output pixels with new min distance to an edge in
	// a radius around this point. In/out calculation is determined by the 
	// color of the pixel underneath the output pixel center. All distances
	// are to pixel centers.   
	
	
	for(int iy = 0; iy < in_size_y; iy++) {
	
		
		for(int ix = 0; ix < in_size_x; ix++) {
			int is_diff = 0;
			
			int p = INPX(ix, iy);
			if(p != 0) continue; // the center pixel should be the black one
			
			// TODO: similar check for interior pixels
			
			if(iy > 0) {
				if(p != INPX(ix, iy - 1)) is_diff = 1;
			}
			
			if(iy < in_size_y) {
				if(p != INPX(ix, iy + 1)) is_diff = 1;
			}	
			
			if(ix > 0) {
				if(p != INPX(ix - 1, iy)) is_diff = 1;
			}	
			
			if(ix < in_size_x) {
				if(p != INPX(ix + 1, iy)) is_diff = 1;
			}	
			
			
			if(!is_diff) continue;
			
			
		
			// there is an edge
			// update all the output pixels
			
			int out_cx = ix / out_mag;
			int out_cy = iy / out_mag;
			
			for(int offy = -out_mag; offy < out_mag; offy++) { // in output pixels
			for(int offx = -out_mag; offx < out_mag; offx++) {
				
				// the distance should be calculated in putput pixels, to the center of the output pixel
				float d = sqrt(offy * offy + offx * offx);
				
				int encoded = sdfEncode(d, 0, 192);
				
				
				// output encode the distance, overwrite if lower
				
				int existing = output[out_cx + offx + ((out_cy + offy) * out_size_x)];
				
					
				if(encoded < existing) { 
					output[out_cx + offx + ((out_cy + offy) * out_size_x)] = encoded;
				}
			}}
		
		}
	}


	{
		static char buf[200];
		sprintf(buf, "new-sdf-output-%d.png", fg->code);
		writePNG(buf, 1, output, out_size_x, out_size_y);
	}

}

















