/******************************************************************************
*
* Copyright (C) 2002 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

#include <stdio.h>
#include "xilmfs.h"

/* some redefinitions for host based testing */
#ifdef TESTING_XILMFS
#define putnum(x) printf("%d ", (x))
#define print(x) printf("%s", (x))
#define inbyte() fgetc(stdin)
#endif

/**
 * list contents of current directory
 * @return 1 on success and 0 on failure
 */
int mfs_ls() {
  return mfs_ls_r(0);
}

/**
 * recursive directory listing
 * list the contents of current directory
 * if any of the entries in the current directory is itself a directory,
 * immediately enter that directory and call mfs_ls_r() once again
 * @param recurse
 * If parameter recurse is non zero continue recursing
 * else stop recursing
 * recurse=0 lists just the current directory
 * recurse = -1 allows unlimited recursion
 * recurse = n stops recursing at a depth of n
 * @return 1 on success and 0 on failure
 */

int mfs_ls_r(int recurse) {
  int fd = mfs_dir_open(".");
  int entry_type;
  int entry_size;
  char *entry_name;
  while (mfs_dir_read(fd, &entry_name, &entry_size, &entry_type) != 0) {
    if (entry_type == MFS_BLOCK_TYPE_DIR) {

      if (!(entry_name[0] == '.' && entry_name[1] == '\0') &&
          !(entry_name[0] == '.' && entry_name[1] == '.' && entry_name[2] == '\0')) {
#if defined (TESTING_XILMFS)
	print("Directory ");
	print(entry_name);
	print(" ");
	putnum(entry_size);
	print("\r\n");
#endif
	if (recurse != 0) {
	  if (!mfs_change_dir(entry_name)) {
#if defined (TESTING_XILMFS)
	    print("Failed\r\n");
#endif
	    mfs_dir_close(fd);
	    return 0;
	  }
	  if (!mfs_ls_r(recurse-1)) {
	    mfs_dir_close(fd);
	    return 0;
	  }
	  if(mfs_change_dir("..")) {
#if defined (TESTING_XILMFS)
		print("cd..\r\n");
#endif
	  }
	  else {
#if defined (TESTING_XILMFS)
	    print("Failed..\r\n");
#endif
	    mfs_dir_close(fd);
	    return 0;
	  }
	}
      }
    }
    else {
#if defined (TESTING_XILMFS)
      print(entry_name);
      print(" ");
      putnum(entry_size);
      print("\r\n");
#endif
    }
  }
  mfs_dir_close(fd);
  return 1;
}

/**
 * print the file to stdout
 * @param filename - file to print
 * @return 1 on success, 0 on failure
 */
int mfs_cat(char *filename) {
  char buf2[513];
  int tmp;
  int fdr = mfs_file_open(filename, MFS_MODE_READ);
  if (fdr < 0) { /* error opening file */
#if defined (TESTING_XILMFS)
    print("Cannot open file\n");
#endif
    return 0;
  }
  while((tmp= mfs_file_read(fdr, buf2, 512))== 512){
    buf2[512]='\0';
#if defined (TESTING_XILMFS)
    print(buf2);
#endif
  }
  if (tmp > 0) {
    buf2[tmp] = '\0';
#if defined (TESTING_XILMFS)
    print(buf2);
#endif
  }
  tmp = mfs_file_close(fdr);
#if defined (TESTING_XILMFS)
  print("\n");
#endif
  return 1;
}

/* FIXME declare inbyte locally to avoid g++ compilation issues
 * this should come from a header file if inbyte is ever included in a header
 */
#if !defined(TESTING_XILMFS)
char inbyte(void);
#endif

/**
 * copy from stdin to named file
 * @param filename - file to print
 * @return 1 on success, 0 on failure
 */
int mfs_copy_stdin_to_file(char *filename) {
  char buf2[512];
  int offset = 0;
  int c;
  int fdw = mfs_file_open(filename, MFS_MODE_CREATE);
  if (fdw < 0) { /* cannot open file */
#if defined (TESTING_XILMFS)
    print ("Cannot open file\n");
#endif
    return 0;
  }
  while ((c = inbyte()) != EOF) {
    buf2[offset++] = c;
    if (offset == 512) {
      mfs_file_write(fdw, buf2, 512);
      offset = 0;
    }
  }
  if (offset != 512) { /* write the last buffer */
    mfs_file_write(fdw, buf2, offset);
  }
  mfs_file_close(fdw);
  return 1;
}

/**
 * copy from_file to to_file
 * to_file is created new
 * copy fails if to_file exists already
 * copy fails is from_file or to_file cannot be opened
 * @param from_file
 * @param to_file
 * @return 1 on success, 0 on failure
 */

int mfs_file_copy(char *from_file, char *to_file) {
  char buf2[512];
  int tmp;
  int fdr = mfs_file_open(from_file, MFS_MODE_READ);
  int fdw = mfs_file_open(to_file, MFS_MODE_CREATE);
  if (fdr < 0 || fdw < 0) { /* error opening file */
#if defined (TESTING_XILMFS)
    print("Cannot open file to read/write\n");
#endif
    mfs_file_close(fdr);
    mfs_file_close(fdw);
    return 0;
  }
  while((tmp= mfs_file_read(fdr, buf2, 512))== 512){
    mfs_file_write(fdw, buf2, 512);
  }
  if (tmp > 0) {
    mfs_file_write(fdw, buf2, tmp);
  }
  tmp = mfs_file_close(fdr);
  tmp = mfs_file_close(fdw);
  return 1;
}






















