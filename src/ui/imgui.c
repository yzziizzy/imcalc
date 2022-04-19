

#include "gui.h"
#include "gui_internal.h"




#define C(r,g,b)  (&((Color4){r,g,b,1.0}))
#define C4(r,g,b,a)  (&((Color4){r,g,b,a}))
#define V(_x,_y) ((Vector2){(_x),(_y)})

#define HOT(id) GUI_SetHot_(gm, id, NULL, NULL);
#define ACTIVE(id) GUI_SetActive_(gm, id, NULL, NULL);

// returns true if clicked
int GUI_Button_(GUIManager* gm, void* id, Vector2 tl, Vector2 sz, char* text) {
	
	int result = 0;
	
	if(GUI_MouseInside_(gm, tl, sz)) {
		HOT(id);
	}
	
	if(gm->activeID == id) {
		if(GUI_MouseWentUp(gm, 0)) {
			if(gm->hotID == id) result = 1;
			ACTIVE(NULL);
		}
	}
	else if(gm->hotID == id) {
		if(GUI_MouseWentDown(gm, 0)) ACTIVE(id);
	}

	
	if(gm->hotID == id) {
		GUI_BoxFilled_(gm, tl, sz, 2, C(.8,.6,.3), C(.7,.4,.2));
	}
	else {
		GUI_BoxFilled_(gm, tl, sz, 2, C(.5,.6,.6), C(.4,.4,.4));
	}
	
	float fontSz = gm->fontSize;
	if(sz.y - (2*2) < fontSz) fontSz = sz.y - (2*2);
	
	gm->curZ += 0.01;
	GUI_TextLineCentered_(gm, text, strlen(text), (Vector2){tl.x, tl.y - fontSz*.25}, sz, "Arial", fontSz, C(.8,.8,.8));
	gm->curZ -= 0.01;
	
	return result;
}


// returns true if toggled on 
int GUI_ToggleButton_(GUIManager* gm, void* id, Vector2 tl, Vector2 sz, char* text, int* state) {
	
	if(GUI_MouseInside_(gm, tl, sz)) {
		HOT(id);
	}
	
	if(gm->activeID == id) {
		if(GUI_MouseWentUp(gm, 0)) {
			if(gm->hotID == id) {
				*state = !*state;
			}
			ACTIVE(NULL);
		}
	}
	else if(gm->hotID == id) {
		if(GUI_MouseWentDown(gm, 0)) ACTIVE(id);
	}

	
	if(gm->hotID == id || *state) {
		GUI_BoxFilled_(gm, tl, sz, 2, C(.8,.6,.3), C(.7,.4,.2));
	}
	else {
		GUI_BoxFilled_(gm, tl, sz, 2, C(.5,.6,.6), C(.4,.4,.4));
	}
	
	float fontSz = gm->fontSize;
	if(sz.y - (2*2) < fontSz) fontSz = sz.y - (2*2);
	
	gm->curZ += 0.01;
	GUI_TextLineCentered_(gm, text, strlen(text), (Vector2){tl.x, tl.y - fontSz*.25}, sz, "Arial", fontSz, C(.8,.8,.8));
	gm->curZ -= 0.01;
	
	return *state;
}


// returns true if checked
int GUI_Checkbox_(GUIManager* gm, void* id, Vector2 tl, char* label, int* state) {
	
	float bs = gm->checkboxBoxSize;
	Vector2 boxSz = {bs, bs};
	
	
	
	if(GUI_MouseInside_(gm, tl, boxSz)) {
		HOT(id);
	}
	
	if(gm->activeID == id) {
		if(GUI_MouseWentUp(gm, 0)) {
			if(gm->hotID == id) {
				*state = !*state;
			}
			ACTIVE(NULL);
		}
	}
	else if(gm->hotID == id) {
		if(GUI_MouseWentDown(gm, 0)) ACTIVE(id);
	}

	
	if(gm->hotID == id || *state) {
		GUI_BoxFilled_(gm, tl, boxSz, 2, C(.8,.6,.3), C(.7,.4,.2));
	}
	else {
		GUI_BoxFilled_(gm, tl, boxSz, 2, C(.5,.6,.6), C(.4,.4,.4));
	}
	
	float fontSz = gm->fontSize;
//	if(sz.y - (2*2) < fontSz) fontSz = sz.y - (2*2);
	
	GUI_TextLine_(gm, label, strlen(label), (Vector2){tl.x + bs + 4, tl.y + fontSz*.75}, "Arial", fontSz, C(.8,.8,.8));
	
	return *state;
}

