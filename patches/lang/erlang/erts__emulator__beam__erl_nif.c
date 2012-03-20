$NetBSD$

--- erts/emulator/beam/erl_nif.c.orig	2011-05-24 11:16:43.000000000 +0000
+++ erts/emulator/beam/erl_nif.c
@@ -65,7 +65,8 @@ static void add_readonly_check(ErlNifEnv
 static int is_offheap(const ErlOffHeap* off_heap);
 #endif
 
-
+void dtrace_nifenv_str(ErlNifEnv *, char *);
+ 
 #define MIN_HEAP_FRAG_SZ 200
 static Eterm* alloc_heap_heavy(ErlNifEnv* env, unsigned need, Eterm* hp);
 
@@ -1686,6 +1687,11 @@ void erl_nif_init()
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
