$NetBSD$

--- erts/emulator/beam/erl_async.c.orig	2011-05-24 11:16:43.000000000 +0000
+++ erts/emulator/beam/erl_async.c
@@ -24,6 +24,7 @@
 #include "erl_sys_driver.h"
 #include "global.h"
 #include "erl_threads.h"
+#include "dtrace-wrapper.h"
 
 typedef struct _erl_async {
     struct _erl_async* next;
@@ -47,14 +48,19 @@ typedef struct {
 #endif
     ErlAsync* head;
     ErlAsync* tail;
-#ifdef ERTS_ENABLE_LOCK_CHECK
     int no;
-#endif
 } AsyncQueue;
 
 static erts_smp_spinlock_t async_id_lock;
 static long async_id = 0;
 
+/*
+ * Some compilers, e.g. GCC 4.2.1 and -O3, will optimize away DTrace
+ * calls if they're the last thing in the function.  :-(
+ * Many thanks to Trond Norbye, via:
+ * https://github.com/memcached/memcached/commit/6298b3978687530bc9d219b6ac707a1b681b2a46
+ */
+static unsigned gcc_optimizer_hack = 0;
 
 #ifndef ERTS_SMP
 
@@ -135,9 +141,7 @@ int init_async(int hndl)
 #ifndef ERTS_SMP
 	q->hndl = hndl;
 #endif
-#ifdef ERTS_ENABLE_LOCK_CHECK
 	q->no = i;
-#endif
 	erts_mtx_init(&q->mtx, "asyncq");
 	erts_cnd_init(&q->cv);
 	erts_thr_create(&q->thr, async_main, (void*)q, &thr_opts);
@@ -175,6 +179,8 @@ int exit_async()
 
 static void async_add(ErlAsync* a, AsyncQueue* q)
 {
+    int len = 0;
+
     if (is_internal_port(a->port)) {
 	ERTS_LC_ASSERT(erts_drvportid2port(a->port));
 	/* make sure the driver will stay around */
@@ -195,12 +201,21 @@ static void async_add(ErlAsync* a, Async
 	q->head = a;
 	q->len++;
     }
+    len = q->len;
     erts_mtx_unlock(&q->mtx);
+    if (DTRACE_ENABLED(aio_pool_add)) {
+        DTRACE_CHARBUF(port_str, 16);
+
+        erts_snprintf(port_str, sizeof(port_str), "%T", a->port);
+        DTRACE2(aio_pool_add, port_str, len);
+    }
+    gcc_optimizer_hack++;
 }
 
 static ErlAsync* async_get(AsyncQueue* q)
 {
     ErlAsync* a;
+    int len;
 
     erts_mtx_lock(&q->mtx);
     while((a = q->tail) == NULL) {
@@ -218,7 +233,14 @@ static ErlAsync* async_get(AsyncQueue* q
 	q->tail = q->tail->prev;
 	q->len--;
     }
+    len = q->len;
     erts_mtx_unlock(&q->mtx);
+    if (DTRACE_ENABLED(aio_pool_get)) {
+        DTRACE_CHARBUF(port_str, 16);
+
+        erts_snprintf(port_str, sizeof(port_str), "%T", a->port);
+        DTRACE2(aio_pool_get, port_str, len);
+    }
     return a;
 }
 
