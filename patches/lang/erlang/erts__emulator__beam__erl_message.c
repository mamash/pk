$NetBSD$

--- erts/emulator/beam/erl_message.c.orig	2011-10-03 18:12:07.000000000 +0000
+++ erts/emulator/beam/erl_message.c
@@ -31,6 +31,7 @@
 #include "erl_process.h"
 #include "erl_nmgc.h"
 #include "erl_binary.h"
+#include "dtrace-wrapper.h"
 
 ERTS_SCHED_PREF_QUICK_ALLOC_IMPL(message,
 				 ErlMessage,
@@ -335,6 +336,7 @@ erts_queue_dist_message(Process *rcvr,
 			Eterm token)
 {
     ErlMessage* mp;
+    Sint tok_label = 0, tok_lastcnt = 0, tok_serial = 0;
 #ifdef ERTS_SMP
     ErtsProcLocks need_locks;
 #endif
@@ -376,6 +378,19 @@ erts_queue_dist_message(Process *rcvr,
 	message_free(mp);
 	msg = erts_msg_distext2heap(rcvr, rcvr_locks, &mbuf, &token, dist_ext);
 	if (is_value(msg))
+            if (DTRACE_ENABLED(message_queued)) {
+                DTRACE_CHARBUF(receiver_name, DTRACE_TERM_BUF_SIZE);
+
+                dtrace_proc_str(rcvr, receiver_name);
+                if (token != NIL) {
+                    tok_label = signed_val(SEQ_TRACE_T_LABEL(token));
+                    tok_lastcnt = signed_val(SEQ_TRACE_T_LASTCNT(token));
+                    tok_serial = signed_val(SEQ_TRACE_T_SERIAL(token));
+                }
+                DTRACE6(message_queued,
+                        receiver_name, size_object(msg), rcvr->msg.len,
+                        tok_label, tok_lastcnt, tok_serial);
+            }
 	    erts_queue_message(rcvr, rcvr_locks, mbuf, msg, token);
     }
     else {
@@ -385,6 +400,22 @@ erts_queue_dist_message(Process *rcvr,
 	ERL_MESSAGE_TOKEN(mp) = token;
 	mp->next = NULL;
 
+        if (DTRACE_ENABLED(message_queued)) {
+            DTRACE_CHARBUF(receiver_name, DTRACE_TERM_BUF_SIZE);
+
+            dtrace_proc_str(rcvr, receiver_name);
+            if (token != NIL) {
+                tok_label = signed_val(SEQ_TRACE_T_LABEL(token));
+                tok_lastcnt = signed_val(SEQ_TRACE_T_LASTCNT(token));
+                tok_serial = signed_val(SEQ_TRACE_T_SERIAL(token));
+            }
+            /*
+             * TODO: We don't know the real size of the external message here.
+             *       -1 will appear to a D script as 4294967295.
+             */
+            DTRACE6(message_queued, receiver_name, -1, rcvr->msg.len + 1,
+                    tok_label, tok_lastcnt, tok_serial);
+        }
 	mp->data.dist_ext = dist_ext;
 	LINK_MESSAGE(rcvr, mp);
 
@@ -462,12 +493,27 @@ erts_queue_message(Process* receiver,
     LINK_MESSAGE(receiver, mp);
 #endif
 
+    if (DTRACE_ENABLED(message_queued)) {
+        DTRACE_CHARBUF(receiver_name, DTRACE_TERM_BUF_SIZE);
+        Sint tok_label = 0, tok_lastcnt = 0, tok_serial = 0;
+
+        dtrace_proc_str(receiver, receiver_name);
+        if (seq_trace_token != NIL && is_tuple(seq_trace_token)) {
+            tok_label = signed_val(SEQ_TRACE_T_LABEL(seq_trace_token));
+            tok_lastcnt = signed_val(SEQ_TRACE_T_LASTCNT(seq_trace_token));
+            tok_serial = signed_val(SEQ_TRACE_T_SERIAL(seq_trace_token));
+        }
+        DTRACE6(message_queued,
+                receiver_name, size_object(message), receiver->msg.len,
+                tok_label, tok_lastcnt, tok_serial);
+    }
+
     notify_new_message(receiver);
 
     if (IS_TRACED_FL(receiver, F_TRACE_RECEIVE)) {
 	trace_receive(receiver, message);
     }
-    
+
 #ifndef ERTS_SMP
     ERTS_HOLE_CHECK(receiver);
 #endif
@@ -774,11 +820,19 @@ erts_send_message(Process* sender,
     Uint msize;
     ErlHeapFragment* bp = NULL;
     Eterm token = NIL;
+    DTRACE_CHARBUF(sender_name, 64);
+    DTRACE_CHARBUF(receiver_name, 64);
+    Sint tok_label = 0, tok_lastcnt = 0, tok_serial = 0;
 
     BM_STOP_TIMER(system);
     BM_MESSAGE(message,sender,receiver);
     BM_START_TIMER(send);
 
+    *sender_name = *receiver_name = '\0';
+    if (DTRACE_ENABLED(message_send)) {
+        erts_snprintf(sender_name, sizeof(sender_name), "%T", sender->id);
+        erts_snprintf(receiver_name, sizeof(receiver_name), "%T", receiver->id);
+    }
     if (SEQ_TRACE_TOKEN(sender) != NIL && !(flags & ERTS_SND_FLG_NO_SEQ_TRACE)) {
         Eterm* hp;
 
@@ -802,6 +856,16 @@ erts_send_message(Process* sender,
         BM_MESSAGE_COPIED(msize);
         BM_SWAP_TIMER(copy,send);
 
+        if (DTRACE_ENABLED(message_send)) {
+            Eterm token2 = NIL;
+
+            token2 = SEQ_TRACE_TOKEN(sender);
+            tok_label = signed_val(SEQ_TRACE_T_LABEL(token2));
+            tok_lastcnt = signed_val(SEQ_TRACE_T_LASTCNT(token2));
+            tok_serial = signed_val(SEQ_TRACE_T_SERIAL(token2));
+            DTRACE6(message_send, sender_name, receiver_name,
+                    msize, tok_label, tok_lastcnt, tok_serial);
+        }
         erts_queue_message(receiver,
 			   receiver_locks,
 			   bp,
@@ -835,6 +899,8 @@ erts_send_message(Process* sender,
 #endif
         LAZY_COPY(sender,message);
         BM_SWAP_TIMER(copy,send);
+        DTRACE6(message_send, sender_name, receiver_name,
+                size_object(message)msize, tok_label, tok_lastcnt, tok_serial);
         ERL_MESSAGE_TERM(mp) = message;
         ERL_MESSAGE_TOKEN(mp) = NIL;
         mp->next = NULL;
@@ -874,6 +940,8 @@ erts_send_message(Process* sender,
 	{
 	    ErlMessage* mp = message_alloc();
 
+            DTRACE6(message_send, sender_name, receiver_name,
+                    size_object(message), tok_label, tok_lastcnt, tok_serial);
 	    mp->data.attached = NULL;
 	    ERL_MESSAGE_TERM(mp) = message;
 	    ERL_MESSAGE_TOKEN(mp) = NIL;
@@ -908,6 +976,8 @@ erts_send_message(Process* sender,
 	message = copy_struct(message, msize, &hp, ohp);
 	BM_MESSAGE_COPIED(msz);
 	BM_SWAP_TIMER(copy,send);
+        DTRACE6(message_send, sender_name, receiver_name,
+                msize, tok_label, tok_lastcnt, tok_serial);
 	erts_queue_message(receiver, receiver_locks, bp, message, token);
         BM_SWAP_TIMER(send,system);
 #else
@@ -928,6 +998,8 @@ erts_send_message(Process* sender,
 	message = copy_struct(message, msize, &hp, &receiver->off_heap);
 	BM_MESSAGE_COPIED(msize);
         BM_SWAP_TIMER(copy,send);
+        DTRACE6(message_send, sender_name, receiver_name,
+                (uint32_t)msize, tok_label, tok_lastcnt, tok_serial);
 	ERL_MESSAGE_TERM(mp) = message;
 	ERL_MESSAGE_TOKEN(mp) = NIL;
 	mp->next = NULL;
