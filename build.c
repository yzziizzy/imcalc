// gcc -lutil build.c -o hacer && ./hacer

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
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>

// link with -lutil


char* sources[] = {
	"src/app.c",
	"src/c_json/json.c",
	"src/c3dlas/c3dlas.c",
	"src/calc.c",
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
	"src/ui/gui.c",
	"src/ui/gui_settings.c",
	"src/ui/guiManager.c",
	"src/ui/imgui.c",
	"src/utilities.c",
	"src/window.c",
	NULL,
};

char* ld_add[] = {
	"-ldl", "-lutil", "-lm", 
	"-lGL", "-lGLEW", "-lGLU",
	"-lX11", "-lXfixes", 
	"-lpng", 
	"-lpcre2-8",
	NULL,
};


// -ffast-math but without reciprocal approximations 
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
	NULL,
};

char** g_gcc_opts_list;
char* g_gcc_opts_flat;

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

typedef struct strlist {
	int len;
	int alloc;
	char** entries;
} strlist;


#define PP_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, _62, _63, N, ...) N
#define PP_RSEQ_N() 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
#define PP_NARG_(...) PP_ARG_N(__VA_ARGS__)
#define PP_NARG(...)  PP_NARG_(__VA_ARGS__, PP_RSEQ_N())

#define check_alloc(x) \
	if((x)->len >= (x)->alloc) { \
		(x)->alloc *= 2; \
		(x)->entries = realloc((x)->entries, (x)->alloc * sizeof(*(x)->entries)); \
	}
void strlist_init(strlist* sl) {
	sl->len = 0;
	sl->alloc = 32;
	sl->entries = malloc(sl->alloc * sizeof(*sl->entries));
}
strlist* strlist_new() {
	strlist* sl = malloc(sizeof(*sl));
	strlist_init(sl);
	return sl;
}
void strlist_push(strlist* sl, char* e) {
	check_alloc(sl);
	sl->entries[sl->len++] = e;
}

typedef unsigned long hash_t;

hash_t strhash(char* str) {
	unsigned long h = 0;
	int c;

	while(c = *str++) {
		h = c + (h << 6) + (h << 16) - h;
	}
	return h;
}
hash_t strnhash(char* str, size_t n) {
	unsigned long h = 0;
	int c;

	while((c = *str++) && n--) {
		h = c + (h << 6) + (h << 16) - h;
	}
		
	return h;
}


typedef struct {
		hash_t hash;
		char* str;
		unsigned int len;
		int refs;
} string_cache_bucket;

typedef struct {
	int fill;
	int alloc;
	string_cache_bucket* buckets;
} string_cache_t;

string_cache_t string_cache;

void string_cache_init(int alloc) {
	string_cache.fill = 0;
	string_cache.alloc = alloc ? alloc : 1024;
	string_cache.buckets = calloc(1, string_cache.alloc * sizeof(*string_cache.buckets));
}

int string_cache_find_bucket(string_cache_t* ht, hash_t hash, char* key) {
	int b = hash % ht->alloc;
	
	for(int i = 0; i < ht->alloc; i++) {
		// empty bucket
		if(ht->buckets[b].str == NULL) return b;
		
		// full bucket
		if(ht->buckets[b].hash == hash) {
			if(0 == strcmp(key, ht->buckets[b].str)) {
				return b;
			}
		}
		
		// probe forward on collisions
		b = (b + 1) % ht->alloc;
	}
	
	// should never reach here
	printf("oops. -1 bucket \n");
	return -1;
}

void string_cache_expand(string_cache_t* ht) {
	int old_alloc = ht->alloc;
	ht->alloc *= 2;
	
	string_cache_bucket* old = ht->buckets;
	ht->buckets = calloc(1, ht->alloc * sizeof(*ht->buckets));
	
	for(int i = 0, f = 0; i < old_alloc && f < ht->fill; i++) { 
		if(!old[i].str) continue;
		
		int b = string_cache_find_bucket(ht, old[i].hash, old[i].str);
		
		ht->buckets[b] = old[i];
		f++;
	}
	
	free(old);
}

