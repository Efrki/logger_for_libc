#define _GNU_SOURCE
#include <dlfcn.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

static void *(*real_malloc)(size_t) = NULL;
static __thread int in_our_malloc = 0;

void *malloc(size_t size) {
	if (real_malloc == NULL) {
		real_malloc = dlsym(RTLD_NEXT, "malloc");

		if (real_malloc == NULL) {
			fprintf(stderr, "Error in dlsym: %s\n", dlerror());
			exit(1);
		}
	}

	if (in_our_malloc) {
		return real_malloc(size);
	}

	in_our_malloc = 1;

	fprintf(stderr, "malloc(%zu) called\n", size);

	void *p = real_malloc(size);

	fprintf(stderr, "malloc returned %p\n", p);

	in_our_malloc = 0;

	return p;
}

static void (*real_free)(void *) = NULL;
static __thread int in_our_free = 0;

void free(void *p) {
	if (real_free == NULL) {
		real_free = dlsym(RTLD_NEXT, "free");

		if (real_free == NULL) {
			fprintf(stderr, "Error in dlsym: %s\n", dlerror());
			exit(1);
		}
	}

	if (in_our_free) {
		real_free(p);
		return;
	}

	in_our_free = 1;

	if (p == NULL) {
		fprintf(stderr, "free(NULL) called\n");
	} else {
		fprintf(stderr, "free(%p) called\n", p);
	}
	real_free(p);

	in_our_free = 0;
}