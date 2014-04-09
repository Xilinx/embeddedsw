/////////////////////////////////////////////////////////////////////////-*-C-*- 
//
// Copyright (c) 2002, 2003 Xilinx, Inc.  All rights reserved.
//
// Xilinx, Inc.
//
// XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A 
// COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
// ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR 
// STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
// IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE 
// FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.  
// XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO 
// THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO 
// ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE 
// FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Description : 
//  Test program to natively compile and test MFS on a host machine
// This program has been compiled and tested using gcc under Cygwin and Solariste
//          gcc test_mfs_filesys.c  mfs_filesys.c mfs_filesys_util.c -o test_mfs_filesys
//
// $Id: test_mfs_filesys.c,v 1.1.16.7 2010/10/01 18:53:25 jece Exp $
//
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <string.h>
#include "xilmfs.h"

struct mfs_file_block efs[200];

int main(int argc, char *argv[]) {
  char buf[512];
  char buf2[512];
  int fdr;
  int fdw;
  int tmp;
  int num_iter;
  mfs_init_fs(20*sizeof(struct mfs_file_block), (char *)efs, MFSINIT_NEW);
  fdr = mfs_file_open(".", MFS_MODE_READ);
  tmp = mfs_file_read(fdr, &(buf[0]), 512);
  tmp = mfs_file_close(fdr);
  tmp = mfs_ls();
  tmp = mfs_create_dir("testdir1");
  tmp = mfs_create_dir("testdir2");
  tmp = mfs_create_dir(".");
  tmp = mfs_ls();
  tmp = mfs_change_dir("testdir1");
  tmp = mfs_create_dir("testdir3");
  tmp = mfs_create_dir("testdir1");
  tmp = mfs_ls();
  tmp = mfs_change_dir("testdir3");
  fdw = mfs_file_open("testfile1", MFS_MODE_CREATE);
  strcpy(buf,"this is a test string");
  for (num_iter = 0; num_iter < 100; num_iter++)
    tmp = mfs_file_write(fdw, buf, strlen(buf));
  fdr = mfs_file_open("testfile1", MFS_MODE_READ);
  while((tmp= mfs_file_read(fdr, buf2, 512))== 512){
    buf2[511]='\0';
    strcpy(buf, buf2);
  }
  tmp = mfs_file_close(fdr);
  tmp = mfs_file_close(fdw);
  tmp = mfs_file_close(fdw); /* should return error */
  tmp = mfs_ls();
  tmp = mfs_cat("testfile1");
  for (num_iter=65; num_iter < 91; num_iter++) {
    strcpy(buf,"testfileA");
    buf[8] = num_iter;
    tmp = mfs_create_dir(buf);
  }
  tmp = mfs_ls();
  tmp = mfs_change_dir("testfileX");
  tmp = mfs_change_dir("..");
  tmp = mfs_change_dir(".");
  tmp = mfs_ls();
  tmp = mfs_delete_dir("testfileX");
  tmp = mfs_delete_dir(".");
  tmp = mfs_delete_file("testfile1");
  tmp = mfs_cat("testfile1"); /* should return error */
  tmp = mfs_ls();
  tmp = mfs_copy_stdin_to_file("stdinfile");
  tmp = mfs_ls();
  tmp = mfs_cat("stdinfile");
  tmp = mfs_rename_file("stdinfile", "tmp1");
  tmp = mfs_file_copy("tmp1", "tmp2");
  tmp = mfs_ls();
  tmp = mfs_cat("tmp2");
  tmp = mfs_create_dir("/testpath1");
  tmp = mfs_ls();
  tmp = mfs_create_dir("/testpath1/testpath2");
  tmp = mfs_change_dir("/testpath1/testpath2");
  tmp = mfs_ls();
  tmp = mfs_change_dir("/testpath1");
  tmp = mfs_ls();
  tmp = mfs_change_dir("/testpath2");
  tmp = mfs_change_dir("testpath2");
  tmp = mfs_create_dir("/a/b/c");
  tmp = mfs_change_dir("/testpath2/");
  tmp = mfs_change_dir("/testpath1/");
  tmp = mfs_ls();
  tmp = mfs_change_dir("/testpath1/testpath2/");
  tmp = mfs_ls();
  tmp = mfs_create_dir("testpath3/");
  tmp = mfs_ls();
  tmp = mfs_delete_dir("/testpath1/testpath2/testpath3");
  tmp = mfs_change_dir("/testpath1");
  tmp = mfs_delete_dir("/testpath1/testpath2/");
  tmp = mfs_ls();
  tmp = mfs_change_dir("/");
  tmp = mfs_ls();
  fdw = mfs_file_open("testappend", MFS_MODE_CREATE);
  strcpy(buf2,"testing append mode...\n");
  tmp = mfs_file_write(fdw, buf2, strlen(buf2));
  tmp = mfs_file_close(fdw);
  tmp = mfs_cat("testappend");
  fdw = mfs_file_open("testappend", MFS_MODE_WRITE);
  tmp = mfs_file_lseek(fdw, 0, MFS_SEEK_END);
  strcpy(buf2, "testing append mode2\n");
  tmp = mfs_file_write(fdw, buf2, strlen(buf2));
  tmp = mfs_file_close(fdw);
  tmp = mfs_cat("testappend");
  fdw = mfs_file_open("testappend", MFS_MODE_WRITE);
  tmp = mfs_file_lseek(fdw, 10, MFS_SEEK_SET);
  strcpy(buf2, "testing append mode3\n");
  tmp = mfs_file_write(fdw, buf2, strlen(buf2));
  tmp = mfs_file_lseek(fdw,-10, MFS_SEEK_END);
  tmp = mfs_file_write(fdw, buf2, strlen(buf2));
  tmp = mfs_file_close(fdw);
  tmp = mfs_cat("testappend");

  return 0;
}



/* testing testing */
