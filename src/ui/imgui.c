

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
	
	gm->curZ += 0.01;
	GUI_TextLineCentered_(gm, text, strlen(text), tl, sz, "Arial", 20, C(.8,.8,.8));
	gm->curZ -= 0.01;
	
	return result;
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


