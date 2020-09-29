/* Copyright (C) 2020 David Brunecz. Subject to GPL 2.0 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define MAX(x, y)	((x) > (y) ? (x) : (y))
#define MIN(x, y)	((x) < (y) ? (x) : (y))

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;

/******************************************************************************/

u32 tickcount_ms(void)
{
	struct timespec t;

	if (clock_gettime(CLOCK_MONOTONIC, &t)) {
		fprintf(stderr, "%s:%d %s()\n", __FILE__, __LINE__, __func__);
		exit(-1);
	}
	return t.tv_sec * 1000 + (t.tv_nsec/1000000);
}

/******************************************************************************/

struct bmap {
	u16 hdr;
	u32 sz;
	u16 rsv1;
	u16 rsv2;
	u32 offs;
	struct dib {
		u32 sz;
		u32 wd;
		u32 ht;
		u16 numclrplane;
		u16 bpp;
		u32 cmp;
		u32 imgsz;
		u32 hres;
		u32 vres;
		u32 numclrplt;
		u32 impclt;
	} dib;
	struct rgbquad {
		u32 rgb;
	} pm[256];
	u8 dat[];
} __attribute__((packed));

#define BI_RGB  0
#define BI_RLE8 1

void display_bitmap_header(struct bmap *b)
{
	fprintf(stderr,
		"%*s : %04x\n"
		"%*s : %d\n"
		"%*s : %d\n"
		"%*s : %d\n"
		"%*s : %d\n"
		"%*s : %d\n"
		"%*s : %d\n"
		"%*s : %d\n"
		"%*s : %d\n"
		"%*s : %d\n"
		"%*s : %d\n"
		"%*s : %d\n"
		"%*s : %d\n"
		"%*s : %d\n"
		"\n"
		,12, "hdr", b->hdr
		,12, "sz", b->sz
		,12, "offs", b->offs
		,14, "sz", b->dib.sz
		,14, "wd", b->dib.wd
		,14, "ht", b->dib.ht
		,14, "numclrplane", b->dib.numclrplane
		,14, "bpp", b->dib.bpp
		,14, "cmp", b->dib.cmp
		,14, "imgsz", b->dib.imgsz
		,14, "hres", b->dib.hres
		,14, "vres", b->dib.vres
		,14, "numclrplt", b->dib.numclrplt
		,14, "impclt", b->dib.impclt
		);
}

struct bmap *loadbitmap(const char *fname)
{
	struct bmap *b;
	struct stat st;
	int fd;

	if (stat(fname, &st)) {
		perror(fname);
		return NULL;
	}

	fd = open(fname, O_RDONLY);
	if (fd < 0) {
		perror(fname);
		return NULL;
	}

	b = malloc(st.st_size);
	if (!b) {
		close(fd);
		return NULL;
	}

	if (st.st_size != read(fd, (void *)b, st.st_size)) {
		perror(fname);
		close(fd);
		return NULL;
	}
	//display_bitmap_header(b);
	return b;
}

#define BITMAP_HEADER_MAGIC	0x4d42
int writebitmap(int fd, int x, int y, int bpp, u8 *dat)
{
	struct bmap bitmap = { 0 }, *b = &bitmap;
	u32 sz = (bpp / 8) * x * y;
	u32 i;

	b->hdr = BITMAP_HEADER_MAGIC;
	b->sz = sizeof(*b) + sz;
	b->offs = (uintptr_t)b->dat - (uintptr_t)b;

	b->dib.sz = sizeof(b->dib);
	b->dib.wd = x;
	b->dib.ht = y;
	b->dib.numclrplane = 1;
	b->dib.bpp = bpp;
	b->dib.cmp = BI_RGB;
	//b->dib.cmp = BI_RLE8;

	//display_bitmap_header(b);

	if (b->dib.bpp == 8)
		for (i = 0; i < 256; i++)
			b->pm[i].rgb = i << 16 | i << 8 | i;

	if ((sizeof(*b) != write(fd, b, sizeof(*b))) ||
	    (sz != write(fd, dat, sz))) {
		fprintf(stderr, "write: %d %s\n", errno, strerror(errno));
		return -1;
	}
	return 0;
}

int storebitmap(const char *fname, u32 x, u32 y, int bpp, u8 *dat)
{
	int ret, fd;

	fd = open(fname, O_CREAT | O_WRONLY, 0644);
	if (fd < 0) {
		perror(fname);
		return -1;
	}

	ret = writebitmap(fd, x, y, bpp, dat);

	close(fd);
	return ret;
}

/******************************************************************************/

#if 0
int k[5][5] = {
	{ -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1 },
	{ -1, -1, 24, -1, -1 },
	{ -1, -1, -1, -1, -1 },
	{ -1, -1, -1, -1, -1 },
};
#endif

#if 0
int k[5][5] = {
	{  0, -1,  1, -1,  0 },
	{ -1, -3,  3, -3, -1 },
	{  1,  3,  4,  3,  1 },
	{ -1, -3,  3, -3, -1 },
	{  0, -1,  1, -1,  0 },
};
#endif

#if 0
int k[3][3] = {
	{ -3,  2, -3 },
	{  2,  4,  2 },
	{ -3,  2, -3 },
};
#endif

#if 0
int k[3][3] = {
	{  2, -2,  2 },
	{ -2,  0, -2 },
	{  2, -2,  2 },
};
#endif

