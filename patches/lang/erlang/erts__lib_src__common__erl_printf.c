$NetBSD$

--- erts/lib_src/common/erl_printf.c.orig	2011-05-24 11:16:43.000000000 +0000
+++ erts/lib_src/common/erl_printf.c
@@ -172,6 +172,7 @@ typedef struct {
 static int
 write_sn(void *vwsnap, char* buf, size_t len)
 {
+    int rv = 0;
     write_sn_arg_t *wsnap = (write_sn_arg_t *) vwsnap;
     ASSERT(wsnap);
     ASSERT(len > 0);
