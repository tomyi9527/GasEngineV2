/*
 * Copyright 2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef HEADER_DSO_CONF_H
# define HEADER_DSO_CONF_H

# define DSO_EXTENSION "@DSO_EXTENSION@"
#if !defined(DSO_VMS) && !defined(DSO_DLCFN) && !defined(DSO_DL) && !defined(DSO_WIN32) && !defined(DSO_DLFCN)
#define DSO_NONE
#endif
#endif
