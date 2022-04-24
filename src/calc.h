#ifndef __imcalc__calc_h__
#define __imcalc__calc_h__

#include <mpfr.h>

#include "ui/gui.h"
#include "sti/vec.h"



typedef struct calc_history {
	mpfr_t value;
	char* text;
} calc_history_t;


typedef struct calculator {
	
	
	VEC(calc_history_t*) history;
	
	int op;
	GUIString acc;
	
} calculator_t;




struct GUIManager;
typedef struct GUIManager GUIManager;





void calc_init(calculator_t* c);
void draw_gui_root(GUIManager* gm, calculator_t* c);







#endif // __imcalc__calc_h__
