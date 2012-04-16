$NetBSD$

--- erts/emulator/beam/copy.c.orig	2011-05-24 11:16:43.000000000 +0000
+++ erts/emulator/beam/copy.c
@@ -30,6 +30,7 @@
 #include "big.h"
 #include "erl_binary.h"
 #include "erl_bits.h"
+#include "dtrace-wrapper.h"
 
 #ifdef HYBRID
 MA_STACK_DECLARE(src);
@@ -59,6 +60,12 @@ copy_object(Eterm obj, Process* to)
     Eterm* hp = HAlloc(to, size);
     Eterm res;
 
+    if (DTRACE_ENABLED(copy_object)) {
+        DTRACE_CHARBUF(proc_name, 64);
+
+        erts_snprintf(proc_name, sizeof(proc_name), "%T", to->id);
+        DTRACE2(copy_object, proc_name, size);
+    }
     res = copy_struct(obj, size, &hp, &to->off_heap);
 #ifdef DEBUG
     if (eq(obj, res) == 0) {
@@ -213,6 +220,8 @@ Eterm copy_struct(Eterm obj, Uint sz, Et
     if (IS_CONST(obj))
 	return obj;
 
+    DTRACE1(copy_struct, (int32_t)sz);
+
     hp = htop = *hpp;
     hbot   = htop + sz;
     hstart = (char *)htop;
