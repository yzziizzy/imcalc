#ifndef __gpuedit_app_h__
#define __gpuedit_app_h__

#include "common_math.h"
#include "common_gl.h"

#include "settings.h"
#include "window.h"
#include "ui/gui.h"
#include "calc.h"


typedef struct AppScreen {
	
	float aspect;
	Vector2 wh;
	
	int resized;
	
} AppScreen;




typedef struct PerViewUniforms {
	Matrix view;
	Matrix proj;
} PerViewUniforms;


typedef struct PerFrameUniforms {
	float wholeSeconds;
	float fracSeconds;
} PerFrameUniforms;


typedef struct AppState {
	
	AppScreen screen;
	
	GlobalSettings globalSettings;
	ThemeSettings themeSettings;
	
	GUIManager* gui;
	RenderPass* guiPass;
	
	calculator_t* calc;

	MatrixStack view;
	MatrixStack proj;
	
	Matrix invView; // TODO: rename these
	Matrix invProj;
	Matrix mProjWorld;
	
	Vector2 cursorPos;
// 	Vector3 cursorPos;
	int cursorIndex;

	
	TextureAtlas* ta;

	
	double lastFrameTime; // frameTime from the previous frame
	double lastFrameDrawTime; // the cost of rendering the previous frame, minus any sleeping
	double frameTime; // ever incrementing time of the this frame
	double frameSpan; // the length of this frame, since last frame
	uint64_t frameCount; // ever incrementing count of the number of frames processed
	
	// performance counters
	struct {
		double preframe;
		double draw;
	} perfTimes;
	
	struct {
		QueryQueue draw; 
		QueryQueue gui; 
		
	} queries;

	
	
	
} AppState;






void initApp(XStuff* xs, AppState* gs, int argc, char* argv[]);
void initAppGL(XStuff* xs, AppState* gs);

void AppState_UpdateSettings(AppState* as, GlobalSettings* gs);

void appLoop(XStuff* xs, AppState* gs, InputState* is);
void SetUpPDP(AppState* as, PassDrawParams* pdp);
void initRenderLoop(AppState* gs);





#endif // __gpuedit_app_h__
