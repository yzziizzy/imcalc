

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


#include "gui.h"
#include "gui_internal.h"

// temp
#include "../calc.h"



static void preFrame(PassFrameParams* pfp, void* gm_);
static void draw(void* gm_, GLuint progID, PassDrawParams* pdp);
static void postFrame(void* gm_);



GUIManager* GUIManager_alloc(GUI_GlobalSettings* gs) {
	GUIManager* gm;
	pcalloc(gm);
	
	GUIManager_init(gm, gs);
	
	return gm;
}





// _init is always called before _initGL
void GUIManager_init(GUIManager* gm, GUI_GlobalSettings* gs) {
	
	
	gm->gs = gs;
	
//	gm->useSoftCursor = 1;
//	gm->softCursorName = "icon/folder";
//	gm->softCursorSize = (Vector2){50,50};
	
	gm->maxInstances = gs->maxInstances;
	
	gm->elementCount = 0;
	gm->elementAlloc = 2048;
	gm->elemBuffer = calloc(1, sizeof(*gm->elemBuffer) * gm->elementAlloc);
	
	gm->fm = FontManager_alloc(gs);
	
// 	gm->ta = TextureAtlas_alloc(gs);
// 	gm->ta->width = 256;
// 	TextureAtlas_addFolder(gm->ta, "pre", "assets/ui/icons", 0);
// 	TextureAtlas_finalize(gm->ta);
	
	gm->minDragDist = 2;
	gm->doubleClickTime = 0.500;
	
	
	
	
	gm->defaults.font = FontManager_findFont(gm->fm, "Arial");
	gm->defaults.fontSize = 10;
	decodeHexColorNorm(gs->textColor, (float*)&(gm->defaults.textColor));

	decodeHexColorNorm(gs->buttonTextColor, (float*)&(gm->defaults.buttonTextColor));
	decodeHexColorNorm(gs->buttonHoverTextColor, (float*)&(gm->defaults.buttonHoverTextColor));
	decodeHexColorNorm(gs->buttonDisTextColor, (float*)&(gm->defaults.buttonDisTextColor));
	decodeHexColorNorm(gs->buttonBgColor, (float*)&(gm->defaults.buttonBgColor));
	decodeHexColorNorm(gs->buttonHoverBgColor, (float*)&(gm->defaults.buttonHoverBgColor));
	decodeHexColorNorm(gs->buttonDisBgColor, (float*)&(gm->defaults.buttonDisBgColor));
	decodeHexColorNorm(gs->buttonBorderColor, (float*)&(gm->defaults.buttonBorderColor));
	decodeHexColorNorm(gs->buttonHoverBorderColor, (float*)&(gm->defaults.buttonHoverBorderColor));
	decodeHexColorNorm(gs->buttonDisBorderColor, (float*)&(gm->defaults.buttonDisBorderColor));

	decodeHexColorNorm(gs->editBorderColor, (float*)&(gm->defaults.editBorderColor));
	decodeHexColorNorm(gs->editBgColor, (float*)&(gm->defaults.editBgColor));
	decodeHexColorNorm(gs->editTextColor, (float*)&(gm->defaults.editTextColor));
	decodeHexColorNorm(gs->editSelBgColor, (float*)&(gm->defaults.editSelBgColor));
	gm->defaults.editWidth = 150;
	gm->defaults.editHeight = 18;

	decodeHexColorNorm(gs->cursorColor, (float*)&(gm->defaults.cursorColor));

	decodeHexColorNorm(gs->tabTextColor, (float*)&(gm->defaults.tabTextColor));
	decodeHexColorNorm(gs->tabBorderColor, (float*)&(gm->defaults.tabBorderColor));
	decodeHexColorNorm(gs->tabActiveBgColor, (float*)&(gm->defaults.tabActiveBgColor));
	decodeHexColorNorm(gs->tabHoverBgColor, (float*)&(gm->defaults.tabHoverBgColor));
	decodeHexColorNorm(gs->tabBgColor, (float*)&(gm->defaults.tabBgColor));

	decodeHexColorNorm(gs->outlineCurrentLineBorderColor, (float*)&(gm->defaults.outlineCurrentLineBorderColor));
	decodeHexColorNorm(gs->selectedItemTextColor, (float*)&(gm->defaults.selectedItemTextColor));
	decodeHexColorNorm(gs->selectedItemBgColor, (float*)&(gm->defaults.selectedItemBgColor));

	decodeHexColorNorm(gs->windowBgBorderColor, (float*)&(gm->defaults.windowBgBorderColor));
	gm->defaults.windowBgBorderWidth = 1;
	decodeHexColorNorm(gs->windowBgColor, (float*)&(gm->defaults.windowBgColor));
	decodeHexColorNorm(gs->windowTitleBorderColor, (float*)&(gm->defaults.windowTitleBorderColor));
	gm->defaults.windowTitleBorderWidth = 1;
	decodeHexColorNorm(gs->windowTitleColor, (float*)&(gm->defaults.windowTitleColor));
	decodeHexColorNorm(gs->windowTitleTextColor, (float*)&(gm->defaults.windowTitleTextColor));
	decodeHexColorNorm(gs->windowCloseBtnBorderColor, (float*)&(gm->defaults.windowCloseBtnBorderColor));
	gm->defaults.windowCloseBtnBorderWidth = 1;
	decodeHexColorNorm(gs->windowCloseBtnColor, (float*)&(gm->defaults.windowCloseBtnColor));
	decodeHexColorNorm(gs->windowScrollbarColor, (float*)&(gm->defaults.windowScrollbarColor));
	decodeHexColorNorm(gs->windowScrollbarBorderColor, (float*)&(gm->defaults.windowScrollbarBorderColor));
	gm->defaults.windowScrollbarBorderWidth = 1;

	decodeHexColorNorm(gs->selectBgColor, (float*)&(gm->defaults.selectBgColor));
	decodeHexColorNorm(gs->selectBorderColor, (float*)&(gm->defaults.selectBorderColor));
	decodeHexColorNorm(gs->selectTextColor, (float*)&(gm->defaults.selectTextColor));
	
	decodeHexColorNorm(gs->trayBgColor, (float*)&(gm->defaults.trayBgColor));
	
	gm->defaults.charWidth_fw = gs->charWidth_fw;
	gm->defaults.lineHeight_fw = gs->lineHeight_fw;
	gm->defaults.font_fw = FontManager_findFont(gm->fm, gs->font_fw);;
	gm->defaults.fontSize_fw = gs->fontSize_fw;
	decodeHexColorNorm(gs->statusBarBgColor, (float*)&(gm->defaults.statusBarBgColor));
	decodeHexColorNorm(gs->statusBarTextColor, (float*)&(gm->defaults.statusBarTextColor));
	decodeHexColorNorm(gs->fileBrowserHeaderTextColor, (float*)&(gm->defaults.fileBrowserHeaderTextColor));
	decodeHexColorNorm(gs->fileBrowserHeaderBorderColor, (float*)&(gm->defaults.fileBrowserHeaderBorderColor));
	decodeHexColorNorm(gs->fileBrowserHeaderBgColor, (float*)&(gm->defaults.fileBrowserHeaderBgColor));
	
	gm->defaults.selectSize = (Vector2){80, 25};
	
	gm->defaultCursor = GUIMOUSECURSOR_ARROW;
	
}