char* strcache(char* in) {
	hash_t hash = strhash(in);
	int b = string_cache_find_bucket(&string_cache, hash, in);
	
	if(!string_cache.buckets[b].str) {
		if(string_cache.fill > string_cache.alloc * .80) {
			string_cache_expand(&string_cache);
			
			// the bucket location has changed
			b = string_cache_find_bucket(&string_cache, hash, in);
		}
		
		string_cache.fill++;
		
		string_cache.buckets[b].str = strdup(in);
		string_cache.buckets[b].hash = hash;
		string_cache.buckets[b].refs = 1;
		string_cache.buckets[b].len = strlen(in);
	}
	else {
		string_cache.buckets[b].refs++;
	}
	
	return string_cache.buckets[b].str;
}

char* strncache(char* in, size_t n) {
	hash_t hash = strnhash(in, n);
	int b = string_cache_find_bucket(&string_cache, hash, in);
	
	if(!string_cache.buckets[b].str) {
		if(string_cache.fill > string_cache.alloc * .80) {
			string_cache_expand(&string_cache);
			
			// the bucket location has changed
			b = string_cache_find_bucket(&string_cache, hash, in);
		}
		
		string_cache.fill++;
		
		string_cache.buckets[b].str = strndup(in, n);
		string_cache.buckets[b].hash = hash;
		string_cache.buckets[b].refs = 1;
		string_cache.buckets[b].len = n;
	}
	else {
		string_cache.buckets[b].refs++;
	}
	
	return string_cache.buckets[b].str;
}

void struncache(char* in) {
	hash_t hash = strhash(in);
	int b = string_cache_find_bucket(&string_cache, hash, in);
	
	if(!string_cache.buckets[b].str) {
		// normal string, free it
		free(in);
		return;
	}
	
	string_cache.buckets[b].refs--;
	if(string_cache.buckets[b].refs == 0) {
		// just do nothing for now. deletion is a pain.
	}
}


typedef struct {
		hash_t hash;
		char* key;
		void* value;
} hashbucket;

typedef struct hashtable {
	int fill;
	int alloc;
	hashbucket* buckets;

} hashtable;


void hash_init(hashtable* ht, int alloc) {
	ht->fill = 0;
	ht->alloc = alloc ? alloc : 128;
	ht->buckets = calloc(1, ht->alloc * sizeof(*ht->buckets));
}

hashtable* hash_new(int alloc) {
	hashtable* ht = malloc(sizeof(*ht));
	hash_init(ht, alloc);
	return ht;
}

int hash_find_bucket(hashtable* ht, hash_t hash, char* key) {
	int b = hash % ht->alloc;
	
	for(int i = 0; i < ht->alloc; i++) {
		// empty bucket
		if(ht->buckets[b].key == NULL) return b;
		
		// full bucket
		if(ht->buckets[b].hash == hash) {
			if(0 == strcmp(key, ht->buckets[b].key)) {
				return b;
			}
		}
		
		// probe forward on collisions
		b = (b + 1) % ht->alloc;
	}
	
	// should never reach here
	printf("oops. -1 bucket \n");
	return -1;
}

void hash_expand(hashtable* ht) {
	int old_alloc = ht->alloc;
	ht->alloc *= 2;
	
	hashbucket* old = ht->buckets;
	ht->buckets = calloc(1, ht->alloc * sizeof(*ht->buckets));
	
	for(int i = 0, f = 0; i < old_alloc && f < ht->fill; i++) { 
		if(!old[i].key) continue;
		
		int b = hash_find_bucket(ht, old[i].hash, old[i].key);
		
		ht->buckets[b] = old[i];
		f++;
	}
	
	free(old);
}

