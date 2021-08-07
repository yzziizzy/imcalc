#ifndef __gputk_image_h__
#define __gputk_image_h__







typedef struct GUIImage {
	GUIHeader header;
	
	Vector2 offsetNorm;
	Vector2 sizeNorm;
	int texIndex;
	
	GLuint64 texHandle;
	GLuint customTexID;
	
} GUIImage;





GUIImage* GUIImage_New(GUIManager* gm, char* name);




typedef struct GUIRenderTarget {
	GUIHeader header;
	
	Vector2i screenRes;
	
	GLuint64 texHandle;
	RenderPipeline* rpl;
	
	
} GUIRenderTarget;

GUIRenderTarget* GUIRenderTarget_new(GUIManager* gm, Vector2 pos, Vector2 size, RenderPipeline* rpl);

void guiRenderTarget_SetScreenRes(GUIRenderTarget* rt, Vector2i newRes);

// also initialize GUIRenderTarget data
void gui_Image_Init(char* file);

#endif // __gputk_image_h__
