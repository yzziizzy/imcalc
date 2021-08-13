#ifndef __EACSMB_gui_internal_h__
#define __EACSMB_gui_internal_h__

// this file is for gui element implementations, not for general outside usage


typedef struct GUIRenderParams {
	Vector2 offset; // parent-defined absolute from the top left of the screen
	Vector2 size; // of the parent's client area that the child lives in
	AABB2 clip; // absolute clipping region
	float baseZ; // accumulated absolute Z from the parent
} GUIRenderParams;





void gui_headerInit(GUIHeader* gh, GUIManager* gm, struct gui_vtbl* vt, struct GUIEventHandler_vtbl* event_vt); 
void gui_defaultUpdatePos(GUIHeader* h, GUIRenderParams* grp, PassFrameParams* pfp);
void gui_selfUpdatePos(GUIHeader* gh, GUIRenderParams* grp, PassFrameParams* pfp);
void gui_columnUpdatePos(GUIHeader* gh, GUIRenderParams* grp, PassFrameParams* pfp);
GUIHeader* gui_defaultFindChild(GUIHeader* obj, char* name);

GUIHeader* GUIHeader_New(GUIManager* gm, struct gui_vtbl* vt, struct GUIEventHandler_vtbl* event_vt);

Vector2 gui_calcPosGrav(GUIHeader* h, GUIRenderParams* grp);
GUIHeader* gui_defaultHitTest(GUIHeader* h, Vector2 absTestPos);
GUIHeader* gui_defaultChildrenHitTest(GUIHeader* h, Vector2 absTestPos);
Vector2 gui_parent2ChildGrav(GUIHeader* child, GUIHeader* parent, Vector2 pt);

void gui_default_ParentResize(GUIHeader* root, GUIEvent* gev);
void gui_default_Delete(GUIHeader* h);

GUIUnifiedVertex* GUIManager_checkElemBuffer(GUIManager* gm, int count);
GUIUnifiedVertex* GUIManager_reserveElements(GUIManager* gm, int count);
void GUIManager_copyElements(GUIManager* gm, GUIUnifiedVertex* elems, int count);

void GUIHeader_render(GUIHeader* gh, PassFrameParams* pfp);
void GUIHeader_renderChildren(GUIHeader* gh, PassFrameParams* pfp);

void GUIHeader_updatePos(GUIHeader* go, GUIRenderParams* grp, PassFrameParams* pfp);


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

void gui_win_drawImg(
	GUIManager* gm, 
	GUIHeader* h, 
	char* name, 
	Vector2 tl, 
	Vector2 sz
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




typedef struct GUIDrawContext {
	unsigned char placementGravity;
	float z;
	float fontSize;
	Vector2 absPos;
	GUIFont* font;
	AABB2 clip;
	Color4 fgColor;
	Color4 bgColor;
	
	GUIHeader* subject;
	GUIManager* gm;
	
	struct GUIDrawContext* parent;
} GUIDrawContext;



GUIDrawContext* GDC_New(GUIHeader* subject);
void GDC_Free(GUIDrawContext* gdc);


void GDC_FgColor(GUIDrawContext* gdc, Color4 c);
void GDC_BgColor(GUIDrawContext* gdc, Color4 c);

void GDC_MoveTo(GUIDrawContext* gdc, Vector2 pos);
void GDC_MoveRel(GUIDrawContext* gdc, Vector2 amt);
void GDC_Z(GUIDrawContext* gdc, float z);
void GDC_Gravity(GUIDrawContext* gdc, unsigned char grav);


// percentage of parent size
void GDC_MoveToPct(GUIDrawContext* gdc, Vector2 pos_pct);
void GDC_MoveRelPct(GUIDrawContext* gdc, Vector2 amt_pct);

void GDC_Box(GUIDrawContext* gdc, Vector2 size);
void GDC_BorderBox(GUIDrawContext* gdc, Vector2 size, float borderWidth);

void GDC_Text(GUIDrawContext* gdc, char* text, size_t len);






#endif // __EACSMB_gui_internal_h__
