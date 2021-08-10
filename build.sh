#!/bin/bash


#	src/app.c \
#	src/fbo.c \
#	src/fcfg.c \
#	src/font.c \
#	src/main.c \
#	src/window.c \
#	src/input.c \

SOURCES=$(cat << EOF
	src/app.c
	src/c_json/json.c
	src/c3dlas/c3dlas.c
	src/clipboard.c
	src/dumpImage.c
	src/fbo.c
	src/fcfg.c
	src/font.c
	src/input.c
	src/json_gl.c
	src/main.c
	src/mdi.c
	src/pass.c
	src/pcBuffer.c
	src/settings.c
	src/shader.c
	src/sti/sti.c
	src/texture.c
	src/textureAtlas.c
	src/ui/commands.c
	src/ui/gui.c
	src/ui/gui_settings.c
	src/ui/guiManager.c
	src/utilities.c
	src/window.c
EOF
)


LDADD="  \
	-ldl -lutil -lm -lGL -lGLEW -lGLU \
	-lX11 -lXfixes -lpng -lpcre2-8 \
"

# -ffast-math but without reciprocal approximations 
CFLAGS=$(cat << EOF
	-I/usr/include/freetype2 -std=gnu11 -g -DLINUX 
	-DSTI_C3DLAS_NO_CONFLICT 
	-DEACSMB_USE_SIMD 
	-DEACSMB_HAVE_SSE4 
	-DEACSMB_HAVE_AVX 
	-march=native -mtune=native 
	-fno-math-errno 
	-fexcess-precision=fast 
	-fno-signed-zeros -fno-trapping-math -fassociative-math 
	-ffinite-math-only -fno-rounding-math 
	-fno-signaling-nans 
	-include config.h 
	-include signal.h 
	-pthread 
	-Wall 
	-Werror 
	-Wextra 
	-Wno-unused-result 
	-Wno-unused-variable 
	-Wno-unused-but-set-variable 
	-Wno-unused-function 
	-Wno-unused-label 
	-Wno-unused-parameter 
	-Wno-pointer-sign 
	-Wno-missing-braces 
	-Wno-maybe-uninitialized 
	-Wno-implicit-fallthrough 
	-Wno-sign-compare 
	-Wno-char-subscripts 
	-Wno-int-conversion 
	-Wno-int-to-pointer-cast 
	-Wno-unknown-pragmas 
	-Wno-sequence-point 
	-Wno-switch 
	-Wno-parentheses 
	-Wno-comment 
	-Wno-strict-aliasing 
	-Werror=implicit-function-declaration 
	-Werror=uninitialized 
	-Werror=return-type 
EOF
)

objlist=""

mkdir -p build

declare -A mtime_lookup
declare -A realpath_lookup


function checkdeps() {
	d=`dirname $1`
	
	sp=`realpath $1`
	op=`realpath $2`
	dp=`realpath "./build/$1.d"`
	
	if [[ ! -d build/$d ]]; then
		mkdir -p build/$d
	fi
	
	if [[ ! -f $2 ]]; then
		echo "Building $2..."
		gcc -c -o $2 $1 $CFLAGS $LDADD
		
		return
	fi
	
	
	stime=${mtime_lookup[$sp]}
	o=$2 #"build/$(echo $1 | sed s/\.c$/\.o/)"
	otime=${mtime_lookup[$op]}
	#echo "stime($sp)=$stime, otime($op)=$otime"
	if [[ $stime -gt  $otime ]]; then
		echo "Rebuilding $2..."
		gcc -c -o $2 $1 $CFLAGS $LDADD
		
		return
	fi
	
	dtime=${mtime_lookup[$dp]}
	if [[ ! -f "build/$1.d" ]] || [[ $stime -gt  $dtime ]]; then
		echo "  Generating dependency list...  $stime -gt  $dtime "
		gcc -MM -MG -MT $1 -MF "build/$1.d" $1 $CFLAGS $LDADD
	fi

	echo "Checking deps: $sp"
	list=`cat build/$1.d | tr '\\n' '\\\\' | sed s/\\\\\\\\//g | sed s/^[^:]*://g`
	for dep in $list; do
		
		
		depp=${realpath_lookup[$dep]}
		if [[ -z $depp ]]; then
			depp=`realpath $dep`
			realpath_lookup[$dep]=$depp
		fi
		
		if [[ -z ${mtime_lookup[$depp]} ]]; then
			echo "    missing dep mtime $depp"
			mtime_lookup[$depp]=`stat -c "%Y" $dep 2> /dev/null`
		fi
		
		dtime=${mtime_lookup[$depp]}
	#	echo "$dep $dtime -gt  $otime "
		if [[ $dtime -gt  $otime ]]; then
			echo "Rebuilding $2..."
			gcc -c -o $2 $1 $CFLAGS $LDADD
			
			break
		fi
	done
	
}

function compile() {
	o="./build/$(echo $1 | sed s/\.c$/\.o/)"
	objlist="$objlist $o"
	
	checkdeps $1 $o
}


old_ifs=$IFS
IFS="
"

llist=`find -L ./ -name "*.[chod]" -printf "%p:%T@\n"`
for pair in $llist; do
	
	f=`realpath ${pair%:*}`;
	d=${pair#*:};
	mtime_lookup[$f]=${d%.*}
	realpath_lookup[$d]=${d%.*}
	#echo "$f = $d"
done


for key in "${!mtime_lookup[@]}"; do echo "$key => ${mtime_lookup[$key]}"; done

IFS=$old_ifs
for f in $SOURCES; do
	#echo "compiling $f"
	compile $f
	
done

echo "Linking executable..."
gcc -o imcalc $objlist $CFLAGS $LDADD