void GUIManager_initGL(GUIManager* gm) {
	static VAOConfig vaoConfig[] = {
		{0, 4, GL_FLOAT, 0, GL_FALSE}, // top, left, bottom, right
		{0, 4, GL_FLOAT, 0, GL_FALSE}, // tlbr clipping planes
		{0, 4, GL_UNSIGNED_BYTE, 0, GL_FALSE}, // tex indices 1&2, tex fade, gui type
		{0, 4, GL_UNSIGNED_SHORT, 0, GL_TRUE}, // tex offset 1&2
		{0, 4, GL_UNSIGNED_SHORT, 0, GL_TRUE}, // tex size 1&2
		
		{0, 4, GL_UNSIGNED_BYTE, 0, GL_TRUE}, // fg color
		{0, 4, GL_UNSIGNED_BYTE, 0, GL_TRUE}, // bg color
		
		{0, 4, GL_FLOAT, 0, GL_FALSE}, // z-index, alpha, opts 1-2
		
		{0, 0, 0, 0, 0}
	};

	
	gm->vao = makeVAO(vaoConfig);
	glBindVertexArray(gm->vao);
	
	int stride = calcVAOStride(0, vaoConfig);

	PCBuffer_startInit(&gm->instVB, gm->maxInstances * stride, GL_ARRAY_BUFFER);
	updateVAO(0, vaoConfig); 
	PCBuffer_finishInit(&gm->instVB);
	
	///////////////////////////////
	// font texture
	
	glGenTextures(1, &gm->fontAtlasID);
	glBindTexture(GL_TEXTURE_2D_ARRAY, gm->fontAtlasID);
	glerr("bind font tex");
	
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0); // no mipmaps for this; it'll get fucked up
	glerr("param font tex");
	
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R8, gm->fm->atlasSize, gm->fm->atlasSize, VEC_LEN(&gm->fm->atlas));
	
	VEC_EACH(&gm->fm->atlas, ind, at) {
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 
			0, 0, ind, // offsets
			gm->fm->atlasSize, gm->fm->atlasSize, 1, 
			GL_RED, GL_UNSIGNED_BYTE, at);
		glerr("load font tex");
	}
	
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	
	
	///////////////////////////////
	// regular texture atlas
	
	glGenTextures(1, &gm->atlasID);
	glBindTexture(GL_TEXTURE_2D_ARRAY, gm->atlasID);
	glerr("bind font tex");
	
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0); // no mipmaps for this; it'll get fucked up
	glerr("param font tex");
	
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, gm->ta->width, gm->ta->width, VEC_LEN(&gm->ta->atlas));
	
	char buf [50];
	
	VEC_EACH(&gm->ta->atlas, ind, at2) {
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 
			0, 0, ind, // offsets
			gm->ta->width, gm->ta->width, 1, 
			GL_RGBA, GL_UNSIGNED_BYTE, at2);
		glerr("load font tex");	
	}
	
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	
	//////////////////////////////////
	
}




