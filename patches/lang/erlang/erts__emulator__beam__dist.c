$NetBSD$

--- erts/emulator/beam/dist.c.orig	2011-10-03 18:12:07.000000000 +0000
+++ erts/emulator/beam/dist.c
@@ -41,6 +41,7 @@
 #include "bif.h"
 #include "external.h"
 #include "erl_binary.h"
+#include "dtrace-wrapper.h"
 
 /* Turn this on to get printouts of all distribution messages
  * which go on the line
@@ -739,6 +740,11 @@ erts_dsig_send_msg(ErtsDSigData *dsdp, E
     Eterm token = NIL;
     Process *sender = dsdp->proc;
     int res;
+    Sint tok_label = 0, tok_lastcnt = 0, tok_serial = 0;
+    Uint msize = 0;
+    DTRACE_CHARBUF(node_name, 64);
+    DTRACE_CHARBUF(sender_name, 64);
+    DTRACE_CHARBUF(receiver_name, 64);
 
     UseTmpHeapNoproc(5);
     if (SEQ_TRACE_TOKEN(sender) != NIL) {
@@ -746,12 +752,28 @@ erts_dsig_send_msg(ErtsDSigData *dsdp, E
 	token = SEQ_TRACE_TOKEN(sender);
 	seq_trace_output(token, message, SEQ_TRACE_SEND, remote, sender);
     }
+    *node_name = *sender_name = *receiver_name = '\0';
+    if (DTRACE_ENABLED(message_send) || DTRACE_ENABLED(message_send_remote)) {
+        erts_snprintf(node_name, sizeof(node_name), "%T", dsdp->dep->sysname);
+        erts_snprintf(sender_name, sizeof(sender_name), "%T", sender->id);
+        erts_snprintf(receiver_name, sizeof(receiver_name), "%T", remote);
+        msize = size_object(message);
+        if (token != NIL) {
+            tok_label = signed_val(SEQ_TRACE_T_LABEL(token));
+            tok_lastcnt = signed_val(SEQ_TRACE_T_LASTCNT(token));
+            tok_serial = signed_val(SEQ_TRACE_T_SERIAL(token));
+        }
+    }
 
     if (token != NIL)
 	ctl = TUPLE4(&ctl_heap[0],
 		     make_small(DOP_SEND_TT), am_Cookie, remote, token);
     else
 	ctl = TUPLE3(&ctl_heap[0], make_small(DOP_SEND), am_Cookie, remote);
+    DTRACE6(message_send, sender_name, receiver_name,
+            msize, tok_label, tok_lastcnt, tok_serial);
+    DTRACE7(message_send_remote, sender_name, node_name, receiver_name,
+            msize, tok_label, tok_lastcnt, tok_serial);
     res = dsig_send(dsdp, ctl, message, 0);
     UnUseTmpHeapNoproc(5);
     return res;
@@ -765,6 +787,11 @@ erts_dsig_send_reg_msg(ErtsDSigData *dsd
     Eterm token = NIL;
     Process *sender = dsdp->proc;
     int res;
+    Sint tok_label = 0, tok_lastcnt = 0, tok_serial = 0;
+    Uint32 msize = 0;
+    DTRACE_CHARBUF(node_name, 64);
+    DTRACE_CHARBUF(sender_name, 64);
+    DTRACE_CHARBUF(receiver_name, 128);
 
     UseTmpHeapNoproc(6);
     if (SEQ_TRACE_TOKEN(sender) != NIL) {
@@ -772,6 +799,19 @@ erts_dsig_send_reg_msg(ErtsDSigData *dsd
 	token = SEQ_TRACE_TOKEN(sender);
 	seq_trace_output(token, message, SEQ_TRACE_SEND, remote_name, sender);
     }
+    *node_name = *sender_name = *receiver_name = '\0';
+    if (DTRACE_ENABLED(message_send) || DTRACE_ENABLED(message_send_remote)) {
+        erts_snprintf(node_name, sizeof(node_name), "%T", dsdp->dep->sysname);
+        erts_snprintf(sender_name, sizeof(sender_name), "%T", sender->id);
+        erts_snprintf(receiver_name, sizeof(receiver_name),
+                      "{%T,%s}", remote_name, node_name);
+        msize = size_object(message);
+        if (token != NIL) {
+            tok_label = signed_val(SEQ_TRACE_T_LABEL(token));
+            tok_lastcnt = signed_val(SEQ_TRACE_T_LASTCNT(token));
+            tok_serial = signed_val(SEQ_TRACE_T_SERIAL(token));
+        }
+    }
 
     if (token != NIL)
 	ctl = TUPLE5(&ctl_heap[0], make_small(DOP_REG_SEND_TT),
@@ -779,6 +819,10 @@ erts_dsig_send_reg_msg(ErtsDSigData *dsd
     else
 	ctl = TUPLE4(&ctl_heap[0], make_small(DOP_REG_SEND),
 		     sender->id, am_Cookie, remote_name);
+    DTRACE6(message_send, sender_name, receiver_name,
+            msize, tok_label, tok_lastcnt, tok_serial);
+    DTRACE7(message_send_remote, sender_name, node_name, receiver_name,
+            msize, tok_label, tok_lastcnt, tok_serial);
     res = dsig_send(dsdp, ctl, message, 0);
     UnUseTmpHeapNoproc(6);
     return res;
@@ -792,6 +836,12 @@ erts_dsig_send_exit_tt(ErtsDSigData *dsd
     Eterm ctl;
     DeclareTmpHeapNoproc(ctl_heap,6);
     int res;
+    Process *sender = dsdp->proc;
+    Sint tok_label = 0, tok_lastcnt = 0, tok_serial = 0;
+    DTRACE_CHARBUF(node_name, 64);
+    DTRACE_CHARBUF(sender_name, 64);
+    DTRACE_CHARBUF(remote_name, 128);
+    DTRACE_CHARBUF(reason_str, 128);
 
     UseTmpHeapNoproc(6);
     if (token != NIL) {	
@@ -802,6 +852,21 @@ erts_dsig_send_exit_tt(ErtsDSigData *dsd
     } else {
 	ctl = TUPLE4(&ctl_heap[0], make_small(DOP_EXIT), local, remote, reason);
     }
+    *node_name = *sender_name = *remote_name = '\0';
+    if (DTRACE_ENABLED(process_exit_signal_remote)) {
+        erts_snprintf(node_name, sizeof(node_name), "%T", dsdp->dep->sysname);
+        erts_snprintf(sender_name, sizeof(sender_name), "%T", sender->id);
+        erts_snprintf(remote_name, sizeof(remote_name),
+                      "{%T,%s}", remote, node_name);
+        erts_snprintf(reason_str, sizeof(reason), "%T", reason);
+        if (token != NIL) {
+            tok_label = signed_val(SEQ_TRACE_T_LABEL(token));
+            tok_lastcnt = signed_val(SEQ_TRACE_T_LASTCNT(token));
+            tok_serial = signed_val(SEQ_TRACE_T_SERIAL(token));
+        }
+    }
+    DTRACE7(process_exit_signal_remote, sender_name, node_name,
+            remote_name, reason_str, tok_label, tok_lastcnt, tok_serial);
     /* forced, i.e ignore busy */
     res = dsig_send(dsdp, ctl, THE_NON_VALUE, 1);
     UnUseTmpHeapNoproc(6);
