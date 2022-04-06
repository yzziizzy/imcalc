

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


#include "gui.h"
#include "gui_internal.h"











void gui_drawBox(GUIManager* gm, Vector2 tl, Vector2 sz, AABB2* clip, float z, Color4* color) {
	GUIUnifiedVertex* v = GUIManager_reserveElements(gm, 1);
	
	*v++ = (GUIUnifiedVertex){
		.pos = {tl.x, tl.y, tl.x + sz.x, tl.y + sz.y},
		.clip = GUI_AABB2_TO_SHADER(*clip),
		
		.guiType = 0, // just a box
		
		.fg = GUI_COLOR4_TO_SHADER(*color), 
		.bg = GUI_COLOR4_TO_SHADER(*color), 
		.z = z,
		.alpha = 1,
	};
}

void gui_drawBoxBorder(
	GUIManager* gm, 
	Vector2 tl, 
	Vector2 sz, 
	AABB2* clip, 
	float z, 
	Color4* bgColor,
	float borderWidth,
	Color4* borderColor
) {
	GUIUnifiedVertex* v = GUIManager_reserveElements(gm, 1);
	
	*v++ = (GUIUnifiedVertex){
		.pos = {tl.x, tl.y, tl.x + sz.x, tl.y + sz.y},
		.clip = GUI_AABB2_TO_SHADER(*clip),
		
		.guiType = 4, // bordered box
		
		.texIndex1 = borderWidth,
		
		.fg = GUI_COLOR4_TO_SHADER(*borderColor), 
		.bg = GUI_COLOR4_TO_SHADER(*bgColor), 
		.z = z,
		.alpha = 1,
	};
}



void gui_drawTriangle(
	GUIManager* gm, 
	Vector2 centroid, 
	float baseWidth, 
	float height, 
	float rotation,
	AABB2* clip, 
	float z, 
	Color4* bgColor
) {
	GUIUnifiedVertex* v = GUIManager_reserveElements(gm, 1);
	
	*v++ = (GUIUnifiedVertex){
		.pos = {centroid.x, centroid.y, baseWidth, height},
		.clip = GUI_AABB2_TO_SHADER(*clip),
		
		.guiType = 6, // triangle
		
		.texIndex1 = 0,
		
		.fg = GUI_COLOR4_TO_SHADER(*bgColor), 
		.bg = GUI_COLOR4_TO_SHADER(*bgColor), 
		.z = z,
		.alpha = 1,
		.rot = rotation,
	};
}

void gui_drawTriangleBorder(
	GUIManager* gm, 
	Vector2 centroid, 
	float baseWidth, 
	float height,
	float rotation,
	AABB2* clip, 
	float z, 
	Color4* bgColor,
	float borderWidth,
	Color4* borderColor
) {
	GUIUnifiedVertex* v = GUIManager_reserveElements(gm, 1);
	
	*v++ = (GUIUnifiedVertex){
		.pos = {centroid.x, centroid.y, baseWidth, height},
		.clip = GUI_AABB2_TO_SHADER(*clip),
		
		.guiType = 6, // triangle
		
		.texIndex1 = borderWidth,
		
		.fg = GUI_COLOR4_TO_SHADER(*borderColor), 
		.bg = GUI_COLOR4_TO_SHADER(*bgColor), 
		.z = z,
		.alpha = 1,
		.rot = rotation,
	};
}




void gui_drawImg(GUIManager* gm, char* name, Vector2 tl, Vector2 sz, AABB2* clip, float z) {
	
	TextureAtlasItem* it;
	if(HT_get(&gm->ta->items, name, &it)) {
		fprintf(stderr, "Could not find gui image '%s' \n", name);
	}
	else {
		// icon
		GUIUnifiedVertex* v = GUIManager_reserveElements(gm, 1);
		*v = (GUIUnifiedVertex){
			.pos = {tl.x, tl.y, tl.x + sz.x, tl.y + sz.y},
			.clip = GUI_AABB2_TO_SHADER(*clip),
			
			.texIndex1 = it->index,
			.texIndex2 = 0,
			.texFade = 0,
			
			.guiType = 2, // simple image
			
			.texOffset1 = { it->offsetNorm.x * 65535, it->offsetNorm.y * 65535 },
			.texOffset2 = 0,
			.texSize1 = { it->sizeNorm.x * 65535, it->sizeNorm.y * 65535 },
			.texSize2 = 0,
			
			.fg = {255,255,255,255},
			.bg = {255,255,255,255},
			
			.z = z,
			.alpha = 1,
		};
	}
}


