
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>

#include <signal.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <pty.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <dirent.h>

#include <sys/stat.h>
#include <sys/types.h>

// link with -lutil


char* sources[] = {
	"src/app.c",
	"src/c_json/json.c",
	"src/c3dlas/c3dlas.c",
	"src/clipboard.c",
	"src/dumpImage.c",
	"src/fbo.c",
	"src/fcfg.c",
	"src/font.c",
	"src/input.c",
	"src/json_gl.c",
	"src/main.c",
	"src/mdi.c",
	"src/pass.c",
	"src/pcBuffer.c",
	"src/settings.c",
	"src/shader.c",
	"src/sti/sti.c",
	"src/texture.c",
	"src/textureAtlas.c",
	"src/ui/commands.c",
	"src/ui/gui.c",
	"src/ui/gui_settings.c",
	"src/ui/guiManager.c",
	"src/utilities.c",
	"src/window.c",
};



char* cflags[] = {
	"-I/usr/include/freetype2", 
	"-std=gnu11", 
	"-ggdb", 
	"-DLINUX",
	"-DSTI_C3DLAS_NO_CONFLICT",
	"-DEACSMB_USE_SIMD",
	"-DEACSMB_HAVE_SSE4",
	"-DEACSMB_HAVE_AVX", 
	"-march=native",
	"-mtune=native", 
	"-fno-math-errno", 
	"-fexcess-precision=fast", 
	"-fno-signed-zeros",
	"-fno-trapping-math", 
	"-fassociative-math", 
	"-ffinite-math-only", 
	"-fno-rounding-math", 
	"-fno-signaling-nans", 
	"-include config.h", 
	"-include signal.h", 
	"-pthread", 
	"-Wall", 
	"-Werror", 
	"-Wextra", 
	"-Wno-unused-result", 
	"-Wno-unused-variable", 
	"-Wno-unused-but-set-variable", 
	"-Wno-unused-function", 
	"-Wno-unused-label", 
	"-Wno-unused-parameter",
	"-Wno-pointer-sign", 
	"-Wno-missing-braces", 
	"-Wno-maybe-uninitialized", 
	"-Wno-implicit-fallthrough", 
	"-Wno-sign-compare", 
	"-Wno-char-subscripts", 
	"-Wno-int-conversion", 
	"-Wno-int-to-pointer-cast", 
	"-Wno-unknown-pragmas",
	"-Wno-sequence-point",
	"-Wno-switch",
	"-Wno-parentheses",
	"-Wno-comment",
	"-Wno-strict-aliasing",
	"-Werror=implicit-function-declaration",
	"-Werror=uninitialized",
	"-Werror=return-type",
};



struct child_process_info {
	int pid;
	int child_stdin;
	int child_stdout;
	int child_stderr;
	FILE* f_stdin;
	FILE* f_stdout;
	FILE* f_stderr;
};

typedef struct rglob_entry {
	char type;
	char* full_path;
	char* file_name;
//	char* dir_name;
} rglob_entry;

typedef struct rglob {
	char* pattern;

	int len;
	int alloc;
	rglob_entry* entries;
	
} rglob;


#define PP_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, N, ...) N
#define PP_RSEQ_N() 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
#define PP_NARG_(...) PP_ARG_N(__VA_ARGS__)
#define PP_NARG(...)  PP_NARG_(__VA_ARGS__, PP_RSEQ_N())

struct child_process_info* exec_process_pipe(char* exec_path, char* args[]);
int mkdirp(char* path, mode_t mode);
char* resolve_path(char* in);
#define path_join(...) path_join_(PP_NARG(__VA_ARGS__), __VA_ARGS__)
char* path_join_(size_t nargs, ...);

#define FSU_EXCLUDE_HIDDEN     (1<<0)
#define FSU_NO_FOLLOW_SYMLINKS (1<<1)
#define FSU_INCLUDE_DIRS       (1<<2)
#define FSU_EXCLUDE_FILES      (1<<3)