void hash_insert(hashtable* ht, char* key, void* value) {
	
	hash_t hash = strhash(key);
	int b = hash_find_bucket(ht, hash, key);
	
	if(!ht->buckets[b].key) {
		if(ht->fill > ht->alloc * .80) {
			hash_expand(ht);
			
			// the bucket location has changed
			b = hash_find_bucket(ht, hash, key);
		}
		
		ht->fill++;
	}
	
	ht->buckets[b].key = strcache(key);
	ht->buckets[b].hash = hash;
	ht->buckets[b].value = value;
}

void* hash_find(hashtable* ht, char* key) {

	hash_t hash = strhash(key);
	int b = hash_find_bucket(ht, hash, key);
	 
	return ht->buckets[b].value;
}

struct child_process_info* exec_process_pipe(char* exec_path, char* args[]);
int mkdirp(char* path, mode_t mode);
char* resolve_path(char* in, time_t* mtime_out);
#define path_join(...) path_join_(PP_NARG(__VA_ARGS__), __VA_ARGS__)
char* path_join_(size_t nargs, ...);
#define concat_lists(...) concat_lists_(PP_NARG(__VA_ARGS__), __VA_ARGS__)
char** concat_lists_(int nargs, ...);
#define strjoin(j, ...) strjoin_(j, PP_NARG(__VA_ARGS__), __VA_ARGS__)
char* strjoin_(char* joiner, size_t nargs, ...);
#define strcatdup(...) strcatdup_(PP_NARG(__VA_ARGS__), __VA_ARGS__)
char* strcatdup_(size_t nargs, ...);
void recursive_glob(char* base_path, char* pattern, int flags, rglob* results);
char* dir_name(char* path);
char* base_name(char* path);
size_t list_len(char** list);
char* join_str_list(char* list[], char* joiner);
char* sprintfdup(char* fmt, ...);
char* read_whole_file(char* path, size_t* srcLen);
static inline char* strskip(char* s, char* skip) {
	return s + strspn(s, skip);
}
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



typedef struct realname_entry {
	char* realname;
	time_t mtime;
} realname_entry;
struct {

	/*
	struct {
		int len;
		int alloc;
		struct {
			char* fake_name;
			realname_entry* entry;
		}* entries;
	} lookup; */

	hashtable names;

	int len;
	int alloc;
	struct realname_entry* entries;
} realname_cache;

void realname_cache_init();
time_t realname_cache_add(char* fake_name, char* real_name);
realname_entry* realname_cache_search_real(char* real_name);
realname_entry* realname_cache_search(char* fake_name);
char* realname_cache_find(char* fake_name);


hashtable mkdir_cache;
strlist compile_cache;

void mkdirp_cached(char* path, mode_t mode) {
	void* there = hash_find(&mkdir_cache, path);
	if(!there) {
		hash_insert(&mkdir_cache, path, NULL);
		mkdirp(path, mode);
	}
}


int compile_cache_execute() {
	int ret = 0;
//	printf("compile cache length %d", compile_cache.len);

	for(int i = 0; i < compile_cache.len; i++) {
//		printf("%s\n", compile_cache.entries[i]);
		ret |= system(compile_cache.entries[i]);
		free(compile_cache.entries[i]);
	}
	
	compile_cache.len = 0;
	return ret;
}

int compile_source(char* src_path, char* obj_path) {
	char* cmd = sprintfdup("gcc -c -o %s %s %s", obj_path, src_path, g_gcc_opts_flat);
//	printf("%s\n", cmd);
	strlist_push(&compile_cache, cmd);
	
	return 0;
}

size_t span_path(char* s) {
	size_t n = 0;
	for(; *s; s++, n++) {
		if(isspace(*s)) break; 
		if(*s == '\\') {
			s++;
			n++;
		}
	} 
	return n;
}

