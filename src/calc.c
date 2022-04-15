





#include "calc.h"
#include "ui/gui.h"
#include "ui/gui_internal.h"
















void draw_gui_root(GUIManager* gm) {


	static double acc = 0;
	static double prev = 0;
	static long op = 0;
	static long decimal = 0;
	static long pdecimal = 0;
	
	static int opts_active = 0;
	static int opt_1 = 0;
	static void* rad = 0;

	if(GUI_ToggleButton_(gm, 680001, (Vector2){10,10}, (Vector2){100,20}, "Options", &opts_active)) {
	
		GUI_Checkbox_(gm, 680002, (Vector2){100,110}, "Option 1", &opt_1);
		
		GUI_RadioButton_(gm, 680010, (Vector2){100,140}, "Radio 1", &rad, 1);
		GUI_RadioButton_(gm, 680011, (Vector2){100,160}, "Radio 2", &rad, 0);
		GUI_RadioButton_(gm, 680012, (Vector2){100,180}, "Radio 3", &rad, 0);
		GUI_RadioButton_(gm, 680013, (Vector2){100,200}, "Radio 4", &rad, 0);
	

	}
	else {
	
		char* num1 = sprintfdup("%f", prev);
		GUI_TextLine_(gm, num1, strlen(num1), (Vector2){270,120}, "Arial", 15, &((Color4){.9,.9,.9,1}));
		free(num1);
	
		char* num = sprintfdup("%f", acc);
		GUI_TextLine_(gm, num, strlen(num), (Vector2){270,150}, "Arial", 20, &((Color4){1,1,1,1}));
		free(num);
		
		GUI_Box_(gm, (Vector2){260,130}, (Vector2){200,30}, 2, &((Color4){.8,.8,.8,1}));
		
			
		gm->curZ = 1.0;
		
		float sz = 40;
		float pad = 5;
		float offx = 20;
		float offy = 50;
		
		#define OFF(x, y) (Vector2){offx+sz*x+pad*x,offy+sz*y+pad*y}
		
		#define NUM(x) \
			if(decimal > 0) { \
				acc += x / pow(10, decimal); \
				decimal++; \
			} \
			else { \
				acc *= 10; acc += x; \
			}
		
		if(GUI_Button_(gm, 690007, OFF(0,0), (Vector2){sz,sz}, "7")) {
			NUM(7.0);
		}
		if(GUI_Button_(gm, 690008, OFF(1,0), (Vector2){sz,sz}, "8")) {
			NUM(8.0);
		}
		if(GUI_Button_(gm, 690009, OFF(2,0), (Vector2){sz,sz}, "9")) {
			NUM(9.0);
		}
		
		if(GUI_Button_(gm, 690004, OFF(0,1), (Vector2){sz,sz}, "4")) {
			NUM(4.0);
		}
		if(GUI_Button_(gm, 690005, OFF(1,1), (Vector2){sz,sz}, "5")) {
			NUM(5.0);
		}
		if(GUI_Button_(gm, 690006, OFF(2,1), (Vector2){sz,sz}, "6")) {
			NUM(6.0);
		}
		
		if(GUI_Button_(gm, 690001, OFF(0,2), (Vector2){sz,sz}, "1")) {
			NUM(1.0);
		}
		if(GUI_Button_(gm, 690002, OFF(1,2), (Vector2){sz,sz}, "2")) {
			NUM(2.0);
		}
		if(GUI_Button_(gm, 690003, OFF(2,2), (Vector2){sz,sz}, "3")) {
			NUM(3.0);
		}
		
		if(GUI_Button_(gm, 690000, OFF(0,3), (Vector2){sz,sz}, "0")) {
			NUM(0.0);
		}
		if(GUI_Button_(gm, 690010, OFF(1,3), (Vector2){sz,sz}, ".")) {
			decimal = decimal ? decimal : 1;
		}
		if(GUI_Button_(gm, 690020, OFF(2,3), (Vector2){sz,sz}, "(-)")) {
			acc *= -1;
		}
		
		if(GUI_Button_(gm, 690110, OFF(3,0), (Vector2){sz,sz}, "-")) {
			// subtraction operator
			prev = acc;
			pdecimal = decimal;
			decimal = 0;
			acc = 0;
			op = '-';
		}
		if(GUI_Button_(gm, 690120, OFF(3,1), (Vector2){sz,sz}, "+")) {
			// addition operator
			prev = acc;
			pdecimal = decimal;
			decimal = 0;
			acc = 0;
			op = '+';
		}
		if(GUI_Button_(gm, 690130, OFF(3,2), (Vector2){sz,sz}, "*")) {
			// multiplication operator
			prev = acc;
			pdecimal = decimal;
			decimal = 0;
			acc = 0;
			op = '*';
		}
		if(GUI_Button_(gm, 690140, OFF(3,3), (Vector2){sz,sz}, "/")) {
			// division operator
			prev = acc;
			pdecimal = decimal;
			decimal = 0;
			acc = 0;
			op = '/';
		}
		
		if(GUI_Button_(gm, 690150, OFF(3,4), (Vector2){sz,sz}, "=")) {
			// equals
			switch(op) {
				case '+': acc += prev; break;
				case '-': acc = prev - acc; break;
				case '*': acc *= prev; break;
				case '/': acc = prev / acc; break;
			
				default:
					goto NOTHIN;
			}
			
			prev = acc;
			pdecimal = decimal;
			decimal = 0;
			acc = 0;
			op = 0;
		NOTHIN:
		}
		
		if(GUI_Button_(gm, 690160, OFF(0,4), (Vector2){sz,sz}, "C")) {
			// clear accumulator
			acc = 0;
			decimal = 0;
		}
		if(GUI_Button_(gm, 690170, OFF(1,4), (Vector2){sz,sz}, "CA")) {
			// clear all
			acc = 0;
			prev = 0;
			pdecimal = 0;
			decimal = 0;
			op = 0;
		}
	
	}
	
	
	
	
	



}