void GUIManager_SetMainWindowTitle(GUIManager* gm, char* title) {
	if(gm->windowTitleSetFn) gm->windowTitleSetFn(gm->windowTitleSetData, title);
}

void GUIManager_SetCursor(GUIManager* gm, int cursor) {
	if(gm->currentCursor == cursor) return;
	gm->currentCursor = cursor;
	if(gm->mouseCursorSetFn) gm->mouseCursorSetFn(gm->mouseCursorSetData, cursor);
}






GUIUnifiedVertex* GUIManager_checkElemBuffer(GUIManager* gm, int count) {
	if(gm->elementAlloc < gm->elementCount + count) {
		gm->elementAlloc = MAX(gm->elementAlloc * 2, gm->elementAlloc + count);
		gm->elemBuffer = realloc(gm->elemBuffer, sizeof(*gm->elemBuffer) * gm->elementAlloc);
	}
	
	return gm->elemBuffer + gm->elementCount;
}

GUIUnifiedVertex* GUIManager_reserveElements(GUIManager* gm, int count) {
	GUIUnifiedVertex* v = GUIManager_checkElemBuffer(gm, count);
	gm->elementCount += count;
	return v;
}

void GUIManager_copyElements(GUIManager* gm, GUIUnifiedVertex* elems, int count) {
	GUIManager_checkElemBuffer(gm, count);
	memcpy(gm->elemBuffer + gm->elementCount, elems, count * sizeof(*gm->elemBuffer));
}



// Textures

void GUIManager_UploadTexture(GUIManager* gm, void* pixeldata, int fmt, int width, int height) {
	
	
	
}




// Event Handling


static unsigned int translateModKeys(GUIManager* gm, InputEvent* iev) {
	unsigned int m = 0;
	
	if(iev->kbmods & IS_CONTROL) {
		m |= GUIMODKEY_CTRL;
		// check which one
	}
	if(iev->kbmods & IS_ALT) {
		m |= GUIMODKEY_ALT;
		// check which one
	}
	if(iev->kbmods & IS_SHIFT) {
		m |= GUIMODKEY_SHIFT;
		// check which one
	}
	if(iev->kbmods & IS_TUX) {
		m |= GUIMODKEY_TUX;
		// check which one
	}
	
	return m;
}


void GUIManager_HandleMouseMove(GUIManager* gm, InputState* is, InputEvent* iev) {
	Vector2 newPos = {
		iev->intPos.x, iev->intPos.y
	};
	
	gm->lastMousePos = newPos;
	

//		.modifiers = translateModKeys(gm, iev), 
	
}