@@ -1618,6 +1683,16 @@ dsig_send(ErtsDSigData *dsdp, Eterm ctl,
 	    if (!(dep->qflgs & ERTS_DE_QFLG_BUSY)) {
 		if (suspended)
 		    resume = 1; /* was busy when we started, but isn't now */
+                if (resume && DTRACE_ENABLED(dist_port_not_busy)) {
+                    DTRACE_CHARBUF(port_str, 64);
+                    DTRACE_CHARBUF(remote_str, 64);
+
+                    erts_snprintf(port_str, sizeof(port_str), "%T", cid);
+                    erts_snprintf(remote_str, sizeof(remote_str),
+                                  "%T", dep->sysname);
+                    DTRACE3(dist_port_not_busy, erts_this_node_sysname,
+                            port_str, remote_str);
+                }
 	    }
 	    else {
 		/* Enqueue suspended process on dist entry */
@@ -1667,6 +1742,17 @@ dsig_send(ErtsDSigData *dsdp, Eterm ctl,
     }
 
     if (suspended) {
+        if (!resume && DTRACE_ENABLED(dist_port_busy)) {
+            DTRACE_CHARBUF(port_str, 64);
+            DTRACE_CHARBUF(remote_str, 64);
+            DTRACE_CHARBUF(pid_str, 16);
+
+            erts_snprintf(port_str, sizeof(port_str), "%T", cid);
+            erts_snprintf(remote_str, sizeof(remote_str), "%T", dep->sysname);
+            erts_snprintf(pid_str, sizeof(pid_str), "%T", c_p->id);
+            DTRACE4(dist_port_busy, erts_this_node_sysname,
+                    port_str, remote_str, pid_str);
+        }
 	if (!resume && erts_system_monitor_flags.busy_dist_port)
 	    monitor_generic(c_p, am_busy_dist_port, cid);
 	return ERTS_DSIG_SEND_YIELD;
@@ -1690,6 +1776,16 @@ dist_port_command(Port *prt, ErtsDistOut
 		 "(%beu bytes) passed.\n",
 		 size);
 
+    if (DTRACE_ENABLED(dist_output)) {
+        DTRACE_CHARBUF(port_str, 64);
+        DTRACE_CHARBUF(remote_str, 64);
+
+        erts_snprintf(port_str, sizeof(port_str), "%T", prt->id);
+        erts_snprintf(remote_str, sizeof(remote_str),
+                      "%T", prt->dist_entry->sysname);
+        DTRACE4(dist_output, erts_this_node_sysname, port_str,
+                remote_str, size);
+    }
     prt->caller = NIL;
     fpe_was_unmasked = erts_block_fpe();
     (*prt->drv_ptr->output)((ErlDrvData) prt->drv_data,
@@ -1732,6 +1828,16 @@ dist_port_commandv(Port *prt, ErtsDistOu
 
     ASSERT(prt->drv_ptr->outputv);
 
+    if (DTRACE_ENABLED(dist_outputv)) {
+        DTRACE_CHARBUF(port_str, 64);
+        DTRACE_CHARBUF(remote_str, 64);
+
+        erts_snprintf(port_str, sizeof(port_str), "%T", prt->id);
+        erts_snprintf(remote_str, sizeof(remote_str),
+                      "%T", prt->dist_entry->sysname);
+        DTRACE4(dist_outputv, erts_this_node_sysname, port_str,
+                remote_str, size);
+    }
     prt->caller = NIL;
     fpe_was_unmasked = erts_block_fpe();
     (*prt->drv_ptr->outputv)((ErlDrvData) prt->drv_data, &eiov);
@@ -2051,6 +2157,16 @@ erts_dist_command(Port *prt, int reds_li
 void
 erts_dist_port_not_busy(Port *prt)
 {
+    if (DTRACE_ENABLED(dist_port_not_busy)) {
+        DTRACE_CHARBUF(port_str, 64);
+        DTRACE_CHARBUF(remote_str, 64);
+
+        erts_snprintf(port_str, sizeof(port_str), "%T", prt->id);
+        erts_snprintf(remote_str, sizeof(remote_str),
+                      "%T", prt->dist_entry->sysname);
+        DTRACE3(dist_port_not_busy, erts_this_node_sysname,
+                port_str, remote_str);
+    }
     erts_schedule_dist_command(prt, NULL);
 }
 
@@ -2977,6 +3093,19 @@ send_nodes_mon_msgs(Process *c_p, Eterm 
     ASSERT(is_immed(what));
     ASSERT(is_immed(node));
     ASSERT(is_immed(type));
+    if (DTRACE_ENABLED(dist_monitor)) {
+        DTRACE_CHARBUF(what_str, 12);
+        DTRACE_CHARBUF(node_str, 64);
+        DTRACE_CHARBUF(type_str, 12);
+        DTRACE_CHARBUF(reason_str, 64);
+
+        erts_snprintf(what_str, sizeof(what_str), "%T", what);
+        erts_snprintf(node_str, sizeof(node_str), "%T", node);
+        erts_snprintf(type_str, sizeof(type_str), "%T", type);
+        erts_snprintf(reason_str, sizeof(reason_str), "%T", reason);
+        DTRACE5(dist_monitor, erts_this_node_sysname,
+                what_str, node_str, type_str, reason_str);
+    }
 
     ERTS_SMP_LC_ASSERT(!c_p
 		       || (erts_proc_lc_my_proc_locks(c_p)