float gui_getTextLineWidth(
	GUIManager* gm,
	GUIFont* font,
	float fontsize,
	char* txt, 
	size_t charCount
) {
	if(txt == NULL || charCount == 0) return 0;
	
	font = gm->defaults.font;
	if(!fontsize) fontsize = gm->defaults.fontSize; // HACK
	float hoff = fontsize * font->ascender;
	float adv = 0;
	
	float spaceadv = font->regular[' '].advance * fontsize;
	
	for(size_t n = 0; txt[n] != 0 && n < charCount; n++) {
		char c = txt[n];
		
		struct charInfo* ci = &font->regular[(int)c];
		
		if(c == '\t') {
			adv += spaceadv * 4; // hardcoded to annoy you
		}
		else if(c != ' ') {
			adv += ci->advance * fontsize;
		}
		else {
			adv += spaceadv;
		}
	}
	
	return adv;	
}



void gui_drawCharacter(
	GUIManager* gm,
	Vector2 tl,
	Vector2 sz,
	AABB2* clip,
	float z,
	int c,
	struct Color4* color,
	GUIFont* font,
	float fontsize
) {
	GUIUnifiedVertex* v = GUIManager_reserveElements(gm, 1);
	struct charInfo* ci = &font->regular[(int)c];
	float hoff = fontsize * font->ascender;
			
	Vector2 off = tl;
	
	float offx = ci->texNormOffset.x;//TextRes_charTexOffset(gm->font, 'A');
	float offy = ci->texNormOffset.y;//TextRes_charTexOffset(gm->font, 'A');
	float widx = ci->texNormSize.x;//TextRes_charWidth(gm->font, 'A');
	float widy = ci->texNormSize.y;//TextRes_charWidth(gm->font, 'A');
	
	v->pos.t = off.y + hoff - ci->topLeftOffset.y * fontsize;
	v->pos.l = off.x + ci->topLeftOffset.x * fontsize;
	v->pos.b = off.y + hoff + ci->size * fontsize - ci->topLeftOffset.y * fontsize;
	v->pos.r = off.x + ci->size * fontsize + ci->topLeftOffset.x * fontsize;
	
	v->guiType = 1; // text
	
	v->texOffset1.x = offx * 65535.0;
	v->texOffset1.y = offy * 65535.0;
	v->texSize1.x = widx *  65535.0;
	v->texSize1.y = widy * 65535.0;
	v->texIndex1 = ci->texIndex;
	
	v->clip = GUI_AABB2_TO_SHADER(*clip);
	v->fg = GUI_COLOR4_TO_SHADER(*color);
	v->bg = (struct ShaderColor4){0};
	v->z = z;
	
	//v++;
// 			gm->elementCount++;


}

void gui_drawTextLine(
	GUIManager* gm,
	Vector2 tl,
	Vector2 sz,
	AABB2* clip,
	struct Color4* color, 
	float z,
	char* txt, 
	size_t charCount
) {
	gui_drawTextLineAdv(gm, tl, sz, clip, color, NULL, 0, GUI_TEXT_ALIGN_LEFT, z, txt, charCount);
}