void GUIManager_HandleMouseClick(GUIManager* gm, InputState* is, InputEvent* iev) {
	
	Vector2 newPos = {
		iev->intPos.x, iev->intPos.y
	};
	
	if(iev->type == EVENT_MOUSEDOWN) {
		gm->mouseWentDown = 1;
	}
	if(iev->type == EVENT_MOUSEUP) {
		gm->mouseWentUp = 1;
	}
	

	
	// buttons: 
	// 1 - left
	// 2 - mid
	// 3 - right
	// 4 - scroll up
	// 5 - scroll down
	// 6 - scroll left
	// 7 - scroll right
	// 8 - front left side (on my mouse)
	// 9 - rear left side (on my mouse)
	

//		.modifiers = translateModKeys(gm, iev),
		// push the latest click onto the stack
	gm->clickHistory[2] = gm->clickHistory[1];
	gm->clickHistory[1] = gm->clickHistory[0];
	gm->clickHistory[0].time = iev->time;
	gm->clickHistory[0].button = iev->button;

}

void GUIManager_HandleKeyInput(GUIManager* gm, InputState* is, InputEvent* iev) {
	
	int type;
	
	// translate event type
	switch(iev->type) {
		case EVENT_KEYDOWN: type = GUIEVENT_KeyDown; break; 
		case EVENT_KEYUP: type = GUIEVENT_KeyUp; break; 
		case EVENT_TEXT: type = GUIEVENT_KeyUp; break; 
		default:
			fprintf(stderr, "!!! Non-keyboard event in GUIManager_HandleKeyInput: %d\n", iev->type);
			return; // not actually a kb event
	}
	
	
//	VEC_PUSH(gm->keysReleased

}



// Rendering



static int gui_elem_sort_fn(GUIUnifiedVertex* a, GUIUnifiedVertex* b) {
	return a->z == b->z ? 0 : (a->z > b->z ? 1 : -1);
}


static void preFrame(PassFrameParams* pfp, void* gm_) {
	GUIManager* gm = (GUIManager*)gm_;
	
	
	GUIUnifiedVertex* vmem = PCBuffer_beginWrite(&gm->instVB);
	if(!vmem) {
		printf("attempted to update invalid PCBuffer in GUIManager\n");
		return;
	}
	
	

	double sort;
	double time;
	double total = 0.0;
//	printf("\n");
	
	sort = getCurrentTime();
	gm->elementCount = 0;
	
//	
//	gm->elementCount = 0;
//	gm->root->size = (Vector2){pfp->dp->targetSize.x, pfp->dp->targetSize.y};
//	gm->root->absClip = (AABB2){0,0, pfp->dp->targetSize.x, pfp->dp->targetSize.y};
//	
//	GUIRenderParams grp = {
//		.offset = {0,0}, 
//		.size = gm->root->size,
//		.clip = gm->root->absClip,
//	};
//	
////#define printf(...)
//	
//	GUIHeader_updatePos(gm->root, &grp, pfp);
	time = timeSince(sort);
	total += time;
//	printf("updatePos time: %fus\n", time  * 1000000.0);
	
	sort = getCurrentTime();
	
	//GUIHeader_render(gm->root, pfp);
	
	
	
	
	
	
	// Rendering code
	

	
	gm->curClip = (AABB2){0,0, pfp->dp->targetSize.x, pfp->dp->targetSize.y};
	gm->curZ = 1.0;
	gm->fontSize = 20.0f;
	gm->checkboxBoxSize = 15;
	
	
//	GUI_BoxFilled_(gm, (Vector2){20,20}, (Vector2){200,20},
//		 8, &((Color4){1.0,0,0,1.0}),
//		&((Color4){0,1.0,0,1.0})
//		);
//	gm->curZ = 1.1;
	
//	GUI_Box_(gm, (Vector2){20,20}, (Vector2){20,20}, 8, &((Color4){0,1,1,1}));
	
	draw_gui_root(gm);
	
	
	
	
	gm->mouseWentUp = 0;
	gm->mouseWentDown = 0;
	gm->hotID = 0;
	
	
	
	
	
	
	time = timeSince(sort);
	total += time;
//	printf("render time: %fus\n", time  * 1000000.0);

	if(gm->useSoftCursor) {
		TextureAtlasItem* it;
		if(HT_get(&gm->ta->items, gm->softCursorName, &it)) {
			printf("could not find gui image '%s'\n", gm->softCursorName);
		}
		else { 
			
			GUIUnifiedVertex* v = GUIManager_reserveElements(gm, 1);
			*v++ = (GUIUnifiedVertex){
				.pos = {
					gm->lastMousePos.x, 
					gm->lastMousePos.y, 
					gm->lastMousePos.x + gm->softCursorSize.x, 
					gm->lastMousePos.y + gm->softCursorSize.y
				},
				.clip = {0,0,999999,999999},
				
				.guiType = 2, // image
				
				.texIndex1 = it->index,
				.texIndex2 = 0,
	
				.texOffset1 = { it->offsetNorm.x * 65535, it->offsetNorm.y * 65535 },
				.texOffset2 = 0,
				.texSize1 = { it->sizeNorm.x * 65535, it->sizeNorm.y * 65535 },
				.texSize2 = 0,
	
				
				.z = 9999999,
				.alpha = 1,
				.rot = 0,
			};
		}
	}

	
// 	static size_t framecount = 0;
	
	
 	sort = getCurrentTime();
// 	gui_debugFileDumpVertexBuffer(gm, "/tmp/gpuedit/presortdump", framecount);
	qsort(gm->elemBuffer, gm->elementCount, sizeof(*gm->elemBuffer), (void*)gui_elem_sort_fn);
 	time = timeSince(sort);
	total += time;
//	printf("qsort time: %fus\n", time  * 1000000.0);
	
// 	gui_debugFileDumpVertexBuffer(gm, "/tmp/gpuedit/framedump", framecount);
	
// 	printf("Elemcount: %ld\n", gm->elementCount);
	
// 	framecount++;
 	sort = getCurrentTime();
	memcpy(vmem, gm->elemBuffer, gm->elementCount * sizeof(*gm->elemBuffer));
	 time = timeSince(sort);
	total += time;
//	printf("memcpy time: %fus\n", time  * 1000000.0);
//	printf("total time: %fus\n", total  * 1000000.0);
#undef printf
}