// return 0 to continue, nonzero to stop all directory scanning
typedef int (*readDirCallbackFn)(char* /*fullPath*/, char* /*fileName*/, unsigned char /*type*/, void* /*data*/);
// returns negative on error, nonzero if scanning was halted by the callback
int recurse_dirs(
	char* path, 
	readDirCallbackFn fn, 
	void* data, 
	int depth, 
	unsigned int flags
);



int main(int argc, char* argv[]) {
	
	mkdirp("build", 0755);
	
	
//	printf("%d: %s\n", err, strerror(errno));
	return 0;
}



int rglob_fn(char* full_path, char* file_name, unsigned char type, void* _results) {
	rglob* res = (rglob*)_results;
	
	if(0 == fnmatch(res->pattern, file_name, 0)) {
		if(res->len >= res->alloc) {
			res->alloc *= 2;
			res->entries = realloc(res->entries, sizeof(*res->entries) * res->alloc);
		}
		
		res->entries[res->len].type = type;
		res->entries[res->len].full_path = strdup(full_path);
		res->entries[res->len].file_name = strdup(file_name);
		res->len++;
	}
	
	return 0;
}

void recursive_glob(char* base_path, char* pattern, int flags, rglob* results) {
	
	// to pass into recurse_dirs()
	results->pattern = pattern;
	results->len = 0;
	results->alloc = 32;
	results->entries = malloc(sizeof(*results->entries) * results->alloc);
	
	recurse_dirs(base_path, rglob_fn, results, -1, flags);
}


// does not handle escaped slashes
int mkdirp(char* path, mode_t mode) {
	
	char* clean_path = strdup(path);
	
	// inch along the path creating each directory in line
	for(char* p = clean_path; *p; p++) {
		if(*p == '/') {
			*p = 0;
			
			if(mkdir(clean_path, mode)) {
				if(errno != EEXIST) goto FAIL;
			}
			
			*p = '/';
		}
	}
	
	// mop up the last dir
	if(mkdir(clean_path, mode)) {
		if(errno != EEXIST) goto FAIL;
	}
	
	free(clean_path);
	return 0;
	
FAIL:
	free(clean_path);
	return -1;
}


// works like realpath(), except also handles ~/
char* resolve_path(char* in) {
	int tmp_was_malloced = 0;
	char* out, *tmp;
	
	if(!in) return NULL;
	
	// skip leading whitespace
	while(isspace(*in)) in++;
	
	// handle home dir shorthand
	if(in[0] == '~') {
		char* home = getenv("HOME");
		
		tmp_was_malloced = 1;
		tmp = malloc(sizeof(*tmp) * (strlen(home) + strlen(in) + 2));
		
		strcpy(tmp, home);
		strcat(tmp, "/"); // just in case
		strcat(tmp, in + 1);
	}
	else tmp = in;
	
	out = realpath(tmp, NULL);
	
	if(tmp_was_malloced) free(tmp);
	
	return out;
}


char* path_join_(size_t nargs, ...) {
	size_t total = 0;
	char* out, *end;
	size_t j_len;
	char* joiner = "/";
	int escape;

	if(nargs == 0) return NULL;

	// calculate total buffer length
	va_list va;
	va_start(va, nargs);

	for(size_t i = 0; i < nargs; i++) {
		char* s = va_arg(va, char*);
		if(s) total += strlen(s);
	}

	va_end(va);

	j_len = strlen(joiner);
	total += j_len * (nargs - 1);

	out = malloc((total + 1) * sizeof(char*));
	end = out;

	va_start(va, nargs);

	for(size_t i = 0; i < nargs; i++) {
		char* s = va_arg(va, char*);
		size_t l = strlen(s);
		
		if(s) {
			if(l > 1) {
				escape = s[l-2] == '\\' ? 1 : 0;
			}

			if(i > 0 && (s[0] == joiner[0])) {
				s++;
				l--;
			}

			if(i > 0 && i != nargs-1 && !escape && (s[l-1] == joiner[0])) {
				l--;
			}

			if(i > 0) {
				strcpy(end, joiner);
				end += j_len;
			}

			// should be strncpy, but GCC is so fucking stupid that it
			//   has a warning about using strncpy to do exactly what 
			//   strncpy does if you read the fucking man page.
			// fortunately, we are already terminating our strings
			//   manually so memcpy is a drop-in replacement here.
			memcpy(end, s, l);
			end += l;
		}
	}

	va_end(va);

	*end = 0;

	return out;
}


