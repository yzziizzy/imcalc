#ifndef __EACSMB_gui_internal_h__
#define __EACSMB_gui_internal_h__

// this file is for gui element implementations, not for general outside usage


typedef struct GUIRenderParams {
	Vector2 offset; // parent-defined absolute from the top left of the screen
	Vector2 size; // of the parent's client area that the child lives in
	AABB2 clip; // absolute clipping region
	float baseZ; // accumulated absolute Z from the parent
} GUIRenderParams;






GUIUnifiedVertex* GUIManager_checkElemBuffer(GUIManager* gm, int count);
GUIUnifiedVertex* GUIManager_reserveElements(GUIManager* gm, int count);
void GUIManager_copyElements(GUIManager* gm, GUIUnifiedVertex* elems, int count);

static inline AABB2 gui_clipTo(AABB2 parent, AABB2 child) {
	return (AABB2){
		.min.x = fmax(parent.min.x, child.min.x),
		.min.y = fmax(parent.min.y, child.min.y),
		.max.x = fmin(parent.max.x, child.max.x),
		.max.y = fmin(parent.max.y, child.max.y),
	};
}


#define GUI_TEXT_ALIGN_LEFT 0x00
#define GUI_TEXT_ALIGN_RIGHT 0x01
#define GUI_TEXT_ALIGN_CENTER 0x02


void GUI_Box_(
	GUIManager* gm, 
	Vector2 tl, 
	Vector2 sz, 
	float width, 
	Color4* borderColor
);

void GUI_BoxFilled_(
	GUIManager* gm, 
	Vector2 tl, 
	Vector2 sz, 
	float width, 
	Color4* borderColor, 
	Color4* bgColor
);

// no wrapping
void GUI_TextLine_(
	GUIManager* gm, 
	char* text, 
	size_t textLen, 
	Vector2 tl, 
	char* fontName, 
	float size, 
	Color4* color
);

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
);




void gui_drawBox(
	GUIManager* gm, 
	Vector2 tl, 
	Vector2 sz, 
	AABB2* clip, 
	float z, 
	Color4* color
);

void gui_drawBoxBorder(
	GUIManager* gm, 
	Vector2 tl, 
	Vector2 sz, 
	AABB2* clip, 
	float z, 
	Color4* bgColor,
	float borderWidth,
	Color4* borderColor
);


void gui_drawTriangle(
	GUIManager* gm, 
	Vector2 centroid, 
	float baseWidth, 
	float height, 
	float rotation,
	AABB2* clip, 
	float z, 
	Color4* bgColor
);

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
);

void gui_drawImg(
	GUIManager* gm, 
	char* name, 
	Vector2 tl, 
	Vector2 sz, 
	AABB2* clip, 
	float z
);




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
);

// stops on linebreak
void gui_drawTextLine(
	GUIManager* gm,
	Vector2 tl,  
	Vector2 sz,  
	AABB2* clip,  
	struct Color4* color,
	float z,
	char* txt, 
	size_t charCount
);

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
);


float gui_getTextLineWidth(
	GUIManager* gm,
	GUIFont* font,
	float fontsize,
	char* txt, 
	size_t charCount
);


float gui_getDefaultUITextWidth(
	GUIManager* gm,
	char* txt, 
	size_t maxChars
);

void gui_drawVCenteredTextLine(
	GUIManager* gm,
	Vector2 tl,  
	Vector2 sz,
	AABB2* clip,
	struct Color4* color,
	float z,
	char* txt, 
	size_t charCount
);



int GUI_MouseInside_(GUIManager* gm, Vector2 tl, Vector2 sz);
int GUI_MouseWentUp(GUIManager* gm, int button);
int GUI_MouseWentDown(GUIManager* gm, int button);

// returns true if clicked
int GUI_Button_(GUIManager* gm, void* id, Vector2 tl, Vector2 sz, char* text);

void GUI_Box_(GUIManager* gm, Vector2 tl, Vector2 sz, float width, Color4* borderColor);

void GUI_BoxFilled_(
	GUIManager* gm, 
	Vector2 tl, 
	Vector2 sz, 
	float width, 
	Color4* borderColor, 
	Color4* bgColor
);

	
// no wrapping
void GUI_TextLine_(
	GUIManager* gm, 
	char* text, 
	size_t textLen, 
	Vector2 tl, 
	char* fontName, 
	float size, 
	Color4* color
);

int GUI_PointInBoxV_(GUIManager* gm, Vector2 tl, Vector2 size, Vector2 testPos);
int GUI_PointInBox_(GUIManager* gm, AABB2 box, Vector2 testPos);








#endif // __EACSMB_gui_internal_h__
