
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <limits.h>
#include <pthread.h>
#include <sys/sysinfo.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "c3dlas/c3dlas.h"


#include "utilities.h"
#include "shader.h"
#include "window.h"
#include "app.h"
#include "clipboard.h"





static XStuff xs;
static AppState app;
static InputState input;





int main(int argc, char* argv[]) {
	int first = 1;
	int configStatus = 0;
	char* wd;
	
	
	GlobalSettings_LoadDefaults(&app.globalSettings);
	app.globalSettings.Theme = &app.themeSettings;
	app.globalSettings.GUI_GlobalSettings = calloc(1, sizeof(*app.globalSettings.GUI_GlobalSettings));
	GUI_GlobalSettings_LoadDefaults(app.globalSettings.GUI_GlobalSettings);
	
	
	GlobalSettings_ReadAllJSONAt(&app.globalSettings, "/etc/gpuedit/");
	
	char* homedir = getenv("HOME");
	GlobalSettings_ReadDefaultsAt(&app.globalSettings, homedir);

	char* curdir = getenv("PWD");
	GlobalSettings_ReadDefaultsAt(&app.globalSettings, curdir);

	ThemeSettings_LoadDefaults(&app.themeSettings);
	char* tmp = path_join(homedir, "/.gpuedit/themes/", app.globalSettings.Theme_path);
	GlobalSettings_LoadFromFile(&app.globalSettings, tmp);
	free(tmp);

	
	// init some path info. 
// 	wd = getcwd(NULL, 0);
// 	app.dataDir = path_join(wd, "data");
	
// 	free(wd);
	
	
	input.doubleClickTime = 0.200;
	input.dragMinDist = 4;
	
	memset(&xs, 0, sizeof(XStuff));
	xs.gs = &app.globalSettings;
	clearInputState(&input);
		
	xs.targetMSAA = 4;
	xs.windowTitle = "imcalc";
	
	// before the window is initialized
	Clipboard_Init();
	
	initXWindow(&xs);
	
	
	initApp(&xs, &app, argc, argv);
	
	
	// initialization loop
	while(1) {
		InputEvent iev;
		processEvents(&xs, &input, &iev, -1);
		
		if(first && xs.ready) {
			initAppGL(&xs, &app);
			first = 0;
			
			appLoop(&xs, &app, &input);
			
			break;
		}
		
		if(app.frameSpan < 1.0/15.0) {
			// shitty estimation based on my machine's heuristics, needs improvement
			float sleeptime = (((1.0/15.0) * 1000000) - (app.frameSpan * 1000000)) * 1.7;
			//printf("sleeptime: %f\n", sleeptime / 1000000);
			//sleeptime = 1000;
			if(sleeptime > 0) usleep(sleeptime); // problem... something is wrong in the math
		}
// 		sleep(1);
	}
	
	
	
	
	return 0;
}







 
 