// returns negative on error, nonzero if scanning was halted by the callback
int recurse_dirs(
	char* path, 
	readDirCallbackFn fn, 
	void* data, 
	int depth, 
	unsigned int flags
) {
	
	DIR* derp;
	struct dirent* result;
	int stop = 0;
	
	if(fn == NULL) {
		fprintf(stderr, "Error: readAllDir called with null function pointer.\n");
		return -1;
	}
	
	derp = opendir(path);
	if(derp == NULL) {
		fprintf(stderr, "Error opening directory '%s': %s\n", path, strerror(errno));
		return -1;
	}
	
	
	while((result = readdir(derp)) && !stop) {
		char* n = result->d_name;
		unsigned char type = DT_UNKNOWN;
		char* fullPath;
		
		// skip self and parent dir entries
		if(n[0] == '.') {
			if(n[1] == '.' && n[2] == 0) continue;
			if(n[1] == 0) continue;
			
			if(flags & FSU_EXCLUDE_HIDDEN) continue;
		}

#ifdef _DIRENT_HAVE_D_TYPE
		type = result->d_type; // the way life should be
#else
		// do some slow extra bullshit to get the type
		fullPath = path_join(path, n);
		
		struct stat upgrade_your_fs;
		
		lstat(fullPath, &upgrade_your_fs);
		
		if(S_ISREG(upgrade_your_fs.st_mode)) type = DT_REG;
		else if(S_ISDIR(upgrade_your_fs.st_mode)) type = DT_DIR;
		else if(S_ISLNK(upgrade_your_fs.st_mode)) type = DT_LNK;
#endif
		
		if(flags & FSU_NO_FOLLOW_SYMLINKS && type == DT_LNK) {
			continue;
		}
		
#ifdef _DIRENT_HAVE_D_TYPE
		fullPath = path_join(path, n);
#endif
		
		if(type == DT_DIR) {
			if(flags & FSU_INCLUDE_DIRS) {
				stop = fn(fullPath, n, type, data);
			}
			if(depth != 0) {
				stop |= recurse_dirs(fullPath, fn, data, depth - 1, flags);
			}
		}
		else if(type == DT_REG) {
			if(!(flags & FSU_EXCLUDE_FILES)) {
				stop = fn(fullPath, n, type, data);
			}
		}
		
		free(fullPath);
	}
	
	
	closedir(derp);
	
	return stop;
}




// effectively a better, asynchronous version of system()
// redirects and captures the child process i/o
struct child_process_info* exec_process_pipe(char* exec_path, char* args[]) {
	
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
		fprintf(stderr, "Error opening new pty for '%s' [errno=%d]\n", exec_path, errno);
		return NULL;
	}
	
	errno = 0;
	
	int child_pid = fork();
	if(child_pid == -1) {
		
		fprintf(stderr, "failed to fork trying to execute '%s'\n", exec_path);
		perror(strerror(errno));
		return NULL;
	}
	else if(child_pid == 0) { // child process
		
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
		execvp(exec_path, args); // never returns if successful
		
		fprintf(stderr, "failed to execute '%s'\n", exec_path);
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
		
		cpi->pid = child_pid;
		
// 		int status;
		// returns 0 if nothing happened, -1 on error
// 		pid = waitpid(childPID, &status, WNOHANG);
		
		return cpi;
	}
	
	return NULL; // shouldn't reach here
}






