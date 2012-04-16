$NetBSD$

--- erts/emulator/beam/erl_bif_port.c.orig	2011-05-24 11:16:43.000000000 +0000
+++ erts/emulator/beam/erl_bif_port.c
@@ -40,6 +40,7 @@
 #include "external.h"
 #include "packet_parser.h"
 #include "erl_bits.h"
+#include "dtrace-wrapper.h"
 
 static int open_port(Process* p, Eterm name, Eterm settings, int *err_nump);
 static byte* convert_environment(Process* p, Eterm env);
@@ -336,6 +337,14 @@ BIF_RETTYPE port_call_3(BIF_ALIST_3)
 		 __FILE__, __LINE__, endp - (bytes + size));
     }
     erts_smp_proc_unlock(BIF_P, ERTS_PROC_LOCK_MAIN);
+    if (DTRACE_ENABLED(driver_call)) {
+        DTRACE_CHARBUF(process_str, DTRACE_TERM_BUF_SIZE);
+        DTRACE_CHARBUF(port_str, DTRACE_TERM_BUF_SIZE);
+
+        dtrace_pid_str(p->connected, process_str);
+        dtrace_port_str(p, port_str);
+        DTRACE5(driver_call, process_str, port_str, p->name, op, real_size);
+    }
     prc  = (char *) port_resp;
     fpe_was_unmasked = erts_block_fpe();
     ret = drv->call((ErlDrvData)p->drv_data, 
@@ -532,6 +541,16 @@ BIF_RETTYPE port_connect_2(BIF_ALIST_2)
 
     prt->connected = pid; /* internal pid */
     erts_smp_port_unlock(prt);
+    if (DTRACE_ENABLED(port_connect)) {
+        DTRACE_CHARBUF(process_str, DTRACE_TERM_BUF_SIZE);
+        DTRACE_CHARBUF(port_str, DTRACE_TERM_BUF_SIZE);
+        DTRACE_CHARBUF(newprocess_str, DTRACE_TERM_BUF_SIZE);
+
+        dtrace_pid_str(prt->connected, process_str);
+        erts_snprintf(port_str, sizeof(port_str), "%T", prt->id);
+        dtrace_proc_str(rp, newprocess_str);
+        DTRACE4(port_connect, process_str, port_str, prt->name, newprocess_str);
+    }
     BIF_RET(am_true);
 }
 
@@ -897,6 +916,14 @@ open_port(Process* p, Eterm name, Eterm 
     erts_smp_proc_unlock(p, ERTS_PROC_LOCK_MAIN);
 
     port_num = erts_open_driver(driver, p->id, name_buf, &opts, err_nump);
+    if (port_num >= 0 && DTRACE_ENABLED(port_open)) {
+        DTRACE_CHARBUF(process_str, DTRACE_TERM_BUF_SIZE);
+        DTRACE_CHARBUF(port_str, DTRACE_TERM_BUF_SIZE);
+
+        dtrace_proc_str(p, process_str);
+        erts_snprintf(port_str, sizeof(port_str), "%T", erts_port[port_num].id);
+        DTRACE3(port_open, process_str, name_buf, port_str);
+    }
 
     erts_smp_proc_lock(p, ERTS_PROC_LOCK_MAIN);
 