// returns true if *this* radio button is active
int GUI_RadioButton_(GUIManager* gm, void* id, Vector2 tl, char* label, void** state, int isDefault) {
	
	float bs = gm->checkboxBoxSize;
	Vector2 boxSz = {bs, bs};
	
	if(*state == NULL && isDefault) {
		*state = id;
	}
	
	if(GUI_MouseInside_(gm, tl, boxSz)) {
		HOT(id);
	}
	
	if(gm->activeID == id) {
		if(GUI_MouseWentUp(gm, 0)) {
			if(gm->hotID == id) {
				*state = id;
			}
			ACTIVE(NULL);
		}
	}
	else if(gm->hotID == id) {
		if(GUI_MouseWentDown(gm, 0)) ACTIVE(id);
	}

	
	if(gm->hotID == id || *state == id) {
		GUI_CircleFilled_(gm, tl, bs, 2, C(.8,.6,.3), C(.7,.4,.2));
	}
	else {
		GUI_CircleFilled_(gm, tl, bs, 2, C(.5,.6,.6), C(.4,.4,.4));
	}
	
	float fontSz = gm->fontSize;
//	if(sz.y - (2*2) < fontSz) fontSz = sz.y - (2*2);
	
	GUI_TextLine_(gm, label, strlen(label), (Vector2){tl.x + bs + 4, tl.y + fontSz*.75}, "Arial", fontSz, C(.8,.8,.8));
	
	return *state == id;
}


// returns 
int GUI_SelectBox_(GUIManager* gm, void* id, Vector2 tl, Vector2 sz, char** options, int* selectedOption) {
	
	
	if(GUI_MouseInside_(gm, tl, sz)) {
		HOT(id);
	}
	
	if(gm->activeID == id) {
//		if(GUI_MouseWentUp(gm, 0)) {
//			if(gm->hotID == id) {
//				*state = !*state;
//			}
//			ACTIVE(NULL);
//		}
	}
	else if(gm->hotID == id) {
		if(GUI_MouseWentDown(gm, 0)) ACTIVE(id);
	}

	
	if(gm->hotID == id) {
		GUI_BoxFilled_(gm, tl, sz, 2, C(.8,.6,.3), C(.7,.4,.2));
	}
	else {
		GUI_BoxFilled_(gm, tl, sz, 2, C(.5,.6,.6), C(.4,.4,.4));
	}
	
	float fontSz = gm->fontSize;
	if(sz.y - (2*2) < fontSz) fontSz = sz.y - (2*2);


	
	if(gm->activeID == id) {
		// draw the dropdown
		gm->curZ += 1000.01;
		
		gm->curZ += 0.01;
		int cnt = 0;
		for(char** p = options; *p; p++) {
			cnt++;
			GUI_TextLineCentered_(gm, *p, strlen(*p), V(tl.x, sz.y * cnt + tl.y - fontSz*.25), sz, "Arial", fontSz, C(.8,.8,.8));
		}
		
		gm->curZ -= 0.01;
		
		
		Vector2 boxSz = {sz.x, sz.y * cnt};
		
		GUI_BoxFilled_(gm, V(tl.x, tl.y + sz.y), boxSz, 2, C(.8,.6,.3), C(.7,.4,.2));
		
		gm->curZ -= 1000.01;
	}
	
	gm->curZ += 0.01;
	GUI_TextLineCentered_(gm, options[*selectedOption], strlen(options[*selectedOption]), (Vector2){tl.x, tl.y - fontSz*.25}, sz, "Arial", fontSz, C(.8,.8,.8));
	gm->curZ -= 0.01;
	
	return 1;
} 


struct intedit_data {
	int cursorPos;
	float blinkTimer;
	
	long lastValue;
	char buffer[64];
};


