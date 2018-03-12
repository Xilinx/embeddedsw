/******************************************************************************
*
* Copyright (C) 2009 - 2017 Xilinx, Inc.  All rights reserved.
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

#include <string.h>
#include "lwip_example_webserver.h"

#define HTTP_ARG_ARRAY_SIZE 1000

static http_arg httpArgArray[HTTP_ARG_ARRAY_SIZE];
static int p_arg_count;
static int httpArgArrayIndex;

int is_cmd_print(char *buf)
{
	/* skip past 'POST /' */
	buf += 6;

	/* then check for cmd/printxhr */
	return (!strncmp(buf, "cmd", 3) && !strncmp(buf + 4, "printxhr", 8));
}

void extract_file_name(char *filename, char *req, int rlen, int maxlen)
{
	char *fstart, *fend;
	int offset;

	/* first locate the file name in the request */
	/* requests are of the form GET /path/to/filename HTTP... */

	offset = strlen("GET ");

	if (req[offset] == '/')
		offset++;

	fstart = req + offset;   /* start marker */

	/* file name finally ends in a space */
	while (req[offset] != ' ')
		offset++;

	fend = req + offset - 1; /* end marker */

	if (fend < fstart) {
		strcpy(filename, "index.htm");
		return;
	}

	/* if there is something wrong with the URL & we ran for for more than
	 * the HTTP buffer length (offset > rlen) or the filename is too long,
	 * throw 404 error */
	if (offset > rlen || fend - fstart > maxlen) {
		*fend = 0;
		strcpy(filename, "404.htm");
		printf("Request filename is too long, length = %d, file = %s (truncated), max = %d\r\n",
				(int)(fend - fstart), fstart, maxlen);
		return;
	}

	/* copy over the filename */
	strncpy(filename, fstart, fend-fstart+1);
	filename[fend-fstart+1] = 0;

	/* if last character is a '/', append index.htm */
	if (*fend == '/')
		strcat(filename, "index.htm");
}

char *get_file_extension(char *fname)
{
	char *fext = fname + strlen(fname) - 1;

	while (fext > fname) {
		if (*fext == '.')
			return fext + 1;
		fext--;
	}

	return NULL;
}

int generate_http_header(char *buf, char *fext, int fsize)
{
	char lbuf[40];

	strcpy(buf, "HTTP/1.1 200 OK\r\nContent-Type: ");

	if (fext == NULL)
		strcat(buf, "text/html");	/* for unknown types */
	else if (!strncmp(fext, "htm", 3))
		strcat(buf, "text/html");	/* html */
	else if (!strncmp(fext, "jpg", 3))
		strcat(buf, "image/jpeg");
	else if (!strncmp(fext, "gif", 3))
		strcat(buf, "image/gif");
	else if (!strncmp(fext, "jsn", 3))
		strcat(buf, "application/json");
	else if (!strncmp(fext, "js", 2))
		strcat(buf, "application/javascript");
	else if (!strncmp(fext, "pdf", 2))
		strcat(buf, "application/pdf");
	else if (!strncmp(fext, "css", 2))
		strcat(buf, "text/css");
	else
		strcat(buf, "text/plain");	/* for unknown types */
	strcat(buf, "\r\n");

	sprintf(lbuf, "Content-length: %d", fsize);
	strcat(buf, lbuf);
	strcat(buf, "\r\n");

	strcat(buf, "Connection: close\r\n");
	strcat(buf, "\r\n");

	return strlen(buf);
}

http_arg *palloc_arg()
{
	http_arg *a = &(httpArgArray[httpArgArrayIndex]);
	httpArgArrayIndex++;
	if (httpArgArrayIndex == HTTP_ARG_ARRAY_SIZE)
		httpArgArrayIndex = 0;
	a->count = p_arg_count++;
	memset(&a->fil, 0, sizeof(a->fil));
	a->fsize = 0;

	return a;
}

void pfree_arg(http_arg *arg)
{
	;
}
