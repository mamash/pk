$NetBSD$

--- erts/emulator/beam/erl_nif.c.orig	2011-10-03 18:12:07.000000000 +0000
+++ erts/emulator/beam/erl_nif.c
@@ -32,6 +32,7 @@
 #include "error.h"
 #include "big.h"
 #include "beam_bp.h"
+#include "dtrace-wrapper.h"
 
 #include <limits.h>
 #include <stddef.h> /* offsetof */
@@ -65,7 +66,8 @@ static void add_readonly_check(ErlNifEnv
 static int is_offheap(const ErlOffHeap* off_heap);
 #endif
 
-
+void dtrace_nifenv_str(ErlNifEnv *, char *);
+ 
 #define MIN_HEAP_FRAG_SZ 200
 static Eterm* alloc_heap_heavy(ErlNifEnv* env, unsigned need, Eterm* hp);
 
@@ -1700,6 +1702,11 @@ void erl_nif_init()
     resource_type_list.name = THE_NON_VALUE;
 }
 
+void dtrace_nifenv_str(ErlNifEnv *env, char *process_buf)
+{
+    dtrace_pid_str(env->proc->id, process_buf);
+}
+
 #ifdef READONLY_CHECK
 /* Use checksums to assert that NIFs do not write into inspected binaries
 */
@@ -1746,3 +1753,79 @@ static unsigned calc_checksum(unsigned c
 
 #endif /* READONLY_CHECK */
 
+#ifdef HAVE_DTRACE
+
+#define MESSAGE_BUFSIZ 1024
+
+static void get_string_maybe(ErlNifEnv *env, const ERL_NIF_TERM term,
+                             char **ptr, char *buf, int bufsiz)
+{
+    ErlNifBinary str_bin;
+
+    if (!enif_inspect_iolist_as_binary(env, term, &str_bin) ||
+        str_bin.size > bufsiz) {
+        *ptr = NULL;
+    } else {
+        memcpy(buf, (char *) str_bin.data, str_bin.size);
+        buf[str_bin.size] = '\0';
+        *ptr = buf;
+    }
+}
+
+ERL_NIF_TERM erl_nif_user_trace_s1(ErlNifEnv* env, int argc,
+                                   const ERL_NIF_TERM argv[])
+{
+    ErlNifBinary message_bin;
+    DTRACE_CHARBUF(messagebuf, MESSAGE_BUFSIZ + 1);
+
+    if (DTRACE_ENABLED(user_trace_s1)) {
+	if (!enif_inspect_iolist_as_binary(env, argv[0], &message_bin) ||
+	    message_bin.size > MESSAGE_BUFSIZ) {
+	    return am_badarg;
+	}
+	memcpy(messagebuf, (char *) message_bin.data, message_bin.size);
+        messagebuf[message_bin.size] = '\0';
+	DTRACE1(user_trace_s1, messagebuf);
+	return am_true;
+    } else {
+	return am_false;
+    }
+}
+
+ERL_NIF_TERM erl_nif_user_trace_i4s4(ErlNifEnv* env, int argc,
+                                     const ERL_NIF_TERM argv[])
+{
+    DTRACE_CHARBUF(procbuf, 32 + 1);
+    DTRACE_CHARBUF(user_tagbuf, MESSAGE_BUFSIZ + 1);
+    char *utbuf = NULL;
+    ErlNifSInt64 i1, i2, i3, i4;
+    DTRACE_CHARBUF(messagebuf1, MESSAGE_BUFSIZ + 1);
+    DTRACE_CHARBUF(messagebuf2, MESSAGE_BUFSIZ + 1);
+    DTRACE_CHARBUF(messagebuf3, MESSAGE_BUFSIZ + 1);
+    DTRACE_CHARBUF(messagebuf4, MESSAGE_BUFSIZ + 1);
+    char *mbuf1 = NULL, *mbuf2 = NULL, *mbuf3 = NULL, *mbuf4 = NULL;
+
+    if (DTRACE_ENABLED(user_trace_i4s4)) {
+	dtrace_nifenv_str(env, procbuf);
+        get_string_maybe(env, argv[0], &utbuf, user_tagbuf, MESSAGE_BUFSIZ);
+        if (! enif_get_int64(env, argv[1], &i1))
+            i1 = 0;
+        if (! enif_get_int64(env, argv[2], &i2))
+            i2 = 0;
+        if (! enif_get_int64(env, argv[3], &i3))
+            i3 = 0;
+        if (! enif_get_int64(env, argv[4], &i4))
+            i4 = 0;
+        get_string_maybe(env, argv[5], &mbuf1, messagebuf1, MESSAGE_BUFSIZ);
+        get_string_maybe(env, argv[6], &mbuf2, messagebuf2, MESSAGE_BUFSIZ);
+        get_string_maybe(env, argv[7], &mbuf3, messagebuf3, MESSAGE_BUFSIZ);
+        get_string_maybe(env, argv[8], &mbuf4, messagebuf4, MESSAGE_BUFSIZ);
+	DTRACE10(user_trace_i4s4, procbuf, utbuf,
+		 i1, i2, i3, i4, mbuf1, mbuf2, mbuf3, mbuf4);
+	return am_true;
+    } else {
+	return am_false;
+    }
+}
+
+#endif /* HAVE_DTRACE */