// returns true on a change
int GUI_IntEdit_(GUIManager* gm, void* id, Vector2 tl, Vector2 sz, long* num) {
	int ret = 0;
	int firstRun = 0;
	struct intedit_data* d;
	
	if(!(d = GUI_GetData_(gm, id))) {
		d = calloc(1, sizeof(*d));
		GUI_SetData_(gm, id, d, free);
		firstRun = 1;
	}
	
	
	if(GUI_MouseInside_(gm, tl, sz)) {
		HOT(id);
	}
	
	if(gm->hotID == id) {
		if(GUI_MouseWentDown(gm, 0)) {
			ACTIVE(id);
		}
	}
	
	if(gm->activeID == id) {
		GUI_BoxFilled_(gm, tl, sz, 2, C(.8,.6,.3), C(.7,.4,.2));
	}
	else {
		GUI_BoxFilled_(gm, tl, sz, 2, C(.5,.6,.6), C(.4,.4,.4));
	}
	
	
	float fontSz = gm->fontSize;
	if(sz.y - (2*2) < fontSz) fontSz = sz.y - (2*2);
	
	GUIFont* font = GUI_FindFont(gm, "Arial");
	if(!font) font = gm->defaults.font;
	

	// handle input
	if(gm->activeID == id) {
		VEC_EACHP(&gm->keysReleased, i, e) {
			switch(e->character) {
				case '0': case '1': case '2':
				case '3': case '4': case '5':
				case '6': case '7': case '8': case '9':
					memmove(d->buffer + d->cursorPos + 1, d->buffer + d->cursorPos, strlen(d->buffer) - d->cursorPos + 1);
					d->buffer[d->cursorPos] = e->character;
					d->cursorPos++;
					*num = strtol(d->buffer, NULL, 10);
					d->blinkTimer = 0;
					
					ret = 1;
					break;
			}
			
			switch(e->keycode) {
				case XK_Left: 
					d->cursorPos = d->cursorPos < 1 ? 0 : d->cursorPos - 1; 
					d->blinkTimer = 0;
					break;
				case XK_Right: 
					d->cursorPos = d->cursorPos + 1 > strlen(d->buffer) ? strlen(d->buffer) : d->cursorPos + 1; 
					d->blinkTimer = 0;
					break;
					
				case XK_BackSpace: 
					memmove(d->buffer + d->cursorPos - 1, d->buffer + d->cursorPos, strlen(d->buffer) - d->cursorPos + 1);
					d->cursorPos = d->cursorPos > 0 ? d->cursorPos - 1 : 0;
					*num = strtol(d->buffer, NULL, 10);
					d->blinkTimer = 0;
					
					ret = 1;
					break;
					
				case XK_Delete: 
					memmove(d->buffer + d->cursorPos, d->buffer + d->cursorPos + 1, strlen(d->buffer) - d->cursorPos);
					*num = strtol(d->buffer, NULL, 10);
					d->blinkTimer = 0;
					
					ret = 1;
					break;
					
				case XK_Return: ACTIVE(NULL); break;
			}
		}
	}
	
	

	// refresh the buffer, maybe
	if(*num != d->lastValue || firstRun) {
		snprintf(d->buffer, 64, "%ld", *num);
		d->lastValue = *num;
	}
	
	// draw cursor
	if(gm->activeID == id) {
		if(d->blinkTimer < 0.5) { 
			float cursorOff = gui_getTextLineWidth(gm, font, fontSz, d->buffer, d->cursorPos);
			GUI_BoxFilled_(gm, V(tl.x + cursorOff , tl.y), V(2,sz.y), 0, C(.8,1,.3), C(.8,1,.3));
		}
		
		d->blinkTimer += gm->timeElapsed;
		d->blinkTimer = fmod(d->blinkTimer, 1.0);
	}
	
	gm->curZ += 0.01;
	GUI_TextLine_(gm, d->buffer, strlen(d->buffer), V(tl.x, tl.y +5+ fontSz * .75), "Arial", fontSz, &((Color4){.9,.9,.9,1}));
	gm->curZ -= 0.01;
	
	return ret;
}


// create a new window, push it to the stack, and set it current
void GUI_BeginWindow_(GUIManager* gm, void* id, Vector2 tl, Vector2 sz, float z, unsigned long flags) {
	GUIWindow* w, *p;
	
	p = VEC_TAIL(&gm->windowStack);
	w = GUIWindow_new(gm, p);
	
	w->id = id;
	w->z = z;
	w->flags = flags;
	
	// TODO: calculate the absolute clipping region somewhere
	w->clip = (AABB2){min: tl, max: {tl.x + sz.y, tl.y + sz.y}};
	
	VEC_PUSH(&p->children, w);
	
	VEC_PUSH(&gm->windowStack, w);
	gm->curWin = w;
}


