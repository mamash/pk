$NetBSD$

--- erts/emulator/beam/beam_emu.c.orig	2011-10-03 18:12:07.000000000 +0000
+++ erts/emulator/beam/beam_emu.c
@@ -40,6 +40,7 @@
 #include "hipe_mode_switch.h"
 #include "hipe_bif1.h"
 #endif
+#include "dtrace-wrapper.h"
 
 /* #define HARDDEBUG 1 */
 
@@ -1080,6 +1081,87 @@ init_emulator(void)
 #  define REG_tmp_arg2
 #endif
 
+#ifdef HAVE_DTRACE
+
+#define DTRACE_CALL(p, m, f, a)                                 \
+    if (DTRACE_ENABLED(function_entry)) {                       \
+        DTRACE_CHARBUF(process_name, DTRACE_TERM_BUF_SIZE);     \
+        DTRACE_CHARBUF(mfa, DTRACE_TERM_BUF_SIZE);              \
+        int depth = (STACK_START(p) - STACK_TOP(p))             \
+            / sizeof(Eterm*);                                   \
+        dtrace_fun_decode(p, m, f, a,                           \
+                          process_name, mfa);                   \
+        DTRACE3(function_entry, process_name, mfa, depth);      \
+    }
+
+#define DTRACE_RETURN(p, m, f, a)                               \
+    if (DTRACE_ENABLED(function_return)) {                      \
+        DTRACE_CHARBUF(process_name, DTRACE_TERM_BUF_SIZE);     \
+        DTRACE_CHARBUF(mfa, DTRACE_TERM_BUF_SIZE);              \
+        int depth = (STACK_START(p) - STACK_TOP(p))             \
+            / sizeof(Eterm*);                                   \
+        dtrace_fun_decode(p, m, f, a,                           \
+                          process_name, mfa);                   \
+        DTRACE3(function_return, process_name, mfa, depth);     \
+    }
+
+#define DTRACE_BIF_ENTRY(p, m, f, a)                            \
+    if (DTRACE_ENABLED(bif_entry)) {                            \
+        DTRACE_CHARBUF(process_name, DTRACE_TERM_BUF_SIZE);     \
+        DTRACE_CHARBUF(mfa, DTRACE_TERM_BUF_SIZE);              \
+        dtrace_fun_decode(p, m, f, a,                           \
+                          process_name, mfa);                   \
+        DTRACE2(bif_entry, process_name, mfa);                  \
+    }
+
+#define DTRACE_BIF_RETURN(p, m, f, a)                           \
+    if (DTRACE_ENABLED(bif_return)) {                           \
+        DTRACE_CHARBUF(process_name, DTRACE_TERM_BUF_SIZE);     \
+        DTRACE_CHARBUF(mfa, DTRACE_TERM_BUF_SIZE);              \
+        dtrace_fun_decode(p, m, f, a,                           \
+                          process_name, mfa);                   \
+        DTRACE2(bif_return, process_name, mfa);                 \
+    }
+
+#define DTRACE_NIF_ENTRY(p, m, f, a)                            \
+    if (DTRACE_ENABLED(nif_entry)) {                            \
+        DTRACE_CHARBUF(process_name, DTRACE_TERM_BUF_SIZE);     \
+        DTRACE_CHARBUF(mfa, DTRACE_TERM_BUF_SIZE);              \
+        dtrace_fun_decode(p, m, f, a,                           \
+                          process_name, mfa);                   \
+        DTRACE2(nif_entry, process_name, mfa);                  \
+    }
+
+#define DTRACE_NIF_RETURN(p, m, f, a)                           \
+    if (DTRACE_ENABLED(nif_return)) {                           \
+        DTRACE_CHARBUF(process_name, DTRACE_TERM_BUF_SIZE);     \
+        DTRACE_CHARBUF(mfa, DTRACE_TERM_BUF_SIZE);              \
+        dtrace_fun_decode(p, m, f, a,                           \
+                          process_name, mfa);                   \
+        DTRACE2(nif_return, process_name, mfa);                 \
+    }
+
+#else /* HAVE_DTRACE */
+
+#define DTRACE_CALL(p, m, f, a)       do {} while (0)
+#define DTRACE_RETURN(p, m, f, a)     do {} while (0)
+#define DTRACE_BIF_ENTRY(p, m, f, a)  do {} while (0)
+#define DTRACE_BIF_RETURN(p, m, f, a) do {} while (0)
+#define DTRACE_NIF_ENTRY(p, m, f, a)  do {} while (0)
+#define DTRACE_NIF_RETURN(p, m, f, a) do {} while (0)
+
+#endif /* HAVE_DTRACE */
+
+void
+dtrace_drvport_str(ErlDrvPort drvport, char *port_buf)
+{
+    Port *port = erts_drvport2port(drvport);
+
+    erts_snprintf(port_buf, DTRACE_TERM_BUF_SIZE, "#Port<%lu.%lu>",
+                  port_channel_no(port->id),
+                  port_number(port->id));
+}
+
 /*
  * process_main() is called twice:
  * The first call performs some initialisation, including exporting
@@ -1276,6 +1358,29 @@ void process_main(void)
 #endif
 	SWAPIN;
 	ASSERT(VALID_INSTR(next));
+
+        if (DTRACE_ENABLED(process_scheduled)) {
+            DTRACE_CHARBUF(process_buf, DTRACE_TERM_BUF_SIZE);
+            DTRACE_CHARBUF(fun_buf, DTRACE_TERM_BUF_SIZE);
+            dtrace_proc_str(c_p, process_buf);
+
+            if (ERTS_PROC_IS_EXITING(c_p)) {
+                strcpy(fun_buf, "<exiting>");
+            } else {
+                BeamInstr *fptr = find_function_from_pc(c_p->i);
+                if (fptr) {
+                    dtrace_fun_decode(c_p, (Eterm)fptr[0],
+                                      (Eterm)fptr[1], (Uint)fptr[2],
+                                      NULL, fun_buf);
+                } else {
+                    erts_snprintf(fun_buf, sizeof(fun_buf),
+                                  "<unknown/%p>", next);
+                }
+            }
+
+            DTRACE2(process_scheduled, process_buf, fun_buf);
+        }
+
 	Goto(next);
     }
 
@@ -1541,7 +1646,13 @@ void process_main(void)
 
 
  OpCase(return): {
+    BeamInstr* fptr;
     SET_I(c_p->cp);
+
+    if (DTRACE_ENABLED(function_return) && (fptr = find_function_from_pc(c_p->cp))) {
+        DTRACE_RETURN(c_p, (Eterm)fptr[0], (Eterm)fptr[1], (Uint)fptr[2]);
+    }
+
     /*
      * We must clear the CP to make sure that a stale value do not
      * create a false module dependcy preventing code upgrading.
@@ -1803,6 +1914,7 @@ void process_main(void)
 	  * remove it...
 	  */
 	 ASSERT(!msgp->data.attached);
