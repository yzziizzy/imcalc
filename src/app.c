
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <pty.h>
#include <termios.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "c3dlas/c3dlas.h"
#include "c_json/json.h"
#include "json_gl.h"

#include "sti/sti.h"

#include "utilities.h"
#include "shader.h"
#include "texture.h"
#include "window.h"
#include "app.h"
//#include "commands.h"
#include "ui/gui.h"






RenderPipeline* rpipe;


// in renderLoop.c, a temporary factoring before a proper renderer is designed
void drawFrame(XStuff* xs, AppState* as, InputState* is);
void setupFBOs(AppState* as, int resized);


// MapBlock* map;
// TerrainBlock* terrain;

void resize_callback(XStuff* xs, void* gm_) {
	GUIManager* gm = (GUIManager*)gm_;
	/*
	GUIEvent gev = {
		.type = GUIEVENT_ParentResize,
		.size = {.x = xs->winSize.x, .y = xs->winSize.y},
		.originalTarget = gm->root,
	};
	
	GUIHeader_TriggerEvent(gm->root, &gev);*/
}

static struct child_process_info* cc;

// nothing in here can use opengl at all.
void initApp(XStuff* xs, AppState* as, int argc, char* argv[]) {
	
	srand((unsigned int)time(NULL));
	
	
	as->lastFrameTime = getCurrentTime();
	as->lastFrameDrawTime = 0;
	
	as->ta = TextureAtlas_alloc(&as->globalSettings);
	as->ta->width = 32;
	TextureAtlas_addFolder(as->ta, "icon", "/usr/share/gpuedit/images", 0);
	TextureAtlas_finalize(as->ta);
// 	
	
	
	
	as->gui = GUIManager_alloc(as->globalSettings.GUI_GlobalSettings);
	as->gui->ta = as->ta;
	xs->onResize = resize_callback;
	xs->onResizeData = as->gui;
	decodeHexColorNorm(as->globalSettings.GUI_GlobalSettings->tabBorderColor, (float*)&(as->gui->defaults.tabBorderColor));
	
	as->gui->windowTitleSetFn = (void*)XStuff_SetWindowTitle;
	as->gui->windowTitleSetData = xs;
	
	as->gui->mouseCursorSetFn = (void*)XStuff_SetMouseCursor;
	as->gui->mouseCursorSetData = xs;
	

	
	
	
	// command line args
	for(int i = 1; i < argc; i++) {
		char* a = argv[i];
		
		// for debugging
//		if(0 == strcmp(a, "--disable-save")) {
//			printf("Buffer saving disabled.\n");
//			g_DisableSave = 1;
//		}
		
		
	}
	

	as->frameCount = 0;
	
	as->debugMode = 0;
	
	int ww, wh;
	ww = xs->winAttr.width;
	wh = xs->winAttr.height;
	
	as->screen.wh.x = (float)ww;
	as->screen.wh.y = (float)wh;
	as->gui->screenSize = (Vector2i){ww, wh};
	
	as->screen.aspect = as->screen.wh.x / as->screen.wh.y;
	as->screen.resized = 0;
	

	// set up matrix stacks
	MatrixStack* view, *proj;
	
	view = &as->view;
	proj = &as->proj;
	
	msAlloc(2, view);
	msAlloc(2, proj);

	msIdent(view);
	msIdent(proj);

}

void initAppGL(XStuff* xs, AppState* as) {
	glerr("left over error on app init");
	
	//gen_sdf_test_samples("Arial", '%');
	//exit(1);
	
	//TextureAtlas_initGL(as->ta, &as->globalSettings);
	
	GUIManager_initGL(as->gui);
	as->guiPass = GUIManager_CreateRenderPass(as->gui);
	

	initRenderLoop(as);
	initRenderPipeline();

	/*
	getPrintGLEnum(GL_MAX_COLOR_ATTACHMENTS, "meh");
	*/
	
	initTextures();
}

