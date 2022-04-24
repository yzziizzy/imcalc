



#include <ctype.h>

#include "calc.h"
#include "ui/gui.h"
#include "ui/gui_internal.h"









static int num_edit_filter(GUIString* s, GUIKeyEvent* e, int pos, void* user_data) {
	char c = e->character;
	
	if(isdigit(c)) return 1;
	
	if(c == '.') {
		return NULL == strchr(s->data, '.');
	}
	
	if(c == '-' && pos == 0 && s->data[0] != '-') return 1;
	
	if(!isprint(c)) return 1;
	
	return 0;
}








void draw_gui_root(GUIManager* gm, calculator_t* c) {


	static double acc = 0;
	static double prev = 0;
	static long op = 0;
	static long decimal = 0;
	static long pdecimal = 0;
	
	static int opts_active = 0;
	static int opt_1 = 0;
	static int selOpt = 1;
	static void* rad = 0;
	
	static long num = 1337;
	static GUIString str = {0};
	
	if(str.alloc == 0) {
		str.data = malloc(64);
		strcpy(str.data, "Foobar");
		str.alloc = 64;
		str.len = strlen(str.data);
	}

	static char* opts[] = {
		"Red",
		"Green",
		"Blue",
		"Yellow",
		NULL,
	};


	if(GUI_ToggleButton_(gm, 680001, (Vector2){10,10}, (Vector2){100,20}, "Options", &opts_active)) {
	
		GUI_Checkbox_(gm, 680002, (Vector2){100,110}, "Option 1", &opt_1);
		
		GUI_RadioButton_(gm, 680010, (Vector2){100,140}, "Radio 1", &rad, 1);
		GUI_RadioButton_(gm, 680011, (Vector2){100,160}, "Radio 2", &rad, 0);
		GUI_RadioButton_(gm, 680012, (Vector2){100,180}, "Radio 3", &rad, 0);
		GUI_RadioButton_(gm, 680013, (Vector2){100,200}, "Radio 4", &rad, 0);
		
		
		GUI_Edit_(gm, 680019, (Vector2){200,20}, (Vector2){200, 30}, &str);
		
		GUI_IntEdit_(gm, 680020, (Vector2){200,200}, (Vector2){200, 30}, &num);

		GUI_SelectBox_(gm, 680030, (Vector2){200,100}, (Vector2){200, 30}, opts, &selOpt);
	}
	else {
	
		char* num1 = sprintfdup("%f", prev);
		GUI_TextLine_(gm, num1, strlen(num1), (Vector2){270,120}, "Arial", 15, &((Color4){.9,.9,.9,1}));
		free(num1);
	
		
		GUI_Edit_SetFilter(680400, num_edit_filter, c);		
			
		gm->curZ = 1.0;
		
		float sz = 40;
		float pad = 5;
		float offx = 20;
		float offy = 50;
		
		#define OFF(x, y) (Vector2){offx+sz*x+pad*x,offy+sz*y+pad*y}
		
		#define NUM(x) \
			GUI_Edit_Trigger(680400, &c->acc, '0' + x);
		
		if(GUI_Button_(gm, 690007, OFF(0,0), (Vector2){sz,sz}, "7")) {
			NUM(7);
		}
		if(GUI_Button_(gm, 690008, OFF(1,0), (Vector2){sz,sz}, "8")) {
			NUM(8);
		}
		if(GUI_Button_(gm, 690009, OFF(2,0), (Vector2){sz,sz}, "9")) {
			NUM(9);
		}
		
		if(GUI_Button_(gm, 690004, OFF(0,1), (Vector2){sz,sz}, "4")) {
			NUM(4);
		}
		if(GUI_Button_(gm, 690005, OFF(1,1), (Vector2){sz,sz}, "5")) {
			NUM(5);
		}
		if(GUI_Button_(gm, 690006, OFF(2,1), (Vector2){sz,sz}, "6")) {
			NUM(6);
		}
		
		if(GUI_Button_(gm, 690001, OFF(0,2), (Vector2){sz,sz}, "1")) {
			NUM(1);
		}
		if(GUI_Button_(gm, 690002, OFF(1,2), (Vector2){sz,sz}, "2")) {
			NUM(2);
		}
		if(GUI_Button_(gm, 690003, OFF(2,2), (Vector2){sz,sz}, "3")) {
			NUM(3);
		}
		
		if(GUI_Button_(gm, 690000, OFF(0,3), (Vector2){sz,sz}, "0")) {
			NUM(0);
		}
		if(GUI_Button_(gm, 690010, OFF(1,3), (Vector2){sz,sz}, ".")) {
//			decimal = decimal ? decimal : 1;
			if(NULL == strchr(c->acc.data, '.')) {
				GUI_Edit_Trigger(680400, &c->acc, '.');
			}
		}
		if(GUI_Button_(gm, 690020, OFF(2,3), (Vector2){sz,sz}, "(-)")) {
			acc *= -1;
			if(c->acc.data[0] == '-') {
				memmove(c->acc.data, c->acc.data + 1, c->acc.len);
				c->acc.len--;
			}
			else {
				memmove(c->acc.data + 1, c->acc.data, c->acc.len + 1);
				c->acc.data[0] = '-';
				c->acc.len++;	
			}
			
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
		
		GUI_Edit_(gm, 680400, (Vector2){270,150}, (Vector2){200, 30}, &c->acc);
	
	}
	
	
	

}





void calc_init(calculator_t* c) {

	c->acc.len = 0;
	c->acc.alloc = 256;
	c->acc.data = calloc(1, sizeof(*c->acc.data) * c->acc.alloc); 
}








