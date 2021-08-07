
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



GLuint proj_ul, view_ul, model_ul;


int g_DisableSave = 0; // debug flag to disable saving

RenderPipeline* rpipe;


// in renderLoop.c, a temporary factoring before a proper renderer is designed
void drawFrame(XStuff* xs, AppState* as, InputState* is);
void setupFBOs(AppState* as, int resized);


// MapBlock* map;
// TerrainBlock* terrain;

void resize_callback(XStuff* xs, void* gm_) {
	GUIManager* gm = (GUIManager*)gm_;
	
	GUIEvent gev = {
		.type = GUIEVENT_ParentResize,
		.size = {.x = xs->winSize.x, .y = xs->winSize.y},
		.originalTarget = gm->root,
	};
	
	GUIHeader_TriggerEvent(gm->root, &gev);
}

static struct child_process_info* cc;

// nothing in here can use opengl at all.
void initApp(XStuff* xs, AppState* as, int argc, char* argv[]) {
	
	srand((unsigned int)time(NULL));
	
	char* args[] = {
		"-al",
		NULL,
	};
	
	as->lastFrameTime = getCurrentTime();
	as->lastFrameDrawTime = 0;
	/*
	struct child_process_info* cc;
	cc = AppState_ExecProcessPipe(NULL, "ls", args);

	char buf[1024];
	while(!feof(cc->f_stdout)) {
		size_t sz = fread(buf, 1, 1024, cc->f_stdout);
		if(sz) printf("'%*s'\n", sz, buf);
	}	
	*/	
	// this costs 5mb of ram
// 	json_gl_init_lookup();
	
	
	as->ta = TextureAtlas_alloc(&as->globalSettings);
	as->ta->width = 32;
	TextureAtlas_addFolder(as->ta, "icon", "/usr/share/gpuedit/images", 0);
	TextureAtlas_finalize(as->ta);
// 	
	
	/*
	Highlighter* ch = pcalloc(ch);
	initCStyles(ch);
	Highlighter_PrintStyles(ch);
	Highlighter_LoadStyles(ch, "config/c_colors.txt");
	*/
	
	as->gui = GUIManager_alloc(as->globalSettings.GUI_GlobalSettings);
	as->gui->ta = as->ta;
	xs->onResize = resize_callback;
	xs->onResizeData = as->gui;
	decodeHexColorNorm(as->globalSettings.GUI_GlobalSettings->tabBorderColor, (float*)&(as->gui->defaults.tabBorderColor));
	decodeHexColorNorm(as->globalSettings.GUI_GlobalSettings->tabActiveBgColor, (float*)&(as->gui->defaults.tabActiveBgColor));
	decodeHexColorNorm(as->globalSettings.GUI_GlobalSettings->tabHoverBgColor, (float*)&(as->gui->defaults.tabHoverBgColor));
	decodeHexColorNorm(as->globalSettings.GUI_GlobalSettings->tabBgColor, (float*)&(as->gui->defaults.tabBgColor));
	decodeHexColorNorm(as->globalSettings.GUI_GlobalSettings->tabTextColor, (float*)&(as->gui->defaults.tabTextColor));
	
	as->gui->windowTitleSetFn = (void*)XStuff_SetWindowTitle;
	as->gui->windowTitleSetData = xs;
	
	as->gui->mouseCursorSetFn = (void*)XStuff_SetMouseCursor;
	as->gui->mouseCursorSetData = xs;
	
	char* flag_names[] = {
		"scrollToCursor",
		"rehighlight",
		"undoSeqBreak",
		"hideMouse",
		"centerOnCursor",
		NULL,
	};
	
	for(int i = 0; flag_names[i]; i++) 
		GUIManager_AddCommandFlag(as->gui, flag_names[i]);
	
	
	/*
	struct {char* n; uint16_t id;} elem_names[] = {
	#define X(a) { #a, CUSTOM_ELEM_TYPE_##a },
	COMMANDELEMTYPE_LIST
		{NULL, 0},
	#undef X
	};
	
	for(int i = 0; elem_names[i].n; i++) 
		GUIManager_AddCommandElement(as->gui, elem_names[i].n, elem_names[i].id);
	
	
	struct {char* en; char* n; uint32_t id;} cmd_names[] = {
	#define X(a, b) { #a, #b, a##Cmd_##b },
	COMMANDTYPE_LIST
		{NULL, NULL, 0},
	#undef X
	};
	
	for(int i = 0; cmd_names[i].n; i++) 
		GUIManager_AddCommand(as->gui, cmd_names[i].en, cmd_names[i].n, cmd_names[i].id);
	*/
	
//	char* homedir = getenv("HOME");
//	char* tmp = path_join(homedir, ".gpuedit/commands.json");

//	CommandList_loadJSONFile(as->gui, "/home/steve/projects/gpuedit/config/commands_new.json");
//	CommandList_loadJSONFile(as->gui, tmp);
//	free(tmp);

	
	
	
	// command line args
	for(int i = 1; i < argc; i++) {
		char* a = argv[i];
		
		// for debugging
		if(0 == strcmp(a, "--disable-save")) {
			printf("Buffer saving disabled.\n");
			g_DisableSave = 1;
		}
		
		
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
	
	TextureAtlas_initGL(as->ta, &as->globalSettings);
	
	GUIManager_initGL(as->gui);
	as->guiPass = GUIManager_CreateRenderPass(as->gui);
	

	initRenderLoop(as);
	initRenderPipeline();
	

	/*
	getPrintGLEnum(GL_MAX_COLOR_ATTACHMENTS, "meh");
	getPrintGLEnum(GL_MAX_DRAW_BUFFERS, "meh");
	getPrintGLEnum(GL_MAX_FRAMEBUFFER_WIDTH, "meh");
	getPrintGLEnum(GL_MAX_FRAMEBUFFER_HEIGHT, "meh");
	getPrintGLEnum(GL_MAX_FRAMEBUFFER_LAYERS, "meh");
	getPrintGLEnum(GL_MAX_FRAMEBUFFER_SAMPLES, "meh");
	getPrintGLEnum(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, "meh");
	getPrintGLEnum(GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS, "meh");
	getPrintGLEnum(GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS, "meh");
	getPrintGLEnum(GL_MAX_TEXTURE_IMAGE_UNITS, "meh");
	getPrintGLEnum(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, "meh");
	getPrintGLEnum(GL_MAX_ARRAY_TEXTURE_LAYERS, "meh");
	getPrintGLEnum(GL_MAX_SAMPLES, "meh");
	getPrintGLEnum(GL_MAX_VERTEX_ATTRIBS, "meh");
	getPrintGLEnum(GL_MIN_PROGRAM_TEXEL_OFFSET, "meh");
	getPrintGLEnum(GL_MAX_PROGRAM_TEXEL_OFFSET, "meh");
	getPrintGLEnum(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, "meh");
	getPrintGLEnum(GL_MAX_UNIFORM_BLOCK_SIZE, "meh");
	getPrintGLEnum(GL_MAX_TEXTURE_SIZE, "meh");
	
	*/
	
	
	
	initTextures();

	
	
	
/*	
	json_file_t* guijsf;
	
	guijsf = json_load_path("assets/config/main_ui.json");
	json_value_t* kids;
	json_obj_get_key(guijsf->root, "children", &kids);
	
	GUICL_LoadChildren(as->gui, as->gui->root, kids);
	
	GUIHeader* ps = GUIHeader_findChild(as->gui->root, "perfstats");
	gt_terrain = GUIHeader_findChild(ps, "terrain");
	gt_solids = GUIHeader_findChild(ps, "solids");
	gt_selection = GUIHeader_findChild(ps, "selection");
	gt_decals = GUIHeader_findChild(ps, "decals");
	gt_emitters = GUIHeader_findChild(ps, "emitters");
	gt_effects = GUIHeader_findChild(ps, "effects");
	gt_lighting = GUIHeader_findChild(ps, "lighting");
	gt_sunShadow = GUIHeader_findChild(ps, "sunShadow");
	gt_shading = GUIHeader_findChild(ps, "shading");
	gt_gui = GUIHeader_findChild(ps, "gui");
	
	
*/

		

}


// effectively a better, asynchronous version of system()
void AppState_ExecProcess(AppState* as, char* execPath, char* args[]) {
	
	
	int childPID = fork();
	
	if(childPID == -1) {
		fprintf(stderr, "failed to fork trying to execute '%s'\n", execPath);
		perror(strerror(errno));
		return;
	}
	else if(childPID == 0) { // child process
		
		execvp(execPath, args); // never returns if successful
		
		fprintf(stderr, "failed to execute '%s'\n", execPath);
		exit(1); // kill the forked process 
	}
	else { // parent process
		// TODO: put the pid and info into an array somewhere
		
// 		int status;
		// returns 0 if nothing happened, -1 on error
// 		pid = waitpid(childPID, &status, WNOHANG);
		
	}
}

// http://git.suckless.org/st/file/st.c.html#l786

// effectively a better, asynchronous version of system()
// redirects and captures the child process i/o
struct child_pty_info* AppState_ExecProcessPTY(AppState* as, char* execPath, char* args[]) {
	
	int master, slave; // pty
	
	
	errno = 0;
	if(openpty(&master, &slave, NULL, NULL, NULL) < 0) {
		fprintf(stderr, "Error opening new pty for '%s' [errno=%d]\n", execPath, errno);
		return NULL;
	}
	
	errno = 0;
	
	int childPID = fork();
	if(childPID == -1) {
		
		fprintf(stderr, "failed to fork trying to execute '%s'\n", execPath);
		perror(strerror(errno));
		return NULL;
	}
	else if(childPID == 0) { // child process
		
		setsid();
		
		// redirect standard fd's to the pipe fd's 
		if(dup2(slave, fileno(stdin)) == -1) {
			printf("failed 1\n");
			exit(errno);
		}
		if(dup2(slave, fileno(stdout)) == -1) {
			printf("failed 2\n");
			exit(errno);
		}
		if(dup2(slave, fileno(stderr)) == -1) {
			printf("failed 3\n");
			exit(errno);
		}
		
		if(ioctl(slave, TIOCSCTTY, NULL) < 0) {
			fprintf(stderr, "ioctl TIOCSCTTY failed: %s, %d\n", execPath, errno);
		}
		
		// close original fd's
		close(master);
		close(slave);
		
		// die when the parent does (linux only)
		prctl(PR_SET_PDEATHSIG, SIGHUP);
		
		// swap for the desired program
		execvp(execPath, args); // never returns if successful
		 
		fprintf(stderr, "failed to execute '%s'\n", execPath);
		exit(1); // kill the forked process 
	}
	else { // parent process
		
		// close the child-end of the pipes
		struct child_pty_info* cpi;
		cpi = calloc(1, sizeof(*cpi));
		cpi->pid = childPID;
		cpi->pty = master;
		
		// set to non-blocking
		fcntl(master, F_SETFL, fcntl(master, F_GETFL) | FNDELAY | O_NONBLOCK);
		
		close(slave);
		
// 		tcsetattr(STDIN_FILENO, TCSANOW, &master);
// 		fcntl(master, F_SETFL, FNDELAY);
		
// 		int status;
		// returns 0 if nothing happened, -1 on error
// 		pid = waitpid(childPID, &status, WNOHANG);
		
		return cpi;
	}
	
	return NULL; // shouldn't reach here
	
}


// effectively a better, asynchronous version of system()
// redirects and captures the child process i/o
struct child_process_info* AppState_ExecProcessPipe(char* execPath, char* args[]) {
	
	int master, slave; //pty
	int in[2]; // io pipes
	int out[2];
	int err[2];
	
	const int RE = 0;
	const int WR = 1;
	
	// 0 = read, 1 = write
	
	if(pipe(in) < 0) {
		return NULL;
	}
	if(pipe(out) < 0) {
		close(in[0]);
		close(in[1]);
		return NULL;
	}
	if(pipe(err) < 0) {
		close(in[0]);
		close(in[1]);
		close(out[0]);
		close(out[1]);
		return NULL;
	}
	
	errno = 0;
	if(openpty(&master, &slave, NULL, NULL, NULL) < 0) {
		close(in[0]);
		close(in[1]);
		close(out[0]);
		close(out[1]);
		fprintf(stderr, "Error opening new pty for '%s' [errno=%d]\n", execPath, errno);
		return NULL;
	}
	
	errno = 0;
	
	int childPID = fork();
	if(childPID == -1) {
		
		fprintf(stderr, "failed to fork trying to execute '%s'\n", execPath);
		perror(strerror(errno));
		return NULL;
	}
	else if(childPID == 0) { // child process
		
		// redirect standard fd's to the pipe fd's 
		if(dup2(in[RE], fileno(stdin)) == -1) {
			printf("failed 1\n");
			exit(errno);
		}
		if(dup2(out[WR], fileno(stdout)) == -1) {
			printf("failed 2\n");
			exit(errno);
		}
		if(dup2(err[WR], fileno(stderr)) == -1) {
			printf("failed 3\n");
			exit(errno);
		}
		
		// close original fd's used by the parent
		close(in[0]);
		close(in[1]);
		close(out[0]);
		close(out[1]);
		close(err[0]);
		close(err[1]);
		
		close(master);
		close(slave);
		
		// die when the parent does (linux only)
		prctl(PR_SET_PDEATHSIG, SIGHUP);
		
		// swap for the desired program
		execvp(execPath, args); // never returns if successful
		
		fprintf(stderr, "failed to execute '%s'\n", execPath);
		exit(1); // kill the forked process 
	}
	else { // parent process
		
		// close the child-end of the pipes
		struct child_process_info* cpi;
		cpi = calloc(1, sizeof(*cpi));
		
		cpi->child_stdin = in[WR];
		cpi->child_stdout = out[RE];
		cpi->child_stderr = err[RE];
		cpi->f_stdin = fdopen(cpi->child_stdin, "wb");
		cpi->f_stdout = fdopen(cpi->child_stdout, "rb");
		cpi->f_stderr = fdopen(cpi->child_stderr, "rb");
		
		// set to non-blocking
		fcntl(cpi->child_stdout, F_SETFL, fcntl(cpi->child_stdout, F_GETFL) | O_NONBLOCK);
		fcntl(cpi->child_stderr, F_SETFL, fcntl(cpi->child_stderr, F_GETFL) | O_NONBLOCK);
		
		close(in[0]);
		close(out[1]); 
		close(err[1]); 
		
		close(slave);
		
		cpi->pid = childPID;
		
// 		int status;
		// returns 0 if nothing happened, -1 on error
// 		pid = waitpid(childPID, &status, WNOHANG);
		
		return cpi;
	}
	
	return NULL; // shouldn't reach here
}


void execProcessPipe_buffer(char** args, char** buffer_out, size_t* size_out/*,int* code_out*/) {
	char** argsv[] = {args, NULL};

	execProcessPipe_bufferv(argsv, buffer_out, size_out);
}


char* execProcessPipe_charpp(char** args, char*** charpp_out, size_t* n_out/*,int* code_out*/) {
	char** argsv[] = {args, NULL};

	return execProcessPipe_charppv(argsv, charpp_out, n_out);
}


void execProcessPipe_bufferv(char*** args, char** buffer_out, size_t* size_out/*,int** code_out*/) {
	struct child_process_info* cc;
	int bufferLength = 1024;

	size_t max_contents = 4 * bufferLength * sizeof(char);
	char* contents = malloc(max_contents);
	size_t offset = 0, n_read;
	char buffer[bufferLength];

	char** filepaths;
	size_t n_filepaths = 0;

	int i = 0;
	while(args[i]) {
		// printf("using search arg: %s\n", args[2]);
		cc = AppState_ExecProcessPipe(
			args[i][0],
			args[i]
		);

		while(!feof(cc->f_stdout)) {
			size_t n_read = fread(buffer, 1, bufferLength, cc->f_stdout);
			if(n_read && (offset + n_read) >= max_contents) {
				max_contents *= 2;
				contents = realloc(contents, max_contents);
			}

			// printf("copy at [%ld]: [[%s]]\n", offset, buffer);
			memcpy((char*)((size_t)contents+offset), buffer, n_read);
			offset += n_read;
		}

		fclose(cc->f_stdin);
		fclose(cc->f_stdout);
		fclose(cc->f_stderr);
		
		// clean up the zombie process
		int status;
		waitpid(cc->pid, &status, 0); 
		
		i++;
	}
	contents[offset] = '\0';
	*buffer_out = contents;
}


char* execProcessPipe_charppv(char*** args, char*** charpp_out, size_t* n_out/*,int** code_out*/) {
	char* contents;

	execProcessPipe_bufferv(args, &contents,  n_out);

	*charpp_out = strsplit_inplace(contents, '\n', n_out);
	
	return contents;
}






/*
char buffer[1024];


errno = 0;
// int len = read(buffer, 5, cc->child_stdout);
int len = read(cc->child_stdout, buffer, 1023);
if(len == -1 && errno != EWOULDBLOCK) {
	printf("1: %d %s\n", errno,  strerror(errno));
}

if(len > 0) {
	buffer[len] = 0;
	printf("from bash[%d]: '%.*s'\n", len, len, buffer);
}

errno = 0;
len = read(cc->child_stderr, buffer, 1023);
if(len == -1 && errno != EWOULDBLOCK) {
	printf("2: %d %s\n", errno, strerror(errno));
}
if(len > 0) {
	buffer[len] = 0;
	printf("from bash[%d]: '%.*s'\n", len, len, buffer);
}
*/




void AppState_UpdateSettings(AppState* as, GlobalSettings* gs) {
	
	as->globalSettings = *gs;
	
//	GUIMainControl_UpdateSettings(as->mc, gs);
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
	
	
		PERF(now = getCurrentTime());
		GUIManager_Reap(as->gui);
		PERF(printf("GUIManager_Reap: %fus\n", timeSince(now) * 1000000));
	}
}