Vector2i viewWH = {
	.x = 0,
	.y = 0
};
void checkResize(XStuff* xs, AppState* as) {
	if(viewWH.x != xs->winAttr.width || viewWH.y != xs->winAttr.height) {
		
		// TODO: destroy all the textures too
		
		//printf("screen 0 resized\n");
		
		viewWH.x = xs->winAttr.width;
		viewWH.y = xs->winAttr.height;
		
		as->screen.wh.x = (float)xs->winAttr.width;
		as->screen.wh.y = (float)xs->winAttr.height;
		as->gui->screenSize = (Vector2i){xs->winAttr.width, xs->winAttr.height};
		
		as->screen.aspect = as->screen.wh.x / as->screen.wh.y;
		
		as->screen.resized = 1;
		
	}
}



void handleEvent(AppState* as, InputState* is, InputEvent* ev) {
// 	printf("%d %c/* */%d-\n", ev->type, ev->character, ev->keysym);
	
	switch(ev->type) {
		case EVENT_KEYUP:
		case EVENT_KEYDOWN:
			GUIManager_HandleKeyInput(as->gui, is, ev);
			break;
		case EVENT_MOUSEUP:
		case EVENT_MOUSEDOWN:
			GUIManager_HandleMouseClick(as->gui, is, ev);
			break;
		case EVENT_MOUSEMOVE:
			GUIManager_HandleMouseMove(as->gui, is, ev);
			break;
	}
}


void prefilterEvent(AppState* as, InputState* is, InputEvent* ev) {
	// drags, etc
	
	// TODO: fix; passthrough atm
	handleEvent(as, is, ev);
	
}




void initRenderLoop(AppState* as) {
	
	// timer queries

	query_queue_init(&as->queries.gui);
	
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
	glBindTexture(GL_TEXTURE_2D, 0);
}



void SetUpPDP(AppState* as, PassDrawParams* pdp) {
	
	pdp->mWorldView = msGetTop(&as->view);
	pdp->mViewProj = msGetTop(&as->proj);
	
	pdp->mProjView = &as->invProj;
	pdp->mViewWorld = &as->invView;
	
	mInverse(pdp->mViewProj, &as->invProj);
	mInverse(pdp->mWorldView, &as->invView);
	
// 	pdp->eyeVec = as->eyeDir;
// 	pdp->eyePos = as->eyePos;
	pdp->targetSize = (Vector2i){as->screen.wh.x, as->screen.wh.y};
	pdp->timeSeconds = (float)(long)as->frameTime;
	pdp->timeFractional = as->frameTime - pdp->timeSeconds;
	
}






//#define PERF(...) __VA_ARGS__
#define PERF(...) 


