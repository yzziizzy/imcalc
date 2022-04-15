

#include "gui.h"
#include "gui_internal.h"




#define C(r,g,b)  (&((Color4){r,g,b,1.0}))
#define C4(r,g,b,a)  (&((Color4){r,g,b,a}))



// returns true if clicked
int GUI_Button_(GUIManager* gm, void* id, Vector2 tl, Vector2 sz, char* text) {
	
	int result = 0;
	
	if(GUI_MouseInside_(gm, tl, sz)) {
		gm->hotID = id;
	}
	
	if(gm->activeID == id) {
		if(GUI_MouseWentUp(gm, 0)) {
			if(gm->hotID == id) result = 1;
			gm->activeID = NULL;
		}
	}
	else if(gm->hotID == id) {
		if(GUI_MouseWentDown(gm, 0)) gm->activeID = id;
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
		gm->hotID = id;
	}
	
	if(gm->activeID == id) {
		if(GUI_MouseWentUp(gm, 0)) {
			if(gm->hotID == id) {
				*state = !*state;
			}
			gm->activeID = NULL;
		}
	}
	else if(gm->hotID == id) {
		if(GUI_MouseWentDown(gm, 0)) gm->activeID = id;
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
		gm->hotID = id;
	}
	
	if(gm->activeID == id) {
		if(GUI_MouseWentUp(gm, 0)) {
			if(gm->hotID == id) {
				*state = !*state;
			}
			gm->activeID = NULL;
		}
	}
	else if(gm->hotID == id) {
		if(GUI_MouseWentDown(gm, 0)) gm->activeID = id;
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
		gm->hotID = id;
	}
	
	if(gm->activeID == id) {
		if(GUI_MouseWentUp(gm, 0)) {
			if(gm->hotID == id) {
				*state = id;
			}
			gm->activeID = NULL;
		}
	}
	else if(gm->hotID == id) {
		if(GUI_MouseWentDown(gm, 0)) gm->activeID = id;
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



int GUI_IntEdit_(GUIManager* gm, void* id, Vector2 tl, Vector2 sz, long num) {
	
	char buf[64];
	
	snprintf(buf, 64, "%ld", num);
	GUI_TextLine_(gm, buf, strlen(buf), tl, "Arial", 15, &((Color4){.9,.9,.9,1}));
	
	return 0;
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