int fake3d[3][3] = {
	{  0,  0,  0 },
	{  0,  1,  0 },
	{  0,  0, -1 },
};

int edge3x3[3][3] = {
	{ -1, -1, -1 },
	{ -1,  8, -1 },
	{ -1, -1, -1 },
};

int identity[3][3] = {
	{ 0, 0, 0 },
	{ 0, 1, 0 },
	{ 0, 0, 0 },
};

#define TIMESTAMP()	fprintf(stderr, "%s %dx%dx%d, %u ms\n", __func__,	\
				b->dib.wd, b->dib.ht, b->dib.bpp / 8, ms);

#define INDEX(b, h, w, pxsz, d)	((h) * ((((b)->dib.wd * pxsz) + 3) & ~3)\
				 + ((w) * pxsz) + (d))

// 0x00RRGGBB(u32) == { BB, GG, RR, 00 }(u8[])

struct kernel {
	int w;
	int h;
	int *k;
};

//struct kernel k1 = { 3, 3, (int *)fake3d };
struct kernel k1 = { 3, 3, (int *)edge3x3 };
//struct kernel k1 = { 5, 5, (int *)k };

static inline void apply_kernel(u8 *out, struct bmap *b, int h, int w, int p,
				struct kernel *k)
{
	u8 *in = (void *)b + b->offs;
	int pxsz = b->dib.bpp / 8;
	int kw, kh;
	int v = 0;

	for (kh = 0; kh < k->h; kh++)
		for (kw = 0; kw < k->w; kw++)
			v += in[INDEX(b, h + kh, w + kw, pxsz, p)]
			     * k->k[kh * k->w + kw];
	out[INDEX(b, h, w, pxsz, p)] = MAX(MIN(v, 255), 0);
}

void filter(u8 *out, struct bmap *b, struct kernel *k)
{
	int pxsz = b->dib.bpp / 8;
	int wo = k->w / 2;
	int ho = k->h / 2;
	int h, w, p;
	u32 ms;

	ms = tickcount_ms();
	for (h = ho; h < b->dib.ht - ho; h++)
		for (w = wo; w < b->dib.wd - wo; w++)
			for (p = 0; p < pxsz; p++)
				apply_kernel(out, b, h - ho, w - wo, p, k);
	ms = tickcount_ms() - ms;
	TIMESTAMP();
}

void average(u8 *out, struct bmap *b)
{
	u8 *in = (void *)b + b->offs;
	int pxsz = b->dib.bpp / 8;
	int h, w, p, v;
	u32 ms;

	ms = tickcount_ms();
	for (h = 0; h < b->dib.ht; h++) {
		for (w = 0; w < b->dib.wd; w++) {
			for (p = 0; p < pxsz; p++) {
				v = out[INDEX(b, h, w, pxsz, p)] +
				     in[INDEX(b, h, w, pxsz, p)];
				out[INDEX(b, h, w, pxsz, p)] = v / 2;
			}
		}
	}

	ms = tickcount_ms() - ms;
	TIMESTAMP();
}

void negative(u8 *out, struct bmap *b)
{
	u8 *in = (void *)b + b->offs;
	int pxsz = b->dib.bpp / 8;
	int h, w, p;
	u32 ms;

	ms = tickcount_ms();
	for (h = 0; h < b->dib.ht; h++)
		for (w = 0; w < b->dib.wd; w++)
			for (p = 0; p < pxsz; p++)
				out[INDEX(b, h, w, pxsz, p)] =
					0xff - in[INDEX(b, h, w, pxsz, p)];

	ms = tickcount_ms() - ms;
	TIMESTAMP();
}

void grayscale(u8 *out, struct bmap *b)
{
	u8 *in = (void *)b + b->offs;
	int pxsz = b->dib.bpp / 8;
	int h, w, p, v;
	u32 ms;

	ms = tickcount_ms();
	for (h = 0; h < b->dib.ht; h++) {
		for (w = 0; w < b->dib.wd; w++) {
			v = 0;
			for (p = 0; p < pxsz; p++)
				v += in[INDEX(b, h, w, pxsz, p)];
			v = v / 3;
			out[INDEX(b, h, w, pxsz, 0)] = v;
			out[INDEX(b, h, w, pxsz, 1)] = v;
			out[INDEX(b, h, w, pxsz, 2)] = v;
		}
	}

	ms = tickcount_ms() - ms;
	TIMESTAMP();
}

int main(int argc, char *argv[])
{
	struct bmap *b;
	u8 *out;
	u32 sz;

	b = loadbitmap(argv[1]);
	if (!b)
		return EXIT_FAILURE;

	sz = b->dib.ht * b->dib.wd * b->dib.bpp / 8;

	out = malloc(sz);
	memset(out, 0, sz);
	//memcpy(out, in, sz);
	//average(out, b);

	negative(out, b);
	storebitmap("/tmp/negative.bmp", b->dib.wd, b->dib.ht, b->dib.bpp, out);

	grayscale(out, b);
	storebitmap("/tmp/grayscale.bmp", b->dib.wd, b->dib.ht, b->dib.bpp, out);

	filter(out, b, &k1);
	storebitmap("/tmp/filter.bmp", b->dib.wd, b->dib.ht, b->dib.bpp, out);

	return EXIT_SUCCESS;
}