void appLoop(XStuff* xs, AppState* as, InputState* is) {
	
	// in seconds
	double frameTimeTarget = 1.0 / (as->globalSettings.AppState_frameRate);
	double lastFrameCost = 0.0;
	double lastFrameStart = getCurrentTime();
	
	// main running loop
	while(1) {
		InputEvent iev;
		PassDrawParams pdp;		
		PassFrameParams pfp;
		double frameCostStart;
		double frameStart;
		double lastFrameSpan;
		double totalEventsTime = 0.0;
		double totalSleepTime = 0.0;
		
		double now;
		PERF(printf("\n\n"));
		
		
		
		frameStart = getCurrentTime();
		lastFrameSpan = frameStart - lastFrameStart;
		PERF(printf("last frame span: %fus [%ffps]\n", 
			lastFrameSpan * 1000000.0,
			1.0 / lastFrameSpan
		));
		lastFrameStart = frameStart;
		
		
		double estimatedSleepTime;
		
		// frameCost is the amount of time it takes to generate a new frame
		estimatedSleepTime = (frameTimeTarget - lastFrameCost) * .98;
		
		struct timespec sleepInterval;
		sleepInterval.tv_sec = estimatedSleepTime;
		sleepInterval.tv_nsec = fmod(estimatedSleepTime, 1.0) * 1000000000.0;
			
		
		// sleep through the beginning of the frame, checking events occasionally
		for(int i = 0; ; i++) {
			int drawRequired = 0;
			PERF(now = getCurrentTime());
				
			if(processEvents(xs, is, &iev, -1)) {
				// handle the event
				prefilterEvent(as, is, &iev);
				PERF(totalEventsTime += timeSince(now));
			}
			else { // no events, see if we need to sleep longer
				now = getCurrentTime();
				
				if(now - frameStart < estimatedSleepTime) {
					
					nanosleep(&sleepInterval, &sleepInterval);
					
					PERF(totalSleepTime += timeSince(now));
				}
				else {
					break; // time to renter the frame
				}
			}
		}
		
		
		
		PERF(printf("events / sleep / total: %fus / %fus / %fus\n", 
			totalEventsTime * 1000000.0,
			totalSleepTime * 1000000.0,
			timeSince(frameStart) * 1000000.0
		));
		PERF(totalSleepTime = 0);
		PERF(totalEventsTime = 0);
		
		frameCostStart = getCurrentTime();
		
		
		//  internal frame time is at the start of render, for the least lag
		as->frameTime = getCurrentTime();
		as->frameSpan = as->frameTime - as->lastFrameTime;		
		as->lastFrameTime = as->frameTime;
		
		
		checkResize(xs, as);
		
			
		/*
		if(frameCounter == 0) {
			
			uint64_t qtime;
	
			#define query_update_gui(qname)		\
			if(!query_queue_try_result(&as->queries.qname, &qtime)) {\
				sdtime = ((double)qtime) / 1000000.0;\
			}\
			snprintf(frameCounterBuf, 128, #qname ":  %.2fms", sdtime);\
			GUIText_setString(gt_##qname, frameCounterBuf);
	
	
			//query_update_gui(gui);
			
			lastPoint = now;
		}
	*/
	
	
			
		
		PERF(now = getCurrentTime());
		
		SetUpPDP(as, &pdp);
		
		pfp.dp = &pdp;
		pfp.timeElapsed = as->frameSpan;
		pfp.appTime = as->frameTime; // this will get regenerated from save files later
		pfp.wallTime = as->frameTime;
		
		glexit("");
		
		glViewport(0, 0, as->screen.wh.x, as->screen.wh.y);
		PERF(printf("pre-render time: %fus\n", timeSince(now) * 1000000.0));
		
		
	// 	glEnable(GL_CULL_FACE);
	// 	glFrontFace(GL_CW); // this is backwards, i think, because of the scaling inversion for z-up
		
		// the cpu will generally block here
		PERF(now = getCurrentTime());
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		PERF(printf("glBindFramebuffer: %fus\n", timeSince(now) * 1000000.0));
		
		PERF(now = getCurrentTime());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		PERF(printf("glClear: %fus\n", timeSince(now) * 1000000.0));
		
		
		// frameCost is the amount of time it takes to generate a new frame
		frameCostStart = getCurrentTime();
		
		
		glexit("");
	// 	query_queue_start(&as->queries.gui);
		
	// 	glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glexit("");
		
		glDisable(GL_DEPTH_TEST);
		
		PERF(now = getCurrentTime());
		RenderPass_preFrameAll(as->guiPass, &pfp);
		RenderPass_renderAll(as->guiPass, pfp.dp);
		RenderPass_postFrameAll(as->guiPass);
		PERF(printf("gui render time: %fus\n", timeSince(now) * 1000000.0));
		
	// 	query_queue_stop(&as->queries.gui);
	
		glexit("");
		
		msPop(&as->view);
		msPop(&as->proj);

		lastFrameCost = timeSince(frameCostStart);
		PERF(printf("frame cost: %fus\n", lastFrameCost * 1000000.0));
		
//		now = getCurrentTime();
//		glFinish();
//		printf("glFinish: %fus\n", timeSince(now) * 1000000.0);
		
		PERF(now = getCurrentTime());
		glXSwapBuffers(xs->display, xs->clientWin);
		PERF(printf("glXSwapBuffers: %fus\n", timeSince(now) * 1000000.0));
	
		
		
		
		as->screen.resized = 0;
		

		now = getCurrentTime();
		as->lastFrameDrawTime = now - as->frameTime; 
	
		as->perfTimes.draw = as->lastFrameDrawTime;
	
	
//		PERF(now = getCurrentTime());
//		GUIManager_Reap(as->gui);
//		PERF(printf("GUIManager_Reap: %fus\n", timeSince(now) * 1000000));
	}
}