strlist* parse_gcc_dep_file(char* dep_file_path, time_t* newest_mtime) {
	size_t dep_src_len = 0;
	strlist* dep_list;
	time_t newest = 0;
	
	char* dep_src = read_whole_file(dep_file_path, &dep_src_len);
	if(!dep_src) return NULL;
	
	dep_list = strlist_new();
	
	// skip the first filename junk
	char* s = strchr(dep_src, ':');
	s++;
	
	int ret = 0;
	
	// gather dep strings, ignoring line continuations
	while(*s) {
		do {
			s = strskip(s, " \t\r\n");
			if(*s == '\\') {
				 if(s[1] == '\r') s++;
				 if(s[1] == '\n') s++;
			}
		} while(isspace(*s));
		
		int dlen = span_path(s);
		if(dlen == 0) break;
		
		time_t dep_mtime;
		char* dep_fake = strncache(s, dlen);
		char* dep_real = resolve_path(dep_fake, &dep_mtime);
		if(dep_mtime > newest) newest = dep_mtime;
	
		strlist_push(dep_list, dep_real);
		
		struncache(dep_fake);
		struncache(dep_real);
		
		s += dlen;
	}
	

	free(dep_src);
	
	if(newest_mtime) *newest_mtime = newest;
	return dep_list;
}

int gen_deps(char* src_path, char* dep_path, time_t src_mtime, time_t obj_mtime) {
	time_t dep_mtime = 0;
	time_t newest_mtime = 0;
	
	char* real_dep_path = resolve_path(dep_path, &dep_mtime);
	if(dep_mtime < src_mtime) {
		//gcc -MM -MG -MT $1 -MF "build/$1.d" $1 $CFLAGS $LDADD
		printf("  generating deps\n"); 
		char* cmd = sprintfdup("gcc -MM -MG -MT '' -MF %s %s %s", dep_path, src_path, g_gcc_opts_flat);
		system(cmd);
		free(cmd);
	}
	
	strlist* deps = parse_gcc_dep_file(real_dep_path, &newest_mtime);
	
	// free or process deps
	
	return newest_mtime > obj_mtime;
FAIL:
	return 0;
}

void check_source(char* raw_src_path, strlist* objs) {
	time_t src_mtime, obj_mtime = 0, dep_mtime = 0;
	
	char* src_path = resolve_path(raw_src_path, &src_mtime);
	char* src_dir = dir_name(raw_src_path);
	char* base = base_name(src_path);
	
	char* build_dir = path_join("build", src_dir);
	char* obj_path = path_join(build_dir, base);
	
	// cheap and dirty
	size_t olen = strlen(obj_path);
	obj_path[olen-1] = 'o';
	
	
	strlist_push(objs, obj_path);
	
	char* dep_path = strcatdup(build_dir, "/", base, ".d");
	
	mkdirp_cached(build_dir, 0755);
	
	char* real_obj_path = resolve_path(obj_path, &obj_mtime);
	if(obj_mtime < src_mtime) {
		printf("  objtime compile\n");
		compile_source(src_path, real_obj_path);
		return;
	}
	
	
	if(gen_deps(src_path, dep_path, src_mtime, obj_mtime)) {
		printf("  deep dep compile\n");
		compile_source(src_path, real_obj_path);
	}
	
	
	//gcc -c -o $2 $1 $CFLAGS $LDADD
}



int main(int argc, char* argv[]) {
	string_cache_init(2048);
	realname_cache_init();
	strlist_init(&compile_cache);
	hash_init(&mkdir_cache, 128);
	
	char* exe_path = "build/imcalc";
	
	
	mkdirp_cached("build", 0755);
	
	g_gcc_opts_list = concat_lists(ld_add, cflags);
	g_gcc_opts_flat = join_str_list(g_gcc_opts_list, " ");
	
//	rglob src;
	//recursive_glob("src", "*.[ch]", 0, &src);
	
	strlist objs;
	strlist_init(&objs);
	
	for(int i = 0; sources[i]; i++) {
		printf("%i: checking %s\n", i, sources[i]);
		check_source(sources[i], &objs);
	}
	
	if(compile_cache_execute()) {
		printf("Build halted due to errors.\n");
		return 1;
	}
	
	
	char* objects_flat = join_str_list(objs.entries, " ");
//	gcc -o imcalc $objlist $CFLAGS $LDADD
	char* cmd = sprintfdup("gcc -o %s %s %s", exe_path, objects_flat, g_gcc_opts_flat);
	system(cmd);
	
	
//	printf("%d: %s\n", err, strerror(errno));
	return 0;
}







