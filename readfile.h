#ifndef __READFILE_H__
#define __READFILE_H__

#define READFILE_CHUNK (64 * 1024)
char *readfile(FILE *in, int *length);

#endif