// pop the window stack and set the previous window to be current
void GUI_EndWindow_(GUIManager* gm) {
	
	if(VEC_LEN(&gm->windowStack) <= 1) {
		fprintf(stderr, "Tried to pop root window\n");
		return;
	}	
	
	VEC_POP(&gm->windowStack, gm->curWin);
	
}


void GUI_Box_(GUIManager* gm, Vector2 tl, Vector2 sz, float width, Color4* borderColor) {
	Color4 clear = {0,0,0,0};
	gui_drawBoxBorder(gm, tl, sz, &gm->curClip, gm->curZ, &clear, width, borderColor);
}

void GUI_BoxFilled_(
	GUIManager* gm, 
	Vector2 tl, 
	Vector2 sz, 
	float width, 
	Color4* borderColor, 
	Color4* bgColor
) {
	gui_drawBoxBorder(gm, tl, sz, &gm->curClip, gm->curZ, bgColor, width, borderColor);
}

void GUI_CircleFilled_(
	GUIManager* gm, 
	Vector2 center, 
	float radius, 
	float borderWidth, 
	Color4* borderColor, 
	Color4* bgColor
) {
	GUIUnifiedVertex* v = GUIManager_reserveElements(gm, 1);
	
	*v++ = (GUIUnifiedVertex){
		.pos = {center.x, center.y, center.x + radius, center.y + radius},
		.clip = GUI_AABB2_TO_SHADER(gm->curClip),
		
		.guiType = 7, // ellipse
		
		.texIndex1 = borderWidth,
		
		.fg = GUI_COLOR4_TO_SHADER(*borderColor), 
		.bg = GUI_COLOR4_TO_SHADER(*bgColor), 
		.z = gm->curZ,
		.alpha = 1,
		.rot = 0,
	};
}

// no wrapping
void GUI_TextLine_(
	GUIManager* gm, 
	char* text, 
	size_t textLen, 
	Vector2 tl, 
	char* fontName, 
	float size, 
	Color4* color
) {
	GUIFont* font = GUI_FindFont(gm, fontName);
	if(!font) font = gm->defaults.font;
	
	gui_drawTextLineAdv(gm, 
		tl, (Vector2){99999999,99999999},
		&gm->curClip,
		color,
		font, size,
		GUI_TEXT_ALIGN_LEFT,
		gm->curZ,
		text, textLen
	);
}

// no wrapping
void GUI_TextLineCentered_(
	GUIManager* gm, 
	char* text, 
	size_t textLen, 
	Vector2 tl, 
	Vector2 sz, 
	char* fontName, 
	float size, 
	Color4* color
) {
	GUIFont* font = GUI_FindFont(gm, fontName);
	if(!font) font = gm->defaults.font;
	
	
	float b = sz.y - ((sz.y - size) / 2);
	
	
	gui_drawTextLineAdv(gm, (Vector2){tl.x, tl.y + b}, sz, &gm->curClip, color, font, size, GUI_TEXT_ALIGN_CENTER, gm->curZ, text, textLen);
}



int GUI_PointInBoxV_(GUIManager* gm, Vector2 tl, Vector2 size, Vector2 testPos) {
	if(!(testPos.x >= tl.x && 
		testPos.y >= tl.y &&
		testPos.x <= (tl.x + size.x) && 
		testPos.y <= (tl.y  + size.y))) {
		
		return 0;
	}
	
	return 1;
}
	
int GUI_PointInBox_(GUIManager* gm, AABB2 box, Vector2 testPos) {
	if(!boxContainsPoint2p(&box, &testPos)) return 0;
	if(!boxContainsPoint2p(&gm->curClip, &testPos)) return 0;
	
	return 1;
}
	
int GUI_MouseInside_(GUIManager* gm, Vector2 tl, Vector2 sz) {
	return GUI_PointInBoxV_(gm, tl, sz, gm->lastMousePos);
}

int GUI_MouseWentUp(GUIManager* gm, int button) {
	return gm->mouseWentUp;
}
int GUI_MouseWentDown(GUIManager* gm, int button) {
	return gm->mouseWentDown;
}



