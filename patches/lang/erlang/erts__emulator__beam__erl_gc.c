$NetBSD$

--- erts/emulator/beam/erl_gc.c.orig	2011-05-24 11:16:43.000000000 +0000
+++ erts/emulator/beam/erl_gc.c
@@ -35,6 +35,7 @@
 #include "hipe_stack.h"
 #include "hipe_mode_switch.h"
 #endif
+#include "dtrace-wrapper.h"
 
 #define ERTS_INACT_WR_PB_LEAVE_MUCH_LIMIT 1
 #define ERTS_INACT_WR_PB_LEAVE_MUCH_PERCENTAGE 20
@@ -344,6 +345,7 @@ erts_garbage_collect(Process* p, int nee
     Uint reclaimed_now = 0;
     int done = 0;
     Uint ms1, s1, us1;
+    DTRACE_CHARBUF(pidbuf, DTRACE_TERM_BUF_SIZE);
 
     if (IS_TRACED_FL(p, F_TRACE_GC)) {
         trace_gc(p, am_gc_start);
@@ -366,14 +368,26 @@ erts_garbage_collect(Process* p, int nee
         FLAGS(p) |= F_NEED_FULLSWEEP;
     }
 
+    *pidbuf = '\0';
+    if (DTRACE_ENABLED(gc_major_start)
+        || DTRACE_ENABLED(gc_major_end)
+        || DTRACE_ENABLED(gc_minor_start)
+        || DTRACE_ENABLED(gc_minor_end)) {
+        dtrace_proc_str(p, pidbuf);
+    }
+
     /*
      * Test which type of GC to do.
      */
     while (!done) {
 	if ((FLAGS(p) & F_NEED_FULLSWEEP) != 0) {
+	    DTRACE2(gc_major_start, pidbuf, need);
 	    done = major_collection(p, need, objv, nobj, &reclaimed_now);
+	    DTRACE2(gc_major_end, pidbuf, reclaimed_now);
 	} else {
+	    DTRACE2(gc_minor_start, pidbuf, need);
 	    done = minor_collection(p, need, objv, nobj, &reclaimed_now);
+	    DTRACE2(gc_minor_end, pidbuf, reclaimed_now);
 	}
     }
 
@@ -1056,6 +1070,13 @@ do_minor(Process *p, int new_sz, Eterm* 
     sys_memcpy(n_heap + new_sz - n, p->stop, n * sizeof(Eterm));
     p->stop = n_heap + new_sz - n;
 
+    if (HEAP_SIZE(p) != new_sz && DTRACE_ENABLED(process_heap_grow)) {
+        DTRACE_CHARBUF(pidbuf, DTRACE_TERM_BUF_SIZE);
+
+        dtrace_proc_str(p, pidbuf);
+        DTRACE3(process_heap_grow, pidbuf, HEAP_SIZE(p), new_sz);
+    }
+
     ERTS_HEAP_FREE(ERTS_ALC_T_HEAP,
 		   (void*)HEAP_START(p),
 		   HEAP_SIZE(p) * sizeof(Eterm));
@@ -1277,6 +1298,13 @@ major_collection(Process* p, int need, E
     sys_memcpy(n_heap + new_sz - n, p->stop, n * sizeof(Eterm));
     p->stop = n_heap + new_sz - n;
 
+    if (HEAP_SIZE(p) != new_sz && DTRACE_ENABLED(process_heap_grow)) {
+        DTRACE_CHARBUF(pidbuf, DTRACE_TERM_BUF_SIZE);
+
+        dtrace_proc_str(p, pidbuf);
+        DTRACE3(process_heap_grow, pidbuf, HEAP_SIZE(p), new_sz);
+    }
+
     ERTS_HEAP_FREE(ERTS_ALC_T_HEAP,
 		   (void *) HEAP_START(p),
 		   (HEAP_END(p) - HEAP_START(p)) * sizeof(Eterm));
@@ -1947,6 +1975,14 @@ grow_new_heap(Process *p, Uint new_sz, E
         HEAP_TOP(p) = new_heap + heap_size;
         HEAP_START(p) = new_heap;
     }
+
+    if (DTRACE_ENABLED(process_heap_grow)) {
+	DTRACE_CHARBUF(pidbuf, DTRACE_TERM_BUF_SIZE);
+
+        dtrace_proc_str(p, pidbuf);
+	DTRACE3(process_heap_grow, pidbuf, HEAP_SIZE(p), new_sz);
+    }
+
     HEAP_SIZE(p) = new_sz;
 }
 
@@ -1985,6 +2021,14 @@ shrink_new_heap(Process *p, Uint new_sz,
         HEAP_TOP(p) = new_heap + heap_size;
         HEAP_START(p) = new_heap;
     }
+
+    if (DTRACE_ENABLED(process_heap_shrink)) {
+	DTRACE_CHARBUF(pidbuf, DTRACE_TERM_BUF_SIZE);
+
+        dtrace_proc_str(p, pidbuf);
+	DTRACE3(process_heap_shrink, pidbuf, HEAP_SIZE(p), new_sz);
+    }
+
     HEAP_SIZE(p) = new_sz;
 }
 
