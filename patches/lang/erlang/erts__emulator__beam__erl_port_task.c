$NetBSD$

--- erts/emulator/beam/erl_port_task.c.orig	2011-05-24 11:16:43.000000000 +0000
+++ erts/emulator/beam/erl_port_task.c
@@ -32,6 +32,7 @@
 #include "global.h"
 #include "erl_port_task.h"
 #include "dist.h"
+#include "dtrace-wrapper.h"
 
 #if defined(DEBUG) && 0
 #define HARD_DEBUG
@@ -61,6 +62,16 @@ do {					\
     (P)->sched.next = NULL;		\
 } while (0)
 
+#define DTRACE_DRIVER(PROBE_NAME, PP)                              \
+    if (DTRACE_ENABLED(driver_ready_input)) {                      \
+        DTRACE_CHARBUF(process_str, DTRACE_TERM_BUF_SIZE);         \
+        DTRACE_CHARBUF(port_str, DTRACE_TERM_BUF_SIZE);            \
+                                                                   \
+        dtrace_pid_str(PP->connected, process_str);                \
+        dtrace_port_str(PP, port_str);                             \
+        DTRACE3(PROBE_NAME, process_str, port_str, PP->name);      \
+    }
+
 erts_smp_atomic_t erts_port_task_outstanding_io_tasks;
 
 struct ErtsPortTaskQueue_ {
@@ -846,12 +857,15 @@ erts_port_task_execute(ErtsRunQueue *run
 	    goto tasks_done;
 	case ERTS_PORT_TASK_TIMEOUT:
 	    reds += ERTS_PORT_REDS_TIMEOUT;
-	    if (!(pp->status & ERTS_PORT_SFLGS_DEAD))
+	    if (!(pp->status & ERTS_PORT_SFLGS_DEAD)) {
+                DTRACE_DRIVER(driver_timeout, pp);
 		(*pp->drv_ptr->timeout)((ErlDrvData) pp->drv_data);
+            }
 	    break;
 	case ERTS_PORT_TASK_INPUT:
 	    reds += ERTS_PORT_REDS_INPUT;
 	    ASSERT((pp->status & ERTS_PORT_SFLGS_DEAD) == 0);
+            DTRACE_DRIVER(driver_ready_input, pp);
 	    /* NOTE some windows drivers use ->ready_input for input and output */
 	    (*pp->drv_ptr->ready_input)((ErlDrvData) pp->drv_data, ptp->event);
 	    io_tasks_executed++;
@@ -859,12 +873,14 @@ erts_port_task_execute(ErtsRunQueue *run
 	case ERTS_PORT_TASK_OUTPUT:
 	    reds += ERTS_PORT_REDS_OUTPUT;
 	    ASSERT((pp->status & ERTS_PORT_SFLGS_DEAD) == 0);
+            DTRACE_DRIVER(driver_ready_output, pp);
 	    (*pp->drv_ptr->ready_output)((ErlDrvData) pp->drv_data, ptp->event);
 	    io_tasks_executed++;
 	    break;
 	case ERTS_PORT_TASK_EVENT:
 	    reds += ERTS_PORT_REDS_EVENT;
 	    ASSERT((pp->status & ERTS_PORT_SFLGS_DEAD) == 0);
+            DTRACE_DRIVER(driver_event, pp);
 	    (*pp->drv_ptr->event)((ErlDrvData) pp->drv_data, ptp->event, ptp->event_data);
 	    io_tasks_executed++;
 	    break;
