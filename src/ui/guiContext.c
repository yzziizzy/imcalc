

#include <stdlib.h>
#include <stdio.h>

#include "gui.h"
#include "gui_internal.h"


GUIDrawContext* GDC_New(GUIHeader* subject) {
	GUIManager* gm = subject->gm;
	
	GUIDrawContext* gdc = pcalloc(gdc);
	
	
	gdc->subject = subject;
	gdc->gm = gm;
	gdc->clip = subject->absClip;
	
	gdc->z = subject->absZ + subject->z;
	gdc->absPos = subject->absTopLeft;
	gdc->font = gm->defaults.font;
	gdc->fontSize = gm->defaults.fontSize;
	gdc->fgColor = gm->defaults.textColor;
	gdc->bgColor = gm->defaults.windowBgColor;
	
	return gdc;
}

void GDC_Free(GUIDrawContext* gdc) {
	free(gdc);
}




void GDC_FgColor(GUIDrawContext* gdc, Color4 c) {
	gdc->fgColor = c;
}
void GDC_BgColor(GUIDrawContext* gdc, Color4 c) {
	gdc->bgColor = c;
}

void GDC_MoveTo(GUIDrawContext* gdc, Vector2 pos) {
	gdc->absPos = vAdd2(pos, gdc->subject->absTopLeft);
}

void GDC_MoveRel(GUIDrawContext* gdc, Vector2 amt) {
	gdc->absPos.x += amt.x;	
	gdc->absPos.y += amt.y;	
}
void GDC_Z(GUIDrawContext* gdc, float z) {
	gdc->z = gdc->subject->absZ + z;
}

void GDC_Gravity(GUIDrawContext* gdc, unsigned char grav) {
	gdc->placementGravity = grav;
}


// percentage of parent size
void GDC_MoveToPct(GUIDrawContext* gdc, Vector2 pos_pct) {
	printf("%s nyi\n", __func__);
}
void GDC_MoveRelPct(GUIDrawContext* gdc, Vector2 amt_pct) {
	printf("%s nyi\n", __func__);
}

void GDC_Box(GUIDrawContext* gdc, Vector2 size) {
	gui_drawBox(gdc->gm, gdc->absPos, size, &gdc->clip, gdc->z, &gdc->fgColor);
}

void GDC_BorderBox(GUIDrawContext* gdc, Vector2 size, float borderWidth) {
	gui_drawBoxBorder(
		gdc->gm, gdc->absPos, size, &gdc->clip, gdc->z, 
		&gdc->bgColor, borderWidth, &gdc->fgColor
	);
}

void GDC_Text(GUIDrawContext* gdc, char* text, size_t len) {
	
	gui_drawTextLineAdv(
		gdc->gm, gdc->absPos,
		(Vector2){999999999,999999999},
		&gdc->clip,
		&gdc->fgColor,
		gdc->font,
		gdc->fontSize,
		GUI_TEXT_ALIGN_LEFT,
		gdc->z,
		text, len
	);
}