size_t list_len(char** list) {
	size_t total = 0;
	for(; *list; list++) total++;
	return total;
}

char** concat_lists_(int nargs, ...) {
	size_t total = 0;
	char** out, **end;

	if(nargs == 0) return NULL;

	// calculate total list length
	va_list va;
	va_start(va, nargs);

	for(size_t i = 0; i < nargs; i++) {
		char** s = va_arg(va, char**);
		if(s) total += list_len(s);
	}

	va_end(va);

	out = malloc((total + 1) * sizeof(char**));
	end = out;

	va_start(va, nargs);
	
	// concat lists
	for(size_t i = 0; i < nargs; i++) {
		char** s = va_arg(va, char**);
		size_t l = list_len(s);
		
		if(s) {
			memcpy(end, s, l * sizeof(*s));
			end += l;
		}
	}

	va_end(va);

	*end = 0;

	return out;
}


char* join_str_list(char* list[], char* joiner) {
	size_t list_len = 0;
	size_t total = 0;
	size_t jlen = strlen(joiner);
	
	// calculate total length
	for(int i = 0; list[i]; i++) {
		list_len++;
		total += strlen(list[i]);
	}
	
	if(total == 0) return strdup("");
	
	total += (list_len - 1) * jlen;
	char* out = malloc((total + 1) * sizeof(*out));
	
	char* end = out;
	for(int i = 0; list[i]; i++) {
		char* s = list[i];
		size_t l = strlen(s);
		
		if(i > 0) {
			memcpy(end, joiner, jlen);
			end += jlen;
		}
		
		if(s) {
			memcpy(end, s, l);
			end += l;
		}
		
		total += strlen(list[i]);
	}
	
	*end = 0;
	
	return out;
}

// concatenate all argument strings together in a new buffer
char* strcatdup_(size_t nargs, ...) {
	size_t total = 0;
	char* out, *end;
	
	if(nargs == 0) return NULL;
	
	// calculate total buffer len
	va_list va;
	va_start(va, nargs);
	
	for(size_t i = 0; i < nargs; i++) {
		char* s = va_arg(va, char*);
		if(s) total += strlen(s);
	}
	
	va_end(va);
	
	out = malloc((total + 1) * sizeof(char*));
	end = out;
	
	va_start(va, nargs);
	
	for(size_t i = 0; i < nargs; i++) {
		char* s = va_arg(va, char*);
		if(s) {
			strcpy(end, s); // not exactly the ost efficient, but maybe faster than
			end += strlen(s); // a C version. TODO: test the speed
		};
	}
	
	va_end(va);
	
	*end = 0;
	
	return out;
}


// concatenate all argument strings together in a new buffer,
//    with the given joining string between them
char* strjoin_(char* joiner, size_t nargs, ...) {
	size_t total = 0;
	char* out, *end;
	size_t j_len;
	
	if(nargs == 0) return NULL;
	
	// calculate total buffer len
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
		if(s) {
			if(i > 0) {
				strcpy(end, joiner);
				end += j_len;
			}
			
			strcpy(end, s); // not exactly the ost efficient, but maybe faster than
			end += strlen(s); // a C version. TODO: test the speed
		};
	}
	
	va_end(va);
	
	*end = 0;
	
	return out;
}