// stops on linebreak
void gui_drawTextLineAdv(
	GUIManager* gm,
	Vector2 tl,
	Vector2 sz,
	AABB2* clip,
	struct Color4* color,
	GUIFont* font,
	float fontsize,
	int align,
	float z,
	char* txt, 
	size_t charCount
) {
	
// 		printf("'%s'\n", bl->buf);
	if(txt == NULL || charCount == 0) return;
	
	
	if(!font) font = gm->defaults.font;
	if(!fontsize) fontsize = gm->defaults.fontSize; // HACK
	float hoff = fontsize * font->ascender;
	float adv = 0;
	if(!color) color = &gm->defaults.textColor;
	
	float alignoff = 0;
	if(align == GUI_TEXT_ALIGN_RIGHT) {
		float txtw = gui_getTextLineWidth(gm, font, fontsize, txt, charCount);
		alignoff = -(txtw - sz.x);
	}
	
	// BUG: the problem is (Vector2){0,0} here
	float maxAdv = sz.x;
	
	
	float spaceadv = font->regular[' '].advance * fontsize;
	
	for(size_t n = 0; txt[n] != 0 && adv < maxAdv && n < charCount; n++) {
		char c = txt[n];
		
		struct charInfo* ci = &font->regular[(int)c];
		
		if(c == '\t') {
			adv += spaceadv * 4; // hardcoded to annoy you
		}
		else if(c != ' ') {
			GUIUnifiedVertex* v = GUIManager_reserveElements(gm, 1);
			
			Vector2 off = tl;
			
			float offx = ci->texNormOffset.x;//TextRes_charTexOffset(gm->font, 'A');
			float offy = ci->texNormOffset.y;//TextRes_charTexOffset(gm->font, 'A');
			float widx = ci->texNormSize.x;//TextRes_charWidth(gm->font, 'A');
			float widy = ci->texNormSize.y;//TextRes_charWidth(gm->font, 'A');
			
			v->pos.t = off.y + hoff - ci->topLeftOffset.y * fontsize;
			v->pos.l = off.x + alignoff + adv + ci->topLeftOffset.x * fontsize;
			v->pos.b = off.y + hoff + ci->size * fontsize - ci->topLeftOffset.y * fontsize;
			v->pos.r = off.x + alignoff + adv + ci->size * fontsize + ci->topLeftOffset.x * fontsize;
			
//			printf("huh... %f, %f\n", - ci->topLeftOffset.y, fontsize);
			printf("huh... %f\n", off.x + alignoff + adv + ci->topLeftOffset.x * fontsize);
			printf("adv:%f  tlo: [%f,%f]  off: [%f,%f, %f,%f]  pos: [%f,%f,%f,%f]\n", 
				adv, ci->topLeftOffset.x, ci->topLeftOffset.x, off.x, off.y, hoff, alignoff, v->pos.t,v->pos.l,v->pos.b,v->pos.r);
			
			v->guiType = 1; // text
			
			v->texOffset1.x = offx * 65535.0;
			v->texOffset1.y = offy * 65535.0;
			v->texSize1.x = widx *  65535.0;
			v->texSize1.y = widy * 65535.0;
			v->texIndex1 = ci->texIndex;
			
			v->clip.t = clip->min.y;
			v->clip.l = clip->min.x;
			v->clip.b = clip->max.y;
			v->clip.r = clip->max.x;
			v->fg = GUI_COLOR4_TO_SHADER(*color);
			v->z = z;
			
			adv += ci->advance * fontsize; // BUG: needs sdfDataSize added in?
			//v++;
// 			gm->elementCount++;
		}
		else {
			adv += spaceadv;
		}
		
		
	}

}


float gui_getDefaultUITextWidth(
	GUIManager* gm,
	char* txt, 
	size_t maxChars
) {
	
	if(txt == NULL || maxChars == 0) return 0;
	
	GUIFont* f = gm->defaults.font;
	float size = gm->defaults.fontSize; // HACK
	float adv = 0;
	
	
	float spaceadv = f->regular[' '].advance * size;
	
	for(size_t n = 0; txt[n] != 0 && n < maxChars; n++) {
		char c = txt[n];
		
		if(c == '\t') {
			adv += spaceadv * 4; // hardcoded to annoy you
		}
		else if(c != ' ') {
			struct charInfo* ci = &f->regular[(int)c];
			adv += ci->advance * size; // BUG: needs sdfDataSize added in?
		}
		else {
			adv += spaceadv;
		}
	}
	
	return adv;
}




