$NetBSD$

--- erts/emulator/beam/erl_process.c.orig	2011-10-03 18:12:07.000000000 +0000
+++ erts/emulator/beam/erl_process.c
@@ -39,6 +39,7 @@
 #include "erl_binary.h"
 #include "beam_bp.h"
 #include "erl_cpu_topology.h"
+#include "dtrace-wrapper.h"
 
 #define ERTS_RUNQ_CHECK_BALANCE_REDS_PER_SCHED (2000*CONTEXT_REDS)
 #define ERTS_RUNQ_CALL_CHECK_BALANCE_REDS \
@@ -5171,6 +5172,13 @@ Process *schedule(Process *p, int calls)
     int actual_reds;
     int reds;
 
+    if (p != NULL && DTRACE_ENABLED(process_unscheduled)) {
+        DTRACE_CHARBUF(process_buf, DTRACE_TERM_BUF_SIZE);
+
+        dtrace_proc_str(p, process_buf);
+        DTRACE1(process_unscheduled, process_buf);
+    }
+
     if (ERTS_USE_MODIFIED_TIMING()) {
 	context_reds = ERTS_MODIFIED_TIMING_CONTEXT_REDS;
 	input_reductions = ERTS_MODIFIED_TIMING_INPUT_REDS;
@@ -6377,6 +6385,14 @@ erl_create_process(Process* parent, /* P
 
     VERBOSE(DEBUG_PROCESSES, ("Created a new process: %T\n",p->id));
 
+    if (DTRACE_ENABLED(process_spawn)) {
+        DTRACE_CHARBUF(process_name, DTRACE_TERM_BUF_SIZE);
+        DTRACE_CHARBUF(mfa, DTRACE_TERM_BUF_SIZE);
+
+        dtrace_fun_decode(p, mod, func, arity, process_name, mfa);
+        DTRACE2(process_spawn, process_name, mfa);
+    }
+
  error:
 
     erts_smp_proc_unlock(parent, ERTS_PROC_LOCKS_ALL_MINOR);
@@ -6945,6 +6961,17 @@ send_exit_signal(Process *c_p,		/* curre
 
     ASSERT(reason != THE_NON_VALUE);
 
+    if(DTRACE_ENABLED(process_exit_signal) && is_pid(from)) {
+        DTRACE_CHARBUF(sender_str, DTRACE_TERM_BUF_SIZE);
+        DTRACE_CHARBUF(receiver_str, DTRACE_TERM_BUF_SIZE);
+        DTRACE_CHARBUF(reason_buf, DTRACE_TERM_BUF_SIZE);
+
+        dtrace_pid_str(from, sender_str);
+        dtrace_proc_str(rp, receiver_str);
+        erts_snprintf(reason_buf, sizeof(reason_buf) - 1, "%T", reason);
+        DTRACE3(process_exit_signal, sender_str, receiver_str, reason_buf);
+    }
+
     if (ERTS_PROC_IS_TRAPPING_EXITS(rp)
 	&& (reason != am_kill || (flags & ERTS_XSIG_FLG_IGN_KILL))) {
 	if (is_not_nil(token) && token_update)
@@ -7380,7 +7407,16 @@ erts_do_exit_process(Process* p, Eterm r
 
     p->arity = 0;		/* No live registers */
     p->fvalue = reason;
-    
+
+    if (DTRACE_ENABLED(process_exit)) {
+        DTRACE_CHARBUF(process_buf, DTRACE_TERM_BUF_SIZE);
+        DTRACE_CHARBUF(reason_buf, 256);
+
+        dtrace_proc_str(p, process_buf);
+        erts_snprintf(reason_buf, sizeof(reason_buf) - 1, "%T", reason);
+        DTRACE2(process_exit, process_buf, reason_buf);
+    }
+
 #ifdef ERTS_SMP
     ERTS_SMP_CHK_HAVE_ONLY_MAIN_PROC_LOCK(p);
     /* By locking all locks (main lock is already locked) when going
