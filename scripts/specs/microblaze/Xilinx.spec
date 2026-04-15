# Copyright (C) 2023-2026 Advanced Micro Devices, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
*startfile:
%{mxl-mode-bootstrap:%:if-exists(crt2%O%s) %:if-exists(crti%O%s) %:if-exists(crtbegin%O%s) %:if-exists(crtinit%O%s)}%{mxl-mode-novectors:%:if-exists(crt3%O%s) %:if-exists(crti%O%s) %:if-exists(crtbegin%O%s) %:if-exists(crtinit%O%s)}%{!mxl-mode-bootstrap:%{!mxl-mode-novectors:%:if-exists(crt0%O%s) %:if-exists(crti%O%s) %:if-exists(crtbegin%O%s) %:if-exists(crtinit%O%s)}}
