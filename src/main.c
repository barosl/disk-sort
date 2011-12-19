#include <stdio.h>
#include <string.h>
#include "str.h"
#include "sort.h"
#include "clock.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>

#define MAX_NAME_LEN 200

typedef struct {
	char name[MAX_NAME_LEN+1];
	int name_len;
} rec_t;

static int read_from_text(const char *fpath_in, FILE *fout) {
	FILE *fin = fopen(fpath_in, "r");
	if (!fin) return -1;

	char buf[1024];

	while (fgets(buf, sizeof(buf), fin)) {
		if (!*buf || buf[strlen(buf)-1] != '\n') continue;
		char *name = str_strip(buf);
		if (!*name) continue;

		rec_t rec;
		snprintf(rec.name, sizeof(rec.name), "%s", name);
		rec.name_len = strlen(rec.name);

		if (fwrite(&rec, sizeof(rec), 1, fout) != 1) {
			fclose(fin);
			return -1;
		}
	}

	fclose(fin);

	return 0;
}

void num_getter(void *user_data, int idx, void *el) {
	*(int*)el = ((int*)user_data)[idx];
}

void num_setter(void *user_data, int idx, void *el) {
	((int*)user_data)[idx] = *(int*)el;
}

int num_cmp(void *a, void *b) {
	return *(int*)a - *(int*)b;
}

void rec_stdio_getter(void *user_data, int idx, void *el) {
	FILE *fp = (FILE*)user_data;

	fseek(fp, sizeof(rec_t)*idx, SEEK_SET);
	fread(el, sizeof(rec_t), 1, fp);
}

void rec_stdio_setter(void *user_data, int idx, void *el) {
	FILE *fp = (FILE*)user_data;

	fseek(fp, sizeof(rec_t)*idx, SEEK_SET);
	fwrite(el, sizeof(rec_t), 1, fp);
}

int rec_stdio_cmp(void *a, void *b) {
	return strcmp(((rec_t*)a)->name, ((rec_t*)b)->name);
}

void rec_posix_getter(void *user_data, int idx, void *el) {
	int fd = (int)(long)user_data;

	lseek(fd, sizeof(rec_t)*idx, SEEK_SET);
	read(fd, el, sizeof(rec_t));
}

void rec_posix_setter(void *user_data, int idx, void *el) {
	int fd = (int)(long)user_data;

	lseek(fd, sizeof(rec_t)*idx, SEEK_SET);
	write(fd, el, sizeof(rec_t));
}

int rec_posix_cmp(void *a, void *b) {
	return strcmp(((rec_t*)a)->name, ((rec_t*)b)->name);
}

void rec_mem_getter(void *user_data, int idx, void *el) {
	rec_t *rec_l = (rec_t*)user_data;

	memcpy(el, &rec_l[idx], sizeof(rec_t));
}

void rec_mem_setter(void *user_data, int idx, void *el) {
	rec_t *rec_l = (rec_t*)user_data;

	memcpy(&rec_l[idx], el, sizeof(rec_t));
}

int rec_mem_cmp(void *a, void *b) {
	return strcmp(((rec_t*)a)->name, ((rec_t*)b)->name);
}

static void write_to_text(const char *fpath, FILE *fin) {
	FILE *fout = fopen(fpath, "w");

	rec_t rec;

	fseek(fin, 0, SEEK_SET);
	while (fread(&rec, sizeof(rec), 1, fin) == 1) {
		fprintf(fout, "%s\n", rec.name);
	}

	fclose(fout);
}

static int input(const char *fpath_bin, const char *fpath_in_1, const char *fpath_in_2) {
	FILE *fp = fopen(fpath_bin, "w+b");
	if (!fp) {
		fprintf(stderr, "error writing file\n");
		return -1;
	}

	if (read_from_text(fpath_in_1, fp) || read_from_text(fpath_in_2, fp)) {
		fclose(fp);

		fprintf(stderr, "error reading file\n");
		return -1;
	}

	fclose(fp);

	return 0;
}