+         /* TODO: Add DTrace probe for this bad message situation? */
 	 UNLINK_MESSAGE(c_p, msgp);
 	 free_message(msgp);
 	 goto loop_rec__;
@@ -1846,6 +1958,22 @@ void process_main(void)
 	 seq_trace_output(SEQ_TRACE_TOKEN(c_p), msg, SEQ_TRACE_RECEIVE, 
 			  c_p->id, c_p);
      }
+     if (DTRACE_ENABLED(message_receive)) {
+         Eterm token2 = NIL;
+         DTRACE_CHARBUF(receiver_name, DTRACE_TERM_BUF_SIZE);
+         Sint tok_label = 0, tok_lastcnt = 0, tok_serial = 0;
+
+         dtrace_proc_str(c_p, receiver_name);
+         token2 = SEQ_TRACE_TOKEN(c_p);
+         if (token2 != NIL) {
+             tok_label = signed_val(SEQ_TRACE_T_LABEL(token2));
+             tok_lastcnt = signed_val(SEQ_TRACE_T_LASTCNT(token2));
+             tok_serial = signed_val(SEQ_TRACE_T_SERIAL(token2));
+         }
+         DTRACE6(message_receive,
+                 receiver_name, size_object(ERL_MESSAGE_TERM(msgp)),
+                 c_p->msg.len - 1, tok_label, tok_lastcnt, tok_serial);
+     }
      UNLINK_MESSAGE(c_p, msgp);
      JOIN_MESSAGE(c_p);
      CANCEL_TIMER(c_p);
@@ -3303,6 +3431,8 @@ void process_main(void)
 	     */
 	    BifFunction vbf;
 
+	    DTRACE_NIF_ENTRY(c_p, (Eterm)I[-3], (Eterm)I[-2], (Uint)I[-1]);
+
 	    c_p->current = I-3; /* current and vbf set to please handle_error */ 
 	    SWAPOUT;
 	    c_p->fcalls = FCALLS - 1;
@@ -3324,6 +3454,9 @@ void process_main(void)
 	    ASSERT(!ERTS_PROC_IS_EXITING(c_p) || is_non_value(nif_bif_result));
 	    PROCESS_MAIN_CHK_LOCKS(c_p);
 	    ERTS_VERIFY_UNUSED_TEMP_ALLOC(c_p);
