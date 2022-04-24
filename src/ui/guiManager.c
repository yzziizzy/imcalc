

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
	
	gm->vertCount = 0;
	gm->vertAlloc = 2048;
	gm->vertBuffer = calloc(1, sizeof(*gm->vertBuffer) * gm->vertAlloc);
	
	
	gm->windowHeap.alloc = 16;
	gm->windowHeap.buf = calloc(1, gm->windowHeap.alloc * sizeof(*gm->windowHeap.buf));
	gm->rootWin = GUIWindow_new(gm, 0);
	
	
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



GUIWindow* GUIWindow_new(GUIManager* gm, GUIWindow* parent) {
	if(gm->windowHeap.cnt >= gm->windowHeap.alloc) {
		gm->windowHeap.alloc *= 2;
		gm->windowHeap.buf = realloc(gm->windowHeap.buf, gm->windowHeap.alloc * sizeof(*gm->windowHeap.buf));
		memset(gm->windowHeap.buf + gm->windowHeap.cnt, 0, (gm->windowHeap.alloc - gm->windowHeap.cnt) * sizeof(*gm->windowHeap.buf)); 
	}
	
	
	GUIWindow* w = &gm->windowHeap.buf[gm->windowHeap.cnt++];
	
	w->parent = parent;
	w->vertCount = 0;
		
	if(w->vertAlloc == 0) {
		w->vertAlloc = 32;
		w->vertBuffer = calloc(1, w->vertAlloc * sizeof(*w->vertBuffer));
	}
	
	VEC_TRUNC(&w->children);
		
	return w;
}



void GUIManager_SetMainWindowTitle(GUIManager* gm, char* title) {
	if(gm->windowTitleSetFn) gm->windowTitleSetFn(gm->windowTitleSetData, title);
}

void GUIManager_SetCursor(GUIManager* gm, int cursor) {
	if(gm->currentCursor == cursor) return;
	gm->currentCursor = cursor;
	if(gm->mouseCursorSetFn) gm->mouseCursorSetFn(gm->mouseCursorSetData, cursor);
}






GUIUnifiedVertex* GUIWindow_checkElemBuffer(GUIWindow* w, int count) {
	if(w->vertAlloc < w->vertCount + count) {
		w->vertAlloc = MAX(w->vertAlloc * 2, w->vertAlloc + count);
		w->vertBuffer = realloc(w->vertBuffer, sizeof(*w->vertBuffer) * w->vertAlloc);
	}
	
	return w->vertBuffer + w->vertCount;
}

GUIUnifiedVertex* GUIManager_reserveElements(GUIManager* gm, int count) {
	GUIUnifiedVertex* v = GUIWindow_checkElemBuffer(gm->curWin, count);
	gm->curWin->vertCount += count;
	gm->totalVerts += count;
	return v;
}

void GUIManager_copyElements(GUIManager* gm, GUIUnifiedVertex* verts, int count) {
	GUIWindow_checkElemBuffer(gm->curWin, count);
	memcpy(gm->curWin->vertBuffer + gm->vertCount, verts, count * sizeof(*gm->curWin->vertBuffer));
	gm->curWin->vertCount += count;
	gm->totalVerts += count;
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
	
	if(type == GUIEVENT_KeyUp) {
		
		VEC_PUSH(&gm->keysReleased, ((GUIKeyEvent){
			.type = GUIEVENT_KeyUp,
			.character = iev->character, 
			.keycode = iev->keysym, 
			.modifiers = translateModKeys(gm, iev),
		}));
	}
}



// Rendering
static int gui_vert_sort_fn(GUIUnifiedVertex* a, GUIUnifiedVertex* b) {
	return a->z == b->z ? 0 : (a->z > b->z ? 1 : -1);
}
static int gui_win_sort_fn(GUIWindow* a, GUIWindow* b) {
	return a->z == b->z ? 0 : (a->z > b->z ? 1 : -1);
}