static FILE *uniq(const char *fpath_in, const char *fpath_out) {
	FILE *fin = fopen(fpath_in, "rb");
	FILE *fout = fopen(fpath_out, "w+b");

	rec_t rec, prev_rec;
	*prev_rec.name = '\0';

	fseek(fin, 0, SEEK_SET);
	while (fread(&rec, sizeof(rec), 1, fin) == 1) {
		if (strcmp(rec.name, prev_rec.name)) {
			prev_rec = rec;
			fwrite(&rec, sizeof(rec), 1, fout);
		}
	}

	fclose(fin);

	return fout;
}

#define INTERFACE_TYPE 3

static int process(const char *fpath_bin, const char *fpath_uniq, const char *fpath_text) {
#if INTERFACE_TYPE == 0
	FILE *fp_bin = fopen(fpath_bin, "r+b");
	if (!fp_bin) return -1;
	fseek(fp_bin, 0, SEEK_END);

//	sel_sort(ftell(fp_bin)/sizeof(rec_t), sizeof(rec_t), fp_bin, rec_stdio_getter, rec_stdio_setter, rec_stdio_cmp);
	heap_sort(ftell(fp_bin)/sizeof(rec_t), sizeof(rec_t), fp_bin, rec_stdio_getter, rec_stdio_setter, rec_stdio_cmp);

	fclose(fp_bin);
#elif INTERFACE_TYPE == 1 || INTERFACE_TYPE == 2 || INTERFACE_TYPE == 3
	int fd_bin = open(fpath_bin, O_RDWR);
	if (fd_bin == -1) return -1;

#if INTERFACE_TYPE == 2
	off_t f_len = lseek(fd_bin, 0, SEEK_END);

	lseek(fd_bin, 0, SEEK_SET);
	rec_t *rec_l = (rec_t*)mmap(NULL, f_len, PROT_WRITE, MAP_SHARED, fd_bin, 0);

	heap_sort(f_len/sizeof(rec_t), sizeof(rec_t), rec_l, rec_mem_getter, rec_mem_setter, rec_mem_cmp);

	munmap(rec_l, f_len);
#elif INTERFACE_TYPE == 3
	off_t f_len = lseek(fd_bin, 0, SEEK_END);
	rec_t *rec_l = (rec_t*)malloc(f_len);

	lseek(fd_bin, 0, SEEK_SET);
	read(fd_bin, rec_l, f_len);

	heap_sort(f_len/sizeof(rec_t), sizeof(rec_t), rec_l, rec_mem_getter, rec_mem_setter, rec_mem_cmp);

	lseek(fd_bin, 0, SEEK_SET);
	write(fd_bin, rec_l, f_len);

	free(rec_l);
#else
	heap_sort(lseek(fd_bin, 0, SEEK_END)/sizeof(rec_t), sizeof(rec_t), (void*)fd_bin, rec_posix_getter, rec_posix_setter, rec_posix_cmp);
#endif

	close(fd_bin);
#endif

	FILE *fp_uniq = uniq(fpath_bin, fpath_uniq);

	write_to_text(fpath_text, fp_uniq);

	fclose(fp_uniq);

	return 0;
}

int main(void) {
	/*
	int l[] = {1, 5, 3, 7, 9, 2, -5, -9, 8};
	heap_sort(sizeof(l)/sizeof(l[0]), sizeof(l[0]), l, num_getter, num_setter, num_cmp);
	*/

	clock_init();

	int st = clock_msec();

	if (input("tmerge.bin", "Com_names1.txt", "Com_names2.txt") || process("tmerge.bin", "merge.bin", "Final_names.txt")) return -1;

	int et = clock_msec() - st;

	printf("Elapsed time: %dh %dm %ds %dms\n", et/1000/3600, et/1000%3600/60, et/1000%60, et%1000);

	return 0;
}
