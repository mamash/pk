$NetBSD$

--- erts/emulator/beam/global.h.orig	2011-10-03 18:12:07.000000000 +0000
+++ erts/emulator/beam/global.h
@@ -1952,4 +1952,66 @@ erts_alloc_message_heap(Uint size,
 #  define UseTmpHeapNoproc(Size) /* Nothing */
 #  define UnUseTmpHeapNoproc(Size) /* Nothing */
 #endif /* HEAP_ON_C_STACK */
+
+#if ERTS_GLB_INLINE_INCL_FUNC_DEF
+
+#include "dtrace-wrapper.h"
+
+ERTS_GLB_INLINE void
+dtrace_pid_str(Eterm pid, char *process_buf)
+{
+    erts_snprintf(process_buf, DTRACE_TERM_BUF_SIZE, "<%lu.%lu.%lu>",
+                  pid_channel_no(pid),
+                  pid_number(pid),
+                  pid_serial(pid));
+}
+
+ERTS_GLB_INLINE void
+dtrace_proc_str(Process *process, char *process_buf)
+{
+    dtrace_pid_str(process->id, process_buf);
+}
+
+ERTS_GLB_INLINE void
+dtrace_port_str(Port *port, char *port_buf)
+{
+    erts_snprintf(port_buf, DTRACE_TERM_BUF_SIZE, "#Port<%lu.%lu>",
+                  port_channel_no(port->id),
+                  port_number(port->id));
+}
+
+ERTS_GLB_INLINE void
+dtrace_fun_decode(Process *process,
+                  Eterm module, Eterm function, int arity,
+                  char *process_buf, char *mfa_buf)
+{
+    char funbuf[DTRACE_TERM_BUF_SIZE];
+    char *funptr = funbuf;
+    char *p = NULL;
+
+    if (process_buf) {
+        dtrace_proc_str(process, process_buf);
+    }
+
+    erts_snprintf(funbuf, sizeof(funbuf), "%T", function);
+    /*
+     * I'm not quite sure how these function names are synthesized,
+     *  but they almost always seem to be in the form of
+     *  '-name/arity-fun-0-' so I'm chopping them up when it's -fun-0-
+     *  (which seems to be the toplevel)
+     */
+    if (funbuf[0] == '\'' && funbuf[1] == '-'
+        && strlen(funbuf) > 3 && funbuf[strlen(funbuf) - 3] == '0') {
+        p = strchr(funbuf, '/');
+        if (p) {
+            *p = 0;
+        }
+        funptr += 2;
+    }
+
+    erts_snprintf(mfa_buf, DTRACE_TERM_BUF_SIZE, "%T:%s/%d",
+                  module, funptr, arity);
+}
+#endif /* #if ERTS_GLB_INLINE_INCL_FUNC_DEF */
+
 #endif /* !__GLOBAL_H__ */
