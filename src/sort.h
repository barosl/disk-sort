#pragma once

extern void sel_sort(int n, int el_size, void *user_data, void (*getter)(void *user_data, int idx, void *el), void (*setter)(void *user_data, int idx, void *el), int (*cmp)(void *a, void *b));
extern void heap_sort(int n, int el_size, void *user_data, void (*getter)(void *user_data, int idx, void *el), void (*setter)(void *user_data, int idx, void *el), int (*cmp)(void *a, void *b));
