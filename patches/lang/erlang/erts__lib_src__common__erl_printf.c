$NetBSD$

--- erts/lib_src/common/erl_printf.c.orig	2011-10-03 18:12:07.000000000 +0000
+++ erts/lib_src/common/erl_printf.c
@@ -173,6 +173,7 @@ typedef struct {
 static int
 write_sn(void *vwsnap, char* buf, size_t len)
 {
+    int rv = 0;
     write_sn_arg_t *wsnap = (write_sn_arg_t *) vwsnap;
     ASSERT(wsnap);
     ASSERT(len > 0);
@@ -180,12 +181,13 @@ write_sn(void *vwsnap, char* buf, size_t
 	size_t sz = len;
 	if (sz >= wsnap->len)
 	    sz = wsnap->len;
+	rv = (int)sz;
 	memcpy((void *) wsnap->buf, (void *) buf, sz);
 	wsnap->buf += sz;
 	wsnap->len -= sz;
 	return sz;
     }
-    return 0;
+    return rv;
 }
 
 static int