+
+	    DTRACE_NIF_RETURN(c_p, (Eterm)I[-3], (Eterm)I[-2], (Uint)I[-1]);
+
 	    goto apply_bif_or_nif_epilogue;
 	 
 	OpCase(apply_bif):
@@ -3343,6 +3476,8 @@ void process_main(void)
 	    c_p->arity = 0;		/* To allow garbage collection on ourselves
 					 * (check_process_code/2).
 					 */
+	    DTRACE_BIF_ENTRY(c_p, (Eterm)I[-3], (Eterm)I[-2], (Uint)I[-1]);
+
 	    SWAPOUT;
 	    c_p->fcalls = FCALLS - 1;
 	    vbf = (BifFunction) Arg(0);
@@ -3401,6 +3536,8 @@ void process_main(void)
 			 bif_nif_arity);
 	    }
 
+	    DTRACE_BIF_RETURN(c_p, (Eterm)I[-3], (Eterm)I[-2], (Uint)I[-1]);
+
 	apply_bif_or_nif_epilogue:
 	    ERTS_SMP_REQ_PROC_MAIN_LOCK(c_p);
 	    ERTS_HOLE_CHECK(c_p);
@@ -6154,6 +6291,13 @@ apply(Process* p, Eterm module, Eterm fu
 	save_calls(p, ep);
     }
 
+    if (DTRACE_ENABLED(function_entry) && ep->address) {
+        BeamInstr *fptr = find_function_from_pc(ep->address);
+        if (fptr) {
+            DTRACE_CALL(p, (Eterm)fptr[0], (Eterm)fptr[1], (Uint)fptr[2]);
+        }
+    }
+
     return ep->address;
 }
 
@@ -6203,6 +6347,13 @@ fixed_apply(Process* p, Eterm* reg, Uint
 	save_calls(p, ep);
     }
 
+    if (DTRACE_ENABLED(function_entry)) {
+        BeamInstr *fptr = find_function_from_pc(ep->address);
+        if (fptr) {
+            DTRACE_CALL(p, (Eterm)fptr[0], (Eterm)fptr[1], (Uint)fptr[2]);
+        }
+    }
+
     return ep->address;
 }
 
@@ -6252,6 +6403,14 @@ erts_hibernate(Process* c_p, Eterm modul
 	c_p->max_arg_reg = sizeof(c_p->def_arg_reg)/sizeof(c_p->def_arg_reg[0]);
     }
 
+    if (DTRACE_ENABLED(process_hibernate)) {
+        DTRACE_CHARBUF(process_name, DTRACE_TERM_BUF_SIZE);
+        DTRACE_CHARBUF(mfa, DTRACE_TERM_BUF_SIZE);
+        dtrace_fun_decode(c_p, module, function, arity,
+                          process_name, mfa);
+        DTRACE2(process_hibernate, process_name, mfa);
+    }
+
     /*
      * Arrange for the process to be resumed at the given MFA with
      * the stack cleared.
@@ -6326,6 +6485,14 @@ call_fun(Process* p,		/* Current process
 	code_ptr = fe->address;
 	actual_arity = (int) code_ptr[-1];
 
+	if (DTRACE_ENABLED(function_entry)) {
+	    BeamInstr *fptr = find_function_from_pc(code_ptr);
+
+	    if (fptr) {
+		DTRACE_CALL(p, fe->module, (Eterm)fptr[1], actual_arity);
+	    }
+	}
+
 	if (actual_arity == arity+num_free) {
 	    if (num_free == 0) {
 		return code_ptr;
@@ -6344,7 +6511,7 @@ call_fun(Process* p,		/* Current process
 	} else {
 	    /*
 	     * Something wrong here. First build a list of the arguments.
-	     */  
+	     */
 
 	    if (is_non_value(args)) {
 		Uint sz = 2 * arity;
@@ -6419,6 +6586,7 @@ call_fun(Process* p,		/* Current process
 	actual_arity = (int) ep->code[2];
 
 	if (arity == actual_arity) {
+	    DTRACE_CALL(p, ep->code[0], ep->code[1], (Uint)ep->code[2]);
 	    return ep->address;
 	} else {
 	    /*
@@ -6474,6 +6642,7 @@ call_fun(Process* p,		/* Current process
 	    reg[1] = function;
 	    reg[2] = args;
 	}
+	DTRACE_CALL(p, module, function, arity);
 	return ep->address;
     } else {
     badfun:
