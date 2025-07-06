#include <stdio.h>
#include <stdlib.h>

int main() {
	printf("App: Starting...\n");
	printf("App: Allocating memory...\n");

	void *p1 = malloc(10);
	void *p2 = malloc(30);

	printf("App: Memory allocated at %p\n", p1);
	printf("App: Memory allocated at %p\n", p2);

	printf("App: Freeing memory...\n");

	free(p1);
	free(p2);
	free(NULL);

	printf("App: Done.\n");
	return 0;
}
