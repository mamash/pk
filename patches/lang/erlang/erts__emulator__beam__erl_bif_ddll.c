$NetBSD$

--- erts/emulator/beam/erl_bif_ddll.c.orig	2011-05-24 11:16:43.000000000 +0000
+++ erts/emulator/beam/erl_bif_ddll.c
@@ -45,6 +45,7 @@
 #include "big.h"
 #include "dist.h"
 #include "erl_version.h"
+#include "dtrace-wrapper.h"
 
 #ifdef ERTS_SMP
 #define DDLL_SMP 1
@@ -1648,6 +1649,7 @@ static int do_unload_driver_entry(DE_Han
 	       diver_list lock here!*/
 	    if (q->finish) {
 		int fpe_was_unmasked = erts_block_fpe();
+		DTRACE1(driver_finish, q->name);
 		(*(q->finish))();
 		erts_unblock_fpe(fpe_was_unmasked);
 	    }
