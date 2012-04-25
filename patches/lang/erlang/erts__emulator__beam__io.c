$NetBSD$

--- erts/emulator/beam/io.c.orig	2011-10-03 18:12:07.000000000 +0000
+++ erts/emulator/beam/io.c
@@ -42,6 +42,7 @@
 #include "erl_bits.h"
 #include "erl_version.h"
 #include "error.h"
+#include "dtrace-wrapper.h"
 
 extern ErlDrvEntry fd_driver_entry;
 extern ErlDrvEntry vanilla_driver_entry;
@@ -179,6 +180,19 @@ typedef struct line_buf_context {
 
 #define LINEBUF_INITIAL 100
 
+#define DTRACE_FORMAT_COMMON_PID_AND_PORT(PID, PORT)         \
+    DTRACE_CHARBUF(process_str, DTRACE_TERM_BUF_SIZE);       \
+    DTRACE_CHARBUF(port_str, DTRACE_TERM_BUF_SIZE);          \
+                                                             \
+    dtrace_pid_str((PID), process_str);                      \
+    dtrace_port_str((PORT), port_str);
+#define DTRACE_FORMAT_COMMON_PROC_AND_PORT(PID, PORT)        \
+    DTRACE_CHARBUF(process_str, DTRACE_TERM_BUF_SIZE);       \
+    DTRACE_CHARBUF(port_str, DTRACE_TERM_BUF_SIZE);          \
+                                                             \
+    dtrace_proc_str((PID), process_str);                     \
+    dtrace_port_str((PORT), port_str);
+    
 
 /* The 'number' field in a port now has two parts: the lowest bits
    contain the index in the port table, and the higher bits are a counter
@@ -638,6 +652,10 @@ erts_open_driver(erts_driver_t* driver,	
 	    trace_sched_ports_where(port, am_in, am_start);
 	}
 	port->caller = pid;
+        if (DTRACE_ENABLED(driver_start)) {
+            DTRACE_FORMAT_COMMON_PID_AND_PORT(pid, port)
+            DTRACE3(driver_start, process_str, driver->name, port_str);
+        }
 	fpe_was_unmasked = erts_block_fpe();
 	drv_data = (*driver->start)((ErlDrvPort)(port_ix),
 				    name, opts);
@@ -1155,6 +1173,10 @@ int erts_write_to_port(Eterm caller_id, 
 	ev.size = size;  /* total size */
 	ev.iov = ivp;
 	ev.binv = bvp;
+        if (DTRACE_ENABLED(driver_outputv)) {
+            DTRACE_FORMAT_COMMON_PID_AND_PORT(caller_id, p)
+            DTRACE4(driver_outputv, process_str, port_str, p->name, size);
+        }
 	fpe_was_unmasked = erts_block_fpe();
 	(*drv->outputv)((ErlDrvData)p->drv_data, &ev);
 	erts_unblock_fpe(fpe_was_unmasked);
@@ -1174,8 +1196,17 @@ int erts_write_to_port(Eterm caller_id, 
 	buf = erts_alloc(ERTS_ALC_T_TMP, size+1);
 	r = io_list_to_buf(list, buf, size);
 
+	if(DTRACE_ENABLED(port_command)) {
+            DTRACE_FORMAT_COMMON_PID_AND_PORT(caller_id, p)
+	    DTRACE4(port_command, process_str, port_str, p->name, "command");
+	}
+
 	if (r >= 0) {
 	    size -= r;
+            if (DTRACE_ENABLED(driver_output)) {
+                DTRACE_FORMAT_COMMON_PID_AND_PORT(caller_id, p)
+                DTRACE4(driver_output, process_str, port_str, p->name, size);
+            }
 	    fpe_was_unmasked = erts_block_fpe();
 	    (*drv->output)((ErlDrvData)p->drv_data, buf, size);
 	    erts_unblock_fpe(fpe_was_unmasked);
@@ -1199,6 +1230,10 @@ int erts_write_to_port(Eterm caller_id, 
 	     */
 	    buf = erts_alloc(ERTS_ALC_T_TMP, size+1); 
 	    r = io_list_to_buf(list, buf, size);
+            if (DTRACE_ENABLED(driver_output)) {
+                DTRACE_FORMAT_COMMON_PID_AND_PORT(caller_id, p)
+                DTRACE4(driver_output, process_str, port_str, p->name, size);
+            }
 	    fpe_was_unmasked = erts_block_fpe();
 	    (*drv->output)((ErlDrvData)p->drv_data, buf, size);
 	    erts_unblock_fpe(fpe_was_unmasked);
@@ -1794,6 +1829,10 @@ static void flush_port(Port *p)
     ERTS_SMP_LC_ASSERT(erts_lc_is_port_locked(p));
 
     if (p->drv_ptr->flush != NULL) {
+        if (DTRACE_ENABLED(driver_flush)) {
+            DTRACE_FORMAT_COMMON_PID_AND_PORT(p->connected, p)
+            DTRACE3(driver_flush, process_str, port_str, p->name);
+        }
         if (IS_TRACED_FL(p, F_TRACE_SCHED_PORTS)) {
 	    trace_sched_ports_where(p, am_in, am_flush);
 	}
@@ -1846,6 +1885,10 @@ terminate_port(Port *prt)
     drv = prt->drv_ptr;
     if ((drv != NULL) && (drv->stop != NULL)) {
 	int fpe_was_unmasked = erts_block_fpe();
+        if (DTRACE_ENABLED(driver_stop)) {
+            DTRACE_FORMAT_COMMON_PID_AND_PORT(prt->connected, prt)
+            DTRACE3(driver_stop, process_str, drv->name, port_str);
+        }
 	(*drv->stop)((ErlDrvData)prt->drv_data);
 	erts_unblock_fpe(fpe_was_unmasked);
 #ifdef ERTS_SMP
@@ -2003,6 +2046,17 @@ erts_do_exit_port(Port *p, Eterm from, E
 
    rreason = (reason == am_kill) ? am_killed : reason;
 
+   if (DTRACE_ENABLED(port_exit)) {
+       DTRACE_CHARBUF(from_str, DTRACE_TERM_BUF_SIZE);
+       DTRACE_CHARBUF(port_str, DTRACE_TERM_BUF_SIZE);
+       DTRACE_CHARBUF(rreason_str, 64);
+
+       erts_snprintf(from_str, sizeof(from_str), "%T", from);
+       dtrace_port_str(p, port_str);
+       erts_snprintf(rreason_str, sizeof(rreason_str), "%T", rreason);
+       DTRACE4(port_exit, from_str, port_str, p->name, rreason_str);
+   }
+
    if ((p->status & (ERTS_PORT_SFLGS_DEAD
 		     | ERTS_PORT_SFLG_EXITING
 		     | ERTS_PORT_SFLG_IMMORTAL))
@@ -2103,6 +2157,11 @@ void erts_port_command(Process *proc,
 	    if (tp[2] == am_close) {
 		erts_port_status_bor_set(port, ERTS_PORT_SFLG_SEND_CLOSED);
 		erts_do_exit_port(port, pid, am_normal);
+
+		if(DTRACE_ENABLED(port_command)) {
+                    DTRACE_FORMAT_COMMON_PROC_AND_PORT(proc, port)
+		    DTRACE4(port_command, process_str, port_str, port->name, "close");
+		}
 		goto done;
 	    } else if (is_tuple_arity(tp[2], 2)) {
 		tp = tuple_val(tp[2]);
@@ -2110,6 +2169,10 @@ void erts_port_command(Process *proc,
 		    if (erts_write_to_port(caller_id, port, tp[2]) == 0)
 			goto done;
 		} else if ((tp[1] == am_connect) && is_internal_pid(tp[2])) {
+		    if(DTRACE_ENABLED(port_command)) {
+                        DTRACE_FORMAT_COMMON_PROC_AND_PORT(proc, port)
+			DTRACE4(port_command, process_str, port_str, port->name, "connect");
+		    }
 		    port->connected = tp[2];
 		    deliver_result(port->id, pid, am_connected);
 		    goto done;
@@ -2211,6 +2274,13 @@ erts_port_control(Process* p, Port* prt,
     erts_smp_proc_unlock(p, ERTS_PROC_LOCK_MAIN);
     ERTS_SMP_CHK_NO_PROC_LOCKS;
 
+    if (DTRACE_ENABLED(port_control) || DTRACE_ENABLED(driver_control)) {
+        DTRACE_FORMAT_COMMON_PROC_AND_PORT(p, prt);
+        DTRACE4(port_control, process_str, port_str, prt->name, command);
+        DTRACE5(driver_control, process_str, port_str, prt->name,
+                command, to_len);
+    }
+
     /*
      * Call the port's control routine.
      */
@@ -2351,6 +2421,8 @@ print_port_info(int to, void *arg, int i
 void
 set_busy_port(ErlDrvPort port_num, int on)
 {
+    DTRACE_CHARBUF(port_str, 16);
+
     ERTS_SMP_CHK_NO_PROC_LOCKS;
 
     ERTS_SMP_LC_ASSERT(erts_lc_is_port_locked(&erts_port[port_num]));
@@ -2358,12 +2430,22 @@ set_busy_port(ErlDrvPort port_num, int o
     if (on) {
         erts_port_status_bor_set(&erts_port[port_num],
 				 ERTS_PORT_SFLG_PORT_BUSY);
+        if (DTRACE_ENABLED(port_busy)) {
+            erts_snprintf(port_str, sizeof(port_str),
+                          "%T", erts_port[port_num].id);
+            DTRACE1(port_busy, port_str);
+        }
     } else {
         ErtsProcList* plp = erts_port[port_num].suspended;
         erts_port_status_band_set(&erts_port[port_num],
 				  ~ERTS_PORT_SFLG_PORT_BUSY);
         erts_port[port_num].suspended = NULL;
 
+        if (DTRACE_ENABLED(port_not_busy)) {
+            erts_snprintf(port_str, sizeof(port_str),
+                          "%T", erts_port[port_num].id);
+            DTRACE1(port_not_busy, port_str);
+        }
 	if (erts_port[port_num].dist_entry) {
 	    /*
 	     * Processes suspended on distribution ports are
@@ -2381,6 +2463,26 @@ set_busy_port(ErlDrvPort port_num, int o
 	 */
 
         if (plp) {
+            /*
+             * Hrm, for blocked dist ports, plp always seems to be NULL.
+             * That's not so fun.
+             * Well, another way to get the same info is using a D
+             * script to correlate an earlier process-port_blocked+pid
+             * event with a later process-scheduled event.  That's
+             * subject to the multi-CPU races with how events are
+             * handled, but hey, that way works most of the time.
+             */
+            if (DTRACE_ENABLED(process_port_unblocked)) {
+                DTRACE_CHARBUF(pid_str, 16);
+                ErtsProcList* plp2 = plp;
+
+                erts_snprintf(port_str, sizeof(port_str),
+                             "%T", erts_port[port_num]);
+                while (plp2 != NULL) {
+                    erts_snprintf(pid_str, sizeof(pid_str), "%T", plp2->pid);
+                    DTRACE2(process_port_unblocked, pid_str, port_str);
+                }
+            }
             /* First proc should be resumed last */
 	    if (plp->next) {
 		erts_resume_processes(plp->next);
@@ -2427,6 +2529,12 @@ void erts_raw_port_command(Port* p, byte
 		 p->drv_ptr->name ? p->drv_ptr->name : "unknown");
 
     p->caller = NIL;
+    if (DTRACE_ENABLED(driver_output)) {
+        DTRACE_CHARBUF(port_str, DTRACE_TERM_BUF_SIZE);
+
+        dtrace_port_str(p, port_str);
+        DTRACE4(driver_output, "-raw-", port_str, p->name, len);
+    }
     fpe_was_unmasked = erts_block_fpe();
     (*p->drv_ptr->output)((ErlDrvData)p->drv_data, (char*) buf, (int) len);
     erts_unblock_fpe(fpe_was_unmasked);
@@ -2442,6 +2550,10 @@ int async_ready(Port *p, void* data)
 	ERTS_SMP_LC_ASSERT(erts_lc_is_port_locked(p));
 	ASSERT(!(p->status & ERTS_PORT_SFLGS_DEAD));
 	if (p->drv_ptr->ready_async != NULL) {
+            if (DTRACE_ENABLED(driver_ready_async)) {
+                DTRACE_FORMAT_COMMON_PID_AND_PORT(p->connected, p)
+                DTRACE3(driver_ready_async, process_str, port_str, p->name);
+            }
 	    (*p->drv_ptr->ready_async)((ErlDrvData)p->drv_data, data);
 	    need_free = 0;
 #ifdef ERTS_SMP
@@ -4423,6 +4535,10 @@ void erts_fire_port_monitor(Port *prt, E
     ASSERT(callback != NULL);
     ref_to_driver_monitor(ref,&drv_monitor);
     DRV_MONITOR_UNLOCK_PDL(prt);
+    if (DTRACE_ENABLED(driver_process_exit)) {
+        DTRACE_FORMAT_COMMON_PID_AND_PORT(prt->connected, prt)
+        DTRACE3(driver_process_exit, process_str, port_str, prt->name);
+    }
     fpe_was_unmasked = erts_block_fpe();
     (*callback)((ErlDrvData) (prt->drv_data), &drv_monitor);
     erts_unblock_fpe(fpe_was_unmasked);
@@ -4871,6 +4987,8 @@ init_driver(erts_driver_t *drv, ErlDrvEn
     else {
 	int res;
 	int fpe_was_unmasked = erts_block_fpe();
+        DTRACE4(driver_init, drv->name, drv->version.major, drv->version.minor,
+                drv->flags);
 	res = (*de->init)();
 	erts_unblock_fpe(fpe_was_unmasked);
 	return res;