void gui_drawVCenteredTextLine(
	GUIManager* gm,
	Vector2 tl,  
	Vector2 sz,  
	AABB2* clip,  
	struct Color4* color,
	float z,
	char* txt, 
	size_t charCount
) {
	
	GUIFont* f = gm->defaults.font;
	float size = gm->defaults.fontSize; // HACK
	float hoff = size * f->height;
	
	float a = sz.y - hoff;
	float b = fmax(a / 2.0, 0);
	
	gui_drawTextLine(gm, (Vector2){tl.x, tl.y + b}, sz, clip, color, z, txt, charCount);
}




GUIFont* GUI_FindFont(GUIManager* gm, char* name) {
	return FontManager_findFont(gm->fm, name);
}

// 




void gui_debugPrintVertex(GUIUnifiedVertex* v, FILE* of) {
	if(v->guiType == 0) {
		fprintf(of, "Box:  % 3.1f,% 3.1f / % 2.1f,% 2.1f | clip [%.2f,%.2f,%.2f,%.2f] | z %.4f \n", 
			   v->pos.l, v->pos.t, v->pos.r, v->pos.b, 
			   v->clip.l, v->clip.t, v->clip.r, v->clip.b, 
			   v->z
		);
	}
	else if(v->guiType == 1) {
		fprintf(of, "Char: % 3.1f,% 3.1f / % 2.1f,% 2.1f | clip [%.2f,%.2f,%.2f,%.2f] | z %.4f \n", 
			   v->pos.l, v->pos.t, v->pos.r, v->pos.b, 
			   v->clip.l, v->clip.t, v->clip.r, v->clip.b, 
			   v->z
		);
	}
	else if(v->guiType == 2) {
		fprintf(of, "Img:  % 3.1f,% 3.1f / % 2.1f,% 2.1f | clip [%.2f,%.2f,%.2f,%.2f] | z %.4f \n", 
			   v->pos.l, v->pos.t, v->pos.r, v->pos.b, 
			   v->clip.l, v->clip.t, v->clip.r, v->clip.b, 
			   v->z
		);
	}
	else if(v->guiType == 3) {
		fprintf(of, "Render Target: % 3.1f,% 3.1f / % 2.1f,% 2.1f | clip [%.2f,%.2f,%.2f,%.2f] | z %.4f \n", 
			   v->pos.l, v->pos.t, v->pos.r, v->pos.b, 
			   v->clip.l, v->clip.t, v->clip.r, v->clip.b, 
			   v->z
		);
	}
	else if(v->guiType == 4) {
		fprintf(of, "BBox: % 3.1f,% 3.1f / % 2.1f,% 2.1f | clip [%.2f,%.2f,%.2f,%.2f] | z %.4f \n", 
			   v->pos.l, v->pos.t, v->pos.r, v->pos.b, 
			   v->clip.l, v->clip.t, v->clip.r, v->clip.b, 
			   v->z
		);
	}
	else if(v->guiType == 6) {
		fprintf(of, "Tri:  % 3.1f,% 3.1f / % 2.1f,% 2.1f | clip [%.2f,%.2f,%.2f,%.2f] | z %.4f \n", 
			   v->pos.l, v->pos.t, v->pos.r, v->pos.b, 
			   v->clip.l, v->clip.t, v->clip.r, v->clip.b, 
			   v->z
		);
	}
	else if(v->guiType == 7) {
		fprintf(of, "Ellipse:  %.2f,%.2f / %.2f,%.2f | clip [%.2f,%.2f,%.2f,%.2f] | z %.4f \n", 
			   v->pos.l, v->pos.t, v->pos.r, v->pos.b, 
			   v->clip.l, v->clip.t, v->clip.r, v->clip.b, 
			   v->z
		);
	}
}

void gui_debugDumpVertexBuffer(GUIUnifiedVertex* buf, size_t cnt, FILE* of) {
	for(size_t i = 0; i < cnt; i++) {
		fprintf(of, "% 5ld ", i);
		gui_debugPrintVertex(buf + i, of);
	}
}

void gui_debugFileDumpVertexBuffer(GUIManager* gm, char* filePrefix, int fileNum) {
	FILE* f;
	char fname[512];
	
	snprintf(fname, 512, "%s-%05d.txt", filePrefix, fileNum);
	
	
	f = fopen(fname, "wb");
	gui_debugDumpVertexBuffer(gm->elemBuffer, gm->elementCount, f);
	fclose(f);
}


