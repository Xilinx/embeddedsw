/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Xilinx nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "metal-test.h"
#include "metal/sys.h"
#include "metal/utilities.h"

static METAL_DECLARE_LIST(test_cases);

void metal_add_test_case(struct metal_test_case *test_case)
{
	metal_list_add_tail(&test_cases, &test_case->node);
}

int metal_tests_run(void)
{
	struct metal_init_params params = METAL_INIT_DEFAULTS;
	struct metal_test_case *test_case;
	struct metal_list *node;
	int error, errors = 0;
	const char *dots = "..................................";
	const char *pad;

	params.log_level = LOG_DEBUG;
	error = metal_init(&params);
	if (error)
		return error;

	metal_list_for_each(&test_cases, node) {
		test_case = metal_container_of(node, struct metal_test_case,
					       node);
		pad = dots + strlen(test_case->name);
		printf("running [%s]\n", test_case->name);
		error = test_case->test();
		printf("result [%s]%s %s%s%s\n",
		       test_case->name, pad, error ? "fail" : "pass",
		       error ? " - error: " : "",
		       error ? strerror(-error) : "");
		if (error)
			errors++;
	}

	metal_finish();

	return errors;
}
