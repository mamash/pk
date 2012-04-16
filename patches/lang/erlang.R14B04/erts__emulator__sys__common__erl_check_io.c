$NetBSD$

--- erts/emulator/sys/common/erl_check_io.c.orig	2011-05-24 11:16:43.000000000 +0000
+++ erts/emulator/sys/common/erl_check_io.c
@@ -35,6 +35,7 @@
 #include "sys.h"
 #include "global.h"
 #include "erl_check_io.h"
+#include "dtrace-wrapper.h"
 
 #ifdef ERTS_SYS_CONTINOUS_FD_NUMBERS
 #  define ERTS_DRV_EV_STATE_EXTRA_SIZE 128
@@ -310,6 +311,7 @@ forget_removed(struct pollset_info* psi)
 	erts_smp_mtx_unlock(mtx);
 	if (drv_ptr) {
 	    int was_unmasked = erts_block_fpe();
+	    DTRACE1(driver_stop_select, drv_ptr->name);
 	    (*drv_ptr->stop_select) (fd, NULL);
 	    erts_unblock_fpe(was_unmasked);
 	    if (drv_ptr->handle) {
@@ -492,6 +494,7 @@ ERTS_CIO_EXPORT(driver_select)(ErlDrvPor
     ErtsDrvEventState *state;
     int wake_poller;
     int ret;
+    DTRACE_CHARBUF(name, 64);
     
     ERTS_SMP_LC_ASSERT(erts_drvport2port(ix)
 		       && erts_lc_is_port_locked(erts_drvport2port(ix)));
@@ -521,6 +524,8 @@ ERTS_CIO_EXPORT(driver_select)(ErlDrvPor
 	if (IS_FD_UNKNOWN(state)) {
 	    /* fast track to stop_select callback */
 	    stop_select_fn = erts_drvport2port(ix)->drv_ptr->stop_select;
+	    strncpy(name, erts_drvport2port(ix)->drv_ptr->name, sizeof(name)-1);
+	    name[sizeof(name)-1] = '\0';
 	    ret = 0;
 	    goto done_unknown;
 	}
@@ -657,6 +662,8 @@ ERTS_CIO_EXPORT(driver_select)(ErlDrvPor
 		    /* Safe to close fd now as it is not in pollset
 		       or there was no need to eject fd (kernel poll) */
 		    stop_select_fn = drv_ptr->stop_select;
+		    strncpy(name, erts_drvport2port(ix)->drv_ptr->name, sizeof(name)-1);
+		    name[sizeof(name)-1] = '\0';
 		}
 		else {
 		    /* Not safe to close fd, postpone stop_select callback. */
@@ -682,6 +689,7 @@ done_unknown:    
     erts_smp_mtx_unlock(fd_mtx(fd));
     if (stop_select_fn) {
 	int was_unmasked = erts_block_fpe();
+	DTRACE1(driver_stop_select, name);
 	(*stop_select_fn)(e, NULL);
 	erts_unblock_fpe(was_unmasked);
     }
