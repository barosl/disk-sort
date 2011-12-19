#include "sort.h"
#include <stdlib.h>
#include <string.h>

void sel_sort(int n, int el_size, void *user_data, void (*getter)(void *user_data, int idx, void *el), void (*setter)(void *user_data, int idx, void *el), int (*cmp)(void *a, void *b)) {
	int i, j;

	void *a = malloc(el_size);
	void *b = malloc(el_size);

	for (i=0;i<n-1;i++) {
		for (j=i+1;j<n;j++) {
			getter(user_data, i, a);
			getter(user_data, j, b);

			if (cmp(a, b) > 0) {
				setter(user_data, i, b);
				setter(user_data, j, a);
			}
		}
	}

	free(a);
	free(b);
}

static void heap_adjust(int i, int n, int el_size, void *a, void *b, void *c, void *user_data, void (*getter)(void *user_data, int idx, void *el), void (*setter)(void *user_data, int idx, void *el), int (*cmp)(void *a, void *b)) {
	int j;

	getter(user_data, i, c);

	for (j=i*2+1;j<n;j=j*2+1) {
		getter(user_data, j, a);

		if (j+1 < n) {
			getter(user_data, j+1, b);

			if (cmp(a, b) < 0) {
				j++;
				memcpy(a, b, el_size);
			}
		}

		if (cmp(c, a) >= 0) break;

		setter(user_data, (j-1)/2, a);
	}

	setter(user_data, (j-1)/2, c);
}

void heap_sort(int n, int el_size, void *user_data, void (*getter)(void *user_data, int idx, void *el), void (*setter)(void *user_data, int idx, void *el), int (*cmp)(void *a, void *b)) {
	int i;

	void *a = malloc(el_size);
	void *b = malloc(el_size);
	void *c = malloc(el_size);

	for (i=n/2-1;i>=0;i--) {
		heap_adjust(i, n, el_size, a, b, c, user_data, getter, setter, cmp);
	}

	for (i=n-1;i>=1;i--) {
		getter(user_data, 0, a);
		getter(user_data, i, b);
		setter(user_data, i, a);
		setter(user_data, 0, b);

		heap_adjust(0, i, el_size, a, b, c, user_data, getter, setter, cmp);
	}

	free(a);
	free(b);
	free(c);
}