// allocates a new buffer and calls sprintf with it
// why isn't this a standard function?
char* sprintfdup(char* fmt, ...) {
	va_list va;
	
	va_start(va, fmt);
	size_t n = vsnprintf(NULL, 0, fmt, va);
	char* buf = malloc(n + 1);
	va_end(va);
	
	va_start(va, fmt);
	vsnprintf(buf, n + 1, fmt, va);
	va_end(va);
	
	return buf;
}

char* dir_name(char* path) {
	char* n = strdup(path);
	char* o = dirname(n);
	return strcache(o);
}

char* base_name(char* path) {
	char* n = strdup(path);
	char* o = basename(n);
	return strcache(o);
}


int rglob_fn(char* full_path, char* file_name, unsigned char type, void* _results) {
	rglob* res = (rglob*)_results;
	
	if(0 == fnmatch(res->pattern, file_name, 0)) {
		check_alloc(res);
		
		res->entries[res->len].type = type;
		res->entries[res->len].full_path = strcache(full_path);
		res->entries[res->len].file_name = strcache(file_name);
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


void realname_cache_init() {
	realname_cache.len = 0;
	realname_cache.alloc = 1024;
	realname_cache.entries = malloc(realname_cache.alloc * sizeof(*realname_cache.entries));

	hash_init(&realname_cache.names, 1024);
}

time_t realname_cache_add(char* fake_name, char* real_name) {
	realname_entry* e = hash_find(&realname_cache.names, fake_name);
	if(e) return e->mtime;
	
	e = hash_find(&realname_cache.names, real_name);
	if(!e) {
		struct stat st;
		lstat(real_name, &st);
		
		e = &realname_cache.entries[realname_cache.len];
		e->realname = strcache(real_name);
		e->mtime = st.st_mtim.tv_sec;
		realname_cache.len++;
		
		hash_insert(&realname_cache.names, real_name, e);
	}
	
	hash_insert(&realname_cache.names, fake_name, e);
	
	return e->mtime;
}

realname_entry* realname_cache_search_real(char* real_name) {
	for(int i = 0; i < realname_cache.len; i++) {
		if(0 == strcmp(real_name, realname_cache.entries[i].realname)) {
			return &realname_cache.entries[i];
		}
	}
	
	return NULL;
}
realname_entry* realname_cache_search(char* fake_name) {
	return hash_find(&realname_cache.names, fake_name);
}

char* realname_cache_find(char* fake_name) {
	realname_entry* r = realname_cache_search(fake_name);
	return r ? r->realname : NULL;
}


// works like realpath(), except also handles ~/
char* resolve_path(char* in, time_t* mtime_out) {
	int tmp_was_malloced = 0;
	char* out, *tmp;
	
	if(!in) return NULL;
	
	realname_entry* e = realname_cache_search(in);
	if(e) {
		if(mtime_out) *mtime_out = e->mtime;
		return strcache(e->realname);
	}
	
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
	
	time_t t = 0;
	if(out) {
		// put it in the cache
		t = realname_cache_add(in, out);
	}
	else {
		// temporary
		struct stat st;
		if(!lstat(in, &st))
			t = st.st_mtim.tv_sec;
	}
	
	if(mtime_out) *mtime_out = t;	
	return out ? out : in;
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

char* read_whole_file(char* path, size_t* srcLen) {
	size_t fsize, total_read = 0, bytes_read;
	char* contents;
	FILE* f;
	
	
	f = fopen(path, "rb");
	if(!f) {
		fprintf(stderr, "Could not open file \"%s\"\n", path);
		return NULL;
	}
	
	fseek(f, 0, SEEK_END);
	fsize = ftell(f);
	rewind(f);
	
	contents = malloc(fsize + 1);
	
	while(total_read < fsize) {
		bytes_read = fread(contents + total_read, sizeof(char), fsize - total_read, f);
		total_read += bytes_read;
	}
	
	contents[fsize] = 0;
	
	fclose(f);
	
	if(srcLen) *srcLen = fsize;
	
	return contents;
}