void GUIManager_appendWindowVerts(GUIManager* gm, GUIWindow* w) {
	
//	double sort = getCurrentTime();
	qsort(w->vertBuffer, w->vertCount, sizeof(*w->vertBuffer), (void*)gui_vert_sort_fn);
//	printf("qsort time: %fus\n", timeSince(sort) * 1000000.0);
	
	if(0&&VEC_LEN(&w->children) == 0) {
		// simple memcpy
		memcpy(gm->vertBuffer + gm->vertCount, w->vertBuffer, w->vertCount * sizeof(*gm->vertBuffer));
		gm->vertCount += w->vertCount;
		
		return;
	}
	
	// zipper the elements and windows together based on z
	
	VEC_SORT(&w->children, gui_win_sort_fn);
	
	int ci = 0;
	GUIWindow** cw = VEC_DATA(&w->children);
	
	int vi = 0;
	GUIUnifiedVertex* v = w->vertBuffer;
	
	GUIUnifiedVertex* tv = gm->vertBuffer + gm->vertCount;
	while(1) {
		
		if(ci < VEC_LEN(&w->children) && ((*cw)->z < v->z || vi >= w->vertCount)) {
			// append the window first
			GUIManager_appendWindowVerts(gm, *cw);
			ci++;
			cw++;
			
			tv = gm->vertBuffer + gm->vertCount;
		}
		else if(vi < w->vertCount) {
			*tv = *v;
			tv->pos.l += w->absClip.min.x;
			tv->pos.r += w->absClip.min.x;
			tv->pos.t += w->absClip.min.y;
			tv->pos.b += w->absClip.min.y;
			tv++;
			vi++;
			v++;
			gm->vertCount++;
		}
		else {
			break; // no more vertices or child windows
		}
	}
	

}



static void preFrame(PassFrameParams* pfp, void* gm_) {
	GUIManager* gm = (GUIManager*)gm_;
	
	

	
	

	double sort;
	double time;
	double total = 0.0;
//	printf("\n");
	
	sort = getCurrentTime();
	
	
	// walk last frame's gui info to set window ids
	float highestZ = -99999999.9;
	GUIWindow* highestW = gm->windowHeap.buf; // incidentally the root window
	GUIWindow* w = gm->windowHeap.buf;
	for(int i = 0; i < gm->windowHeap.cnt; i++, w++) {
		if(w->z >= highestZ && boxContainsPoint2p(&w->absClip, &gm->lastMousePos)) {
			highestZ = w->z;
			highestW = w;
		}
	}
	
	gm->mouseWinID = highestW->id;
	
	// reset the IM gui cache
	
	gm->totalVerts = 0;
	gm->vertCount = 0;
	
	// clean up the windows from last frame
	gm->windowHeap.cnt = 0;
	gm->rootWin = GUIWindow_new(gm, 0);
	VEC_TRUNC(&gm->windowStack);
	VEC_PUSH(&gm->windowStack, gm->rootWin);
	gm->curWin = gm->rootWin;
	
	gm->rootWin->absClip = (AABB2){min: {0,0}, max: {pfp->dp->targetSize.x, pfp->dp->targetSize.y}};
	gm->rootWin->clip = gm->rootWin->absClip;
	VEC_TRUNC(&gm->clipStack);
	gm->curClip = gm->rootWin->clip;
	
	gm->curZ = 1.0;
	gm->fontSize = 20.0f;
	gm->checkboxBoxSize = 15;
	gm->time = pfp->appTime;
	gm->timeElapsed = pfp->timeElapsed;
	
	/* soft cursor needs special handling
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

	*/

}

static void draw(void* gm_, GLuint progID, PassDrawParams* pdp) {
	GUIManager* gm = (GUIManager*)gm_;
	size_t offset;

	GUIUnifiedVertex* vmem = PCBuffer_beginWrite(&gm->instVB);
	if(!vmem) {
		printf("attempted to update invalid PCBuffer in GUIManager\n");
		return;
	}

	if(gm->totalVerts >= gm->vertAlloc) {
		gm->vertAlloc = nextPOT(gm->totalVerts);
		gm->vertBuffer = realloc(gm->vertBuffer, gm->vertAlloc * sizeof(*gm->vertBuffer));
	}
	
	GUIManager_appendWindowVerts(gm, gm->rootWin);

 //	sort = getCurrentTime();
	memcpy(vmem, gm->vertBuffer, gm->vertCount * sizeof(*gm->vertBuffer));
	// time = timeSince(sort);
//	total += time;
	

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
	glDrawArrays(GL_POINTS, offset / sizeof(GUIUnifiedVertex), gm->vertCount);
	
	glexit("");
}



static void postFrame(void* gm_) {
	GUIManager* gm = (GUIManager*)gm_;
	PCBuffer_afterDraw(&gm->instVB);
	
	// reset the inter-frame event accumulators
	VEC_TRUNC(&gm->keysReleased);
	gm->mouseWentUp = 0;
	gm->mouseWentDown = 0;
	gm->hotID = 0;
	
	
	// gc the element data
	VEC_EACHP(&gm->elementData, i, d) {
	RESTART:
		if(d->age > 10) {
					
			if(d->freeFn) {
				d->freeFn(d->data);
			}
			
			if(i < VEC_LEN(&gm->elementData) - 1)  {
				*d = VEC_TAIL(&gm->elementData);
				VEC_LEN(&gm->elementData)--;
				gm->elementData.len--;
				goto RESTART;
			}
			else {
				VEC_LEN(&gm->elementData)--;
				break;
			}
		}
		else {
			d->age++;
		}
	}
	
	
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