static void draw(void* gm_, GLuint progID, PassDrawParams* pdp) {
	GUIManager* gm = (GUIManager*)gm_;
	size_t offset;

// 	if(mdi->uniformSetup) {
// 		(*mdi->uniformSetup)(mdi->data, progID);
// 	}
	GLuint ts_ul = glGetUniformLocation(progID, "fontTex");
	GLuint ta_ul = glGetUniformLocation(progID, "atlasTex");
	
	glActiveTexture(GL_TEXTURE0 + 29);
	glUniform1i(ts_ul, 29);
	glexit("text sampler uniform");
 	glBindTexture(GL_TEXTURE_2D_ARRAY, gm->fontAtlasID);
//  	glBindTexture(GL_TEXTURE_2D, gt->font->textureID); // TODO check null ptr
 	glexit("bind texture");



	glActiveTexture(GL_TEXTURE0 + 28);
	glUniform1i(ta_ul, 28);
	glexit("text sampler uniform");
 	glBindTexture(GL_TEXTURE_2D_ARRAY, gm->atlasID);
//  	glBindTexture(GL_TEXTURE_2D, gt->font->textureID); // TODO check null ptr
 	glexit("bind texture");
	
	// ------- draw --------
	
	glBindVertexArray(gm->vao);
	
	PCBuffer_bind(&gm->instVB);
	offset = PCBuffer_getOffset(&gm->instVB);
	glDrawArrays(GL_POINTS, offset / sizeof(GUIUnifiedVertex), gm->elementCount);
	
	glexit("");
}



static void postFrame(void* gm_) {
	GUIManager* gm = (GUIManager*)gm_;
	PCBuffer_afterDraw(&gm->instVB);
}


RenderPass* GUIManager_CreateRenderPass(GUIManager* gm) {
	
	RenderPass* rp;
	PassDrawable* pd;

	pd = GUIManager_CreateDrawable(gm);

	rp = calloc(1, sizeof(*rp));
	RenderPass_init(rp);
	RenderPass_addDrawable(rp, pd);
	//rp->fboIndex = LIGHTING;
	
	return rp;
}


PassDrawable* GUIManager_CreateDrawable(GUIManager* gm) {
	
	PassDrawable* pd;
	static ShaderProgram* prog = NULL;
	
	if(!prog) {
		prog = loadCombinedProgram("guiUnified");
		glexit("");
	}
	
	
	pd = Pass_allocDrawable("GUIManager");
	pd->data = gm;
	pd->preFrame = preFrame;
	pd->draw = draw;
	pd->postFrame = postFrame;
	pd->prog = prog;
	
	return pd;;
}





