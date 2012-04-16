$NetBSD$

--- erts/emulator/drivers/common/efile_drv.c.orig	2011-05-24 11:16:43.000000000 +0000
+++ erts/emulator/drivers/common/efile_drv.c
@@ -103,6 +103,7 @@
 #include "erl_threads.h"
 #include "zlib.h"
 #include "gzio.h"
+#include "dtrace-wrapper.h"
 #include <ctype.h>
 #include <sys/types.h>
 
@@ -110,6 +111,43 @@ void erl_exit(int n, char *fmt, ...);
 
 static ErlDrvSysInfo sys_info;
 
+/* For explanation of this var, see comment for same var in erl_async.c */
+static unsigned gcc_optimizer_hack = 0;
+
+#ifdef  HAVE_DTRACE
+
+#define DTRACE_EFILE_BUFSIZ 128
+
+#define DTRACE_INVOKE_SETUP(op) \
+    do { DTRACE3(efile_drv_int_entry, d->sched_i1, d->sched_i2, op); } while (0)
+#define DTRACE_INVOKE_SETUP_BY_NAME(op) \
+    struct t_data *d = (struct t_data *) data ; \
+    DTRACE_INVOKE_SETUP(op)
+#define DTRACE_INVOKE_RETURN(op) \
+    do { DTRACE3(efile_drv_int_return, d->sched_i1, d->sched_i2, \
+                 op); } while (0) ; gcc_optimizer_hack++ ;
+
+/* Assign human-friendlier id numbers to scheduler & I/O worker threads */
+int             dt_driver_idnum = 0;
+int             dt_driver_io_worker_base = 5000;
+erts_mtx_t      dt_driver_mutex;
+pthread_key_t   dt_driver_key;
+
+typedef struct {
+    int         thread_num;
+    Uint64      tag;
+} dt_private;
+
+dt_private *get_dt_private(int);
+#else  /* HAVE_DTRACE */
+typedef struct {
+    char        dummy;          /* Unused except to quiet some compilers */
+} dt_private;
+
+#define DTRACE_INVOKE_SETUP(op)            do {} while (0)
+#define DTRACE_INVOKE_SETUP_BY_NAME(op)    do {} while (0)
+#define DTRACE_INVOKE_RETURN(op)           do {} while (0)
+#endif  /* HAVE_DTRACE */
 
 /* #define TRACE 1 */
 #ifdef TRACE
@@ -251,6 +289,10 @@ typedef struct {
     ErlDrvPDL       q_mtx;    /* Mutex for the driver queue, known by the emulator. Also used for
 				 mutual exclusion when accessing field(s) below. */
     size_t          write_buffered;
+#ifdef HAVE_DTRACE
+    int             idnum;      /* Unique ID # for this driver thread/desc */
+    char            port_str[DTRACE_TERM_BUF_SIZE];
+#endif
 } file_descriptor;
 
 
@@ -337,6 +379,13 @@ struct t_data
     void         (*free)(void *);
     int            again;
     int            reply;
+#ifdef  HAVE_DTRACE
+    int               sched_i1;
+    Uint64            sched_i2;
+    char              sched_utag[DTRACE_EFILE_BUFSIZ+1];
+#else
+    char              sched_utag[1];
+#endif
     int            result_ok;
     Efile_error    errInfo;
     int            flags;
@@ -399,8 +448,6 @@ struct t_data
     char b[1];
 };
 
-
-
 #define EF_ALLOC(S)		driver_alloc((S))
 #define EF_REALLOC(P, S)	driver_realloc((P), (S))
 #define EF_SAFE_ALLOC(S)	ef_safe_alloc((S))
@@ -429,7 +476,7 @@ static void *ef_safe_realloc(void *op, U
  * ErlIOVec manipulation functions.
  */
 
-/* char EV_CHAR(ErlIOVec *ev, int p, int q) */
+/* char EV_CHAR_P(ErlIOVec *ev, int p, int q) */
 #define EV_CHAR_P(ev, p, q)                   \
     (((char *)(ev)->iov[(q)].iov_base) + (p))
 
@@ -625,6 +672,10 @@ file_init(void)
 			    ? atoi(buf)
 			    : 0);
     driver_system_info(&sys_info, sizeof(ErlDrvSysInfo));
+#ifdef  HAVE_DTRACE
+    erts_mtx_init(&dt_driver_mutex, "efile_drv dtrace mutex");
+    pthread_key_create(&dt_driver_key, NULL);
+#endif  /* HAVE_DTRACE */
     return 0;
 }
 
@@ -661,6 +712,10 @@ file_start(ErlDrvPort port, char* comman
     desc->write_error = 0;
     MUTEX_INIT(desc->q_mtx, port); /* Refc is one, referenced by emulator now */
     desc->write_buffered = 0;
+#ifdef  HAVE_DTRACE
+    dtrace_drvport_str(port, desc->port_str);
+    get_dt_private(0);           /* throw away return value */
+#endif  /* HAVE_DTRACE */
     return (ErlDrvData) desc;
 }
 
@@ -680,8 +735,10 @@ static void do_close(int flags, SWord fd
 static void invoke_close(void *data)
 {
     struct t_data *d = (struct t_data *) data;
+    DTRACE_INVOKE_SETUP(FILE_CLOSE);
     d->again = 0;
     do_close(d->flags, d->fd);
+    DTRACE_INVOKE_RETURN(FILE_CLOSE);
 }
 
 /*********************************************************************
@@ -904,49 +961,63 @@ static void invoke_name(void *data, int 
 
 static void invoke_mkdir(void *data)
 {
+    DTRACE_INVOKE_SETUP_BY_NAME(FILE_MKDIR);
     invoke_name(data, efile_mkdir);
+    DTRACE_INVOKE_RETURN(FILE_MKDIR);
 }
 
 static void invoke_rmdir(void *data)
 {
+    DTRACE_INVOKE_SETUP_BY_NAME(FILE_RMDIR);
     invoke_name(data, efile_rmdir);
+    DTRACE_INVOKE_RETURN(FILE_RMDIR);
 }
 
 static void invoke_delete_file(void *data)
 {
+    DTRACE_INVOKE_SETUP_BY_NAME(FILE_DELETE);
     invoke_name(data, efile_delete_file);
+    DTRACE_INVOKE_RETURN(FILE_DELETE);
 }
 
 static void invoke_chdir(void *data)
 {
+    DTRACE_INVOKE_SETUP_BY_NAME(FILE_CHDIR);
     invoke_name(data, efile_chdir);
+    DTRACE_INVOKE_RETURN(FILE_CHDIR);
 }
 
 static void invoke_fdatasync(void *data)
 {
     struct t_data *d = (struct t_data *) data;
     int fd = (int) d->fd;
+    DTRACE_INVOKE_SETUP(FILE_FDATASYNC);
 
     d->again = 0;
     d->result_ok = efile_fdatasync(&d->errInfo, fd);
+    DTRACE_INVOKE_RETURN(FILE_FDATASYNC);
 }
 
 static void invoke_fsync(void *data)
 {
     struct t_data *d = (struct t_data *) data;
     int fd = (int) d->fd;
+    DTRACE_INVOKE_SETUP(FILE_FSYNC);
 
     d->again = 0;
     d->result_ok = efile_fsync(&d->errInfo, fd);
+    DTRACE_INVOKE_RETURN(FILE_FSYNC);
 }
 
 static void invoke_truncate(void *data)
 {
     struct t_data *d = (struct t_data *) data;
     int fd = (int) d->fd;
+    DTRACE_INVOKE_SETUP(FILE_TRUNCATE);
 
     d->again = 0;
     d->result_ok = efile_truncate_file(&d->errInfo, &fd, d->flags);
+    DTRACE_INVOKE_RETURN(FILE_TRUNCATE);
 }
 
 static void invoke_read(void *data)
@@ -954,6 +1025,7 @@ static void invoke_read(void *data)
     struct t_data *d = (struct t_data *) data;
     int status, segment;
     size_t size, read_size;
+    DTRACE_INVOKE_SETUP(FILE_READ);
 
     segment = d->again && d->c.read.bin_size >= 2*FILE_SEGMENT_READ;
     if (segment) {
@@ -988,6 +1060,7 @@ static void invoke_read(void *data)
     } else {
 	d->again = 0;
     }
+    DTRACE_INVOKE_RETURN(FILE_READ);
 }
 
 static void free_read(void *data)
@@ -1004,6 +1077,7 @@ static void invoke_read_line(void *data)
     int status;
     size_t read_size;
     int local_loop = (d->again == 0);
+    DTRACE_INVOKE_SETUP(FILE_READ_LINE);
 
     do {
 	size_t size = (d->c.read_line.binp)->orig_size - 
@@ -1095,6 +1169,7 @@ static void invoke_read_line(void *data)
 	    break;
 	}
     } while (local_loop);
+    DTRACE_INVOKE_RETURN(FILE_READ_LINE);
 }
 
 static void free_read_line(void *data)
@@ -1110,6 +1185,7 @@ static void invoke_read_file(void *data)
     struct t_data *d = (struct t_data *) data;
     size_t read_size;
     int chop;
+    DTRACE_INVOKE_SETUP(FILE_READ_FILE);
     
     if (! d->c.read_file.binp) { /* First invocation only */
 	int fd;
@@ -1146,12 +1222,14 @@ static void invoke_read_file(void *data)
 		   &read_size);
     if (d->result_ok) {
 	d->c.read_file.offset += read_size;
-	if (chop) return; /* again */
+	if (chop) goto chop_done; /* again */
     }
  close:
     efile_closefile((int) d->fd);
  done:
     d->again = 0;
+ chop_done:
+    DTRACE_INVOKE_RETURN(FILE_READ_FILE);
 }
 
 static void free_read_file(void *data)
@@ -1171,6 +1249,7 @@ static void invoke_preadv(void *data)
     ErlIOVec        *ev = &c->eiov;
     size_t           bytes_read_so_far = 0;
     unsigned char   *p = (unsigned char *)ev->iov[0].iov_base + 4+4+8*c->cnt;
+    DTRACE_INVOKE_SETUP(FILE_PREADV);
 
     while (c->cnt < c->n) {
 	size_t read_size = ev->iov[1 + c->cnt].iov_len - c->size;
@@ -1192,7 +1271,7 @@ static void invoke_preadv(void *data)
 	    bytes_read_so_far += bytes_read;
 	    if (chop && bytes_read == read_size) {
 		c->size += bytes_read;
-		return;
+		goto done;
 	    }
 	    ASSERT(bytes_read <= read_size);
 	    ev->iov[1 + c->cnt].iov_len = bytes_read + c->size;
@@ -1203,7 +1282,7 @@ static void invoke_preadv(void *data)
 	    if (d->again 
 		&& bytes_read_so_far >= FILE_SEGMENT_READ
 		&& c->cnt < c->n) {
-		return;
+		goto done;
 	    }
 	} else {
 	    /* In case of a read error, ev->size will not be correct,
@@ -1214,6 +1293,8 @@ static void invoke_preadv(void *data)
 	}
     }					
     d->again = 0;
+ done:
+    DTRACE_INVOKE_RETURN(FILE_PREADV);
 }
 
 static void free_preadv(void *data) {
@@ -1235,6 +1316,7 @@ static void invoke_ipread(void *data)
     size_t bytes_read = 0;
     char buf[2*sizeof(Uint32)];
     Uint32 offset, size;
+    DTRACE_INVOKE_SETUP(FILE_IPREAD);
     
     /* Read indirection header */
     if (! efile_pread(&d->errInfo, (int) d->fd, c->offsets[0], 
@@ -1273,14 +1355,17 @@ static void invoke_ipread(void *data)
     /* Read data block */
     d->invoke = invoke_preadv;
     invoke_preadv(data);
+    DTRACE_INVOKE_RETURN(FILE_IPREAD);
     return;
  error:
     d->result_ok = 0;
     d->again = 0;
+    DTRACE_INVOKE_RETURN(FILE_IPREAD);
     return;
  done:
     d->result_ok = !0;
     d->again = 0;
+    DTRACE_INVOKE_RETURN(FILE_IPREAD);
 }
 
 /* invoke_writev and invoke_pwritev are the only thread functions that
@@ -1303,6 +1388,7 @@ static void invoke_writev(void *data) {
     size_t         size;
     size_t         p;
     int            segment;
+    DTRACE_INVOKE_SETUP(FILE_WRITE);
 
     segment = d->again && d->c.writev.size >= 2*FILE_SEGMENT_WRITE;
     if (segment) {
@@ -1372,6 +1458,7 @@ static void invoke_writev(void *data) {
 	TRACE_F(("w%lu", (unsigned long)size));
 
     }
+    DTRACE_INVOKE_RETURN(FILE_WRITE);
 }
 
 static void free_writev(void *data) {
@@ -1385,34 +1472,40 @@ static void free_writev(void *data) {
 static void invoke_pwd(void *data)
 {
     struct t_data *d = (struct t_data *) data;
+    DTRACE_INVOKE_SETUP(FILE_PWD);
 
     d->again = 0;
     d->result_ok = efile_getdcwd(&d->errInfo,d->drive, d->b+1,
 				 RESBUFSIZE-1);
+    DTRACE_INVOKE_RETURN(FILE_PWD);
 }
 
 static void invoke_readlink(void *data)
 {
     struct t_data *d = (struct t_data *) data;
     char resbuf[RESBUFSIZE];	/* Result buffer. */
+    DTRACE_INVOKE_SETUP(FILE_READLINK);
 
     d->again = 0;
     d->result_ok = efile_readlink(&d->errInfo, d->b, resbuf+1,
 				  RESBUFSIZE-1);
     if (d->result_ok != 0)
 	FILENAME_COPY((char *) d->b + 1, resbuf+1);
+    DTRACE_INVOKE_RETURN(FILE_READLINK);
 }
 
 static void invoke_altname(void *data)
 {
     struct t_data *d = (struct t_data *) data;
     char resbuf[RESBUFSIZE];	/* Result buffer. */
+    DTRACE_INVOKE_SETUP(FILE_ALTNAME);
 
     d->again = 0;
     d->result_ok = efile_altname(&d->errInfo, d->b, resbuf+1,
 				  RESBUFSIZE-1);
     if (d->result_ok != 0)
 	FILENAME_COPY((char *) d->b + 1, resbuf+1);
+    DTRACE_INVOKE_RETURN(FILE_ALTNAME);
 }
 
 static void invoke_pwritev(void *data) {
@@ -1425,6 +1518,7 @@ static void invoke_pwritev(void *data) {
     size_t            p;
     int               segment;
     size_t            size, write_size;
+    DTRACE_INVOKE_SETUP(FILE_PWRITEV);
 
     segment = d->again && c->size >= 2*FILE_SEGMENT_WRITE;
     if (segment) {
@@ -1504,6 +1598,7 @@ static void invoke_pwritev(void *data) {
     }
  done:
     EF_FREE(iov); /* Free our copy of the vector, nothing to restore */
+    DTRACE_INVOKE_RETURN(FILE_PWRITEV);
 }
 
 static void free_pwritev(void *data) {
@@ -1519,9 +1614,14 @@ static void invoke_flstat(void *data)
 {
     struct t_data *d = (struct t_data *) data;
 
+    DTRACE3(efile_drv_int_entry, d->sched_i1, d->sched_i2,
+            d->command == FILE_LSTAT ? FILE_LSTAT : FILE_FSTAT);
     d->again = 0;
     d->result_ok = efile_fileinfo(&d->errInfo, &d->info,
 				  d->b, d->command == FILE_LSTAT);
+    DTRACE3(efile_drv_int_entry, d->sched_i1, d->sched_i2,
+            d->command == FILE_LSTAT ? FILE_LSTAT : FILE_FSTAT);
+    gcc_optimizer_hack++;
 }
 
 static void invoke_link(void *data)
@@ -1529,10 +1629,12 @@ static void invoke_link(void *data)
     struct t_data *d = (struct t_data *) data;
     char *name = d->b;
     char *new_name;
+    DTRACE_INVOKE_SETUP(FILE_LINK);
 
     d->again = 0;
     new_name = name+FILENAME_BYTELEN(name)+FILENAME_CHARSIZE;
     d->result_ok = efile_link(&d->errInfo, name, new_name);
+    DTRACE_INVOKE_RETURN(FILE_LINK);
 }
 
 static void invoke_symlink(void *data)
@@ -1540,10 +1642,12 @@ static void invoke_symlink(void *data)
     struct t_data *d = (struct t_data *) data;
     char *name = d->b;
     char *new_name;
+    DTRACE_INVOKE_SETUP(FILE_SYMLINK);
 
     d->again = 0;
     new_name = name+FILENAME_BYTELEN(name)+FILENAME_CHARSIZE;
     d->result_ok = efile_symlink(&d->errInfo, name, new_name);
+    DTRACE_INVOKE_RETURN(FILE_SYMLINK);
 }
 
 static void invoke_rename(void *data)
@@ -1551,24 +1655,29 @@ static void invoke_rename(void *data)
     struct t_data *d = (struct t_data *) data;
     char *name = d->b;
     char *new_name;
+    DTRACE_INVOKE_SETUP(FILE_RENAME);
 
     d->again = 0;
     new_name = name+FILENAME_BYTELEN(name)+FILENAME_CHARSIZE;
     d->result_ok = efile_rename(&d->errInfo, name, new_name);
+    DTRACE_INVOKE_RETURN(FILE_RENAME);
 }
 
 static void invoke_write_info(void *data)
 {
     struct t_data *d = (struct t_data *) data;
+    DTRACE_INVOKE_SETUP(FILE_WRITE_INFO);
 
     d->again = 0;
     d->result_ok = efile_write_info(&d->errInfo, &d->info, d->b);
+    DTRACE_INVOKE_RETURN(FILE_WRITE_INFO);
 }
 
 static void invoke_lseek(void *data)
 {
     struct t_data *d = (struct t_data *) data;
     int status;
+    DTRACE_INVOKE_SETUP(FILE_LSEEK);
 
     d->again = 0;
     if (d->flags & EFILE_COMPRESSED) {
@@ -1593,6 +1702,7 @@ static void invoke_lseek(void *data)
 			    &d->c.lseek.location);
     }
     d->result_ok = status;
+    DTRACE_INVOKE_RETURN(FILE_LSEEK);
 }
 
 static void invoke_readdir(void *data)
@@ -1602,6 +1712,7 @@ static void invoke_readdir(void *data)
     char *p = NULL;
     int buf_sz = 0;
     size_t tmp_bs;
+    DTRACE_INVOKE_SETUP(FILE_READDIR);
 
     d->again = 0;
     d->errInfo.posix_errno = 0;
@@ -1646,13 +1757,14 @@ static void invoke_readdir(void *data)
 	    break;
 	}
     }
+    DTRACE_INVOKE_RETURN(FILE_READDIR);
 }
 
 static void invoke_open(void *data)
 {
     struct t_data *d = (struct t_data *) data;
-    
     int status = 1;		/* Status of open call. */
+    DTRACE_INVOKE_SETUP(FILE_OPEN);
 
     d->again = 0;
     if ((d->flags & EFILE_COMPRESSED) == 0) {
@@ -1685,6 +1797,7 @@ static void invoke_open(void *data)
     }
 
     d->result_ok = status;
+    DTRACE_INVOKE_RETURN(FILE_OPEN);
 }
 
 static void invoke_fadvise(void *data)
@@ -1694,15 +1807,18 @@ static void invoke_fadvise(void *data)
     off_t offset = (off_t) d->c.fadvise.offset;
     off_t length = (off_t) d->c.fadvise.length;
     int advise = (int) d->c.fadvise.advise;
+    DTRACE_INVOKE_SETUP(FILE_FADVISE);
 
     d->again = 0;
     d->result_ok = efile_fadvise(&d->errInfo, fd, offset, length, advise);
+    DTRACE_INVOKE_RETURN(FILE_FADVISE);
 }
 
 static void free_readdir(void *data)
 {
     struct t_data *d = (struct t_data *) data;
     struct t_readdir_buf *b1 = d->c.read_dir.first_buf;
+
     while (b1) {
 	struct t_readdir_buf *b2 = b1;
 	b1 = b1->next;
@@ -1767,12 +1883,13 @@ static void cq_execute(file_descriptor *
     DRIVER_ASYNC(d->level, desc, d->invoke, void_ptr=d, d->free);
 }
 
-static int async_write(file_descriptor *desc, int *errp,
-		       int reply, Uint32 reply_size) {
+static struct t_data *async_write(file_descriptor *desc, int *errp,
+		       int reply, Uint32 reply_size,
+                       Sint64 *dt_i1, Sint64 *dt_i2, Sint64 *dt_i3) {
     struct t_data *d;
     if (! (d = EF_ALLOC(sizeof(struct t_data) - 1))) {
 	if (errp) *errp = ENOMEM;
-	return -1;
+	return NULL;
     }
     TRACE_F(("w%lu", (unsigned long)desc->write_buffered));
     d->command = FILE_WRITE;
@@ -1781,6 +1898,11 @@ static int async_write(file_descriptor *
     d->c.writev.port = desc->port;
     d->c.writev.q_mtx = desc->q_mtx;
     d->c.writev.size = desc->write_buffered;
+    if (dt_i1 != NULL) {
+        *dt_i1 = d->fd;
+        *dt_i2 = d->flags;
+        *dt_i3 = d->c.writev.size;
+    }
     d->reply = reply;
     d->c.writev.free_size = 0;
     d->c.writev.reply_size = reply_size;
@@ -1789,18 +1911,41 @@ static int async_write(file_descriptor *
     d->level = 1;
     cq_enq(desc, d);
     desc->write_buffered = 0;
-    return 0;
+    return d;
 }
 
-static int flush_write(file_descriptor *desc, int *errp) {
-    int    result;
+static int flush_write(file_descriptor *desc, int *errp,
+                       dt_private *dt_priv, char *dt_utag) {
+    int    result = 0;
+    Sint64 dt_i1 = 0, dt_i2 = 0, dt_i3 = 0;
+    struct t_data *d = NULL;
+
     MUTEX_LOCK(desc->q_mtx);
     if (desc->write_buffered > 0) {
-	result = async_write(desc, errp, 0, 0);
-    } else {
-	result = 0;
+	if ((d = async_write(desc, errp, 0, 0,
+                             &dt_i1, &dt_i2, &dt_i3)) == NULL) {
+            result = -1;
+        }
     }
     MUTEX_UNLOCK(desc->q_mtx);
+#ifdef HAVE_DTRACE
+    if (d != NULL) {
+        d->sched_i1 = dt_priv->thread_num;
+        d->sched_i2 = dt_priv->tag;
+        d->sched_utag[0] = '\0';
+        if (dt_utag != NULL) {
+            if (dt_utag[0] == '\0') {
+                dt_utag = NULL;
+            } else {
+                strncpy(d->sched_utag, dt_utag, sizeof(d->sched_utag) - 1);
+                d->sched_utag[sizeof(d->sched_utag) - 1] = '\0';
+            }
+        }
+        DTRACE11(efile_drv_entry, dt_priv->thread_num, dt_priv->tag++,
+                 dt_utag, FILE_WRITE,
+                 NULL, NULL, dt_i1, dt_i2, dt_i3, 0, desc->port_str);
+    }
+#endif /* HAVE_DTRACE */
     return result;
 }
 
@@ -1813,9 +1958,10 @@ static int check_write_error(file_descri
     return 0;
 }
 
-static int flush_write_check_error(file_descriptor *desc, int *errp) {
+static int flush_write_check_error(file_descriptor *desc, int *errp,
+                                   dt_private *dt_priv, char *dt_utag) {
     int r;
-    if ( (r = flush_write(desc, errp)) != 0) {
+    if ( (r = flush_write(desc, errp, dt_priv, dt_utag)) != 0) {
 	check_write_error(desc, NULL);
 	return r;
     } else {
@@ -1823,12 +1969,13 @@ static int flush_write_check_error(file_
     }
 }
 
-static int async_lseek(file_descriptor *desc, int *errp, int reply, 
-		       Sint64 offset, int origin) {
+static struct t_data *async_lseek(file_descriptor *desc, int *errp, int reply, 
+		       Sint64 offset, int origin,
+                       Sint64 *dt_i1, Sint64 *dt_i2, Sint64 *dt_i3) {
     struct t_data *d;
     if (! (d = EF_ALLOC(sizeof(struct t_data)))) {
 	*errp = ENOMEM;
-	return -1;
+	return NULL;
     }
     d->flags = desc->flags;
     d->fd = desc->fd;
@@ -1836,11 +1983,16 @@ static int async_lseek(file_descriptor *
     d->reply = reply;
     d->c.lseek.offset = offset;
     d->c.lseek.origin = origin;
+    if (dt_i1 != NULL) {
+        *dt_i1 = d->fd;
+        *dt_i2 = d->c.lseek.offset;
+        *dt_i3 = d->c.lseek.origin;
+    }
     d->invoke = invoke_lseek;
     d->free = free_data;
     d->level = 1;
     cq_enq(desc, d);
-    return 0;
+    return d;
 }
 
 static void flush_read(file_descriptor *desc) {
@@ -1852,18 +2004,37 @@ static void flush_read(file_descriptor *
     }
 }
 
-static int lseek_flush_read(file_descriptor *desc, int *errp) {
+static int lseek_flush_read(file_descriptor *desc, int *errp,
+                            dt_private *dt_priv, char *dt_utag) {
     int r = 0;
     size_t read_size = desc->read_size;
+    Sint64 dt_i1 = 0, dt_i2 = 0, dt_i3 = 0;
+    struct t_data *d;
+
+    flush_read(desc);
     if (read_size != 0) {
-	flush_read(desc);
-	if ((r = async_lseek(desc, errp, 0, 
-			     -((ssize_t)read_size), EFILE_SEEK_CUR)) 
-	    < 0) {
-	    return r;
-	}
-    } else {
-	flush_read(desc);
+	if ((d = async_lseek(desc, errp, 0, 
+                             -((ssize_t)read_size), EFILE_SEEK_CUR,
+                             &dt_i1, &dt_i2, &dt_i3)) == NULL) {
+            r = -1;
+        } else {
+#ifdef HAVE_DTRACE
+            d->sched_i1 = dt_priv->thread_num;
+            d->sched_i2 = dt_priv->tag;
+            d->sched_utag[0] = '\0';
+            if (dt_utag != NULL) {
+                if (dt_utag[0] == '\0') {
+                    dt_utag = NULL;
+                } else {
+                    strncpy(d->sched_utag, dt_utag, sizeof(d->sched_utag) - 1);
+                    d->sched_utag[sizeof(d->sched_utag) - 1] = '\0';
+                }
+            }
+            DTRACE11(efile_drv_entry, dt_priv->thread_num, dt_priv->tag++,
+                     dt_utag, FILE_LSEEK,
+                     NULL, NULL, dt_i1, dt_i2, dt_i3, 0, desc->port_str);
+#endif /* HAVE_DTRACE */
+        }
     }
     return r;
 }
@@ -1880,11 +2051,23 @@ file_async_ready(ErlDrvData e, ErlDrvThr
     struct t_data *d = (struct t_data *) data;
     char header[5];		/* result code + count */
     char resbuf[RESBUFSIZE];	/* Result buffer. */
-    
+#ifdef  HAVE_DTRACE
+    int sched_i1 = d->sched_i1, sched_i2 = d->sched_i2, command = d->command,
+        result_ok = d->result_ok,
+        posix_errno = d->result_ok ? 0 : d->errInfo.posix_errno;
+    DTRACE_CHARBUF(sched_utag, DTRACE_EFILE_BUFSIZ+1);
+
+    sched_utag[0] = '\0';
+    if (DTRACE_ENABLED(efile_drv_return)) {
+        strncpy(sched_utag, d->sched_utag, DTRACE_EFILE_BUFSIZ);
+        sched_utag[DTRACE_EFILE_BUFSIZ] = '\0';
+    }
+#endif  /* HAVE_DTRACE */
 
     TRACE_C('r');
 
     if (try_again(desc, d)) {
+        /* DTRACE TODO: what kind of probe makes sense here? */
 	return;
     }
 
@@ -2087,6 +2270,9 @@ file_async_ready(ErlDrvData e, ErlDrvThr
 	  if (d->reply) {
 	      TRACE_C('K');
 	      reply_ok(desc);
+#ifdef HAVE_DTRACE
+              result_ok = 1;
+#endif
 	  }
 	  free_data(data);
 	  break;
@@ -2119,6 +2305,8 @@ file_async_ready(ErlDrvData e, ErlDrvThr
       default:
 	abort();
     }
+    DTRACE6(efile_drv_return, sched_i1, sched_i2, sched_utag,
+            command, result_ok, posix_errno);
     if (desc->write_buffered != 0 && desc->timer_state == timer_idle) {
 	desc->timer_state = timer_write;
 	driver_set_timer(desc->port, desc->write_delay);
@@ -2140,7 +2328,11 @@ file_output(ErlDrvData e, char* buf, int
     char* name;			/* Points to the filename in buf. */
     int command;
     struct t_data *d = NULL;
-
+    char *dt_utag = NULL, *dt_s1 = NULL, *dt_s2 = NULL;
+    Sint64 dt_i1 = 0, dt_i2 = 0, dt_i3 = 0, dt_i4 = 0;
+#ifdef  HAVE_DTRACE
+    dt_private *dt_priv = get_dt_private(0);
+#endif  /* HAVE_DTRACE */
 
     TRACE_C('o');
 
@@ -2155,6 +2347,8 @@ file_output(ErlDrvData e, char* buf, int
 	d = EF_SAFE_ALLOC(sizeof(struct t_data) - 1 + FILENAME_BYTELEN(name) + FILENAME_CHARSIZE);
 	
 	FILENAME_COPY(d->b, name);
+	dt_s1 = d->b;
+	dt_utag = name + strlen(d->b) + 1;
 	d->command = command;
 	d->invoke = invoke_mkdir;
 	d->free = free_data;
@@ -2166,6 +2360,8 @@ file_output(ErlDrvData e, char* buf, int
 	d = EF_SAFE_ALLOC(sizeof(struct t_data) - 1 + FILENAME_BYTELEN(name) + FILENAME_CHARSIZE);
 	
 	FILENAME_COPY(d->b, name);
+	dt_s1 = d->b;
+	dt_utag = name + strlen(d->b) + 1;
 	d->command = command;
 	d->invoke = invoke_rmdir;
 	d->free = free_data;
@@ -2177,6 +2373,8 @@ file_output(ErlDrvData e, char* buf, int
 	d = EF_SAFE_ALLOC(sizeof(struct t_data) - 1 + FILENAME_BYTELEN(name) + FILENAME_CHARSIZE);
 	
 	FILENAME_COPY(d->b, name);
+	dt_s1 = d->b;
+	dt_utag = name + strlen(d->b) + 1;
 	d->command = command;
 	d->invoke = invoke_delete_file;
 	d->free = free_data;
@@ -2193,7 +2391,10 @@ file_output(ErlDrvData e, char* buf, int
 			      + FILENAME_BYTELEN(new_name) + FILENAME_CHARSIZE);
 	
 	    FILENAME_COPY(d->b, name);
+	    dt_s1 = d->b;
 	    FILENAME_COPY(d->b + namelen, new_name);
+	    dt_s2 = d->b + namelen;
+	    dt_utag = buf + namelen + strlen(dt_s2) + 1;
 	    d->flags = desc->flags;
 	    d->fd = fd;
 	    d->command = command;
@@ -2207,6 +2408,8 @@ file_output(ErlDrvData e, char* buf, int
 	d = EF_SAFE_ALLOC(sizeof(struct t_data) - 1 + FILENAME_BYTELEN(name) + FILENAME_CHARSIZE);
 	
 	FILENAME_COPY(d->b, name);
+	dt_s1 = d->b;
+	dt_utag = name + strlen(d->b) + 1;
 	d->command = command;
 	d->invoke = invoke_chdir;
 	d->free = free_data;
@@ -2218,6 +2421,7 @@ file_output(ErlDrvData e, char* buf, int
 	    d = EF_SAFE_ALLOC(sizeof(struct t_data) - 1 + RESBUFSIZE + 1);
 	
 	    d->drive = *(uchar*)buf;
+	    dt_utag = buf + 1;
 	    d->command = command;
 	    d->invoke = invoke_pwd;
 	    d->free = free_data;
@@ -2233,6 +2437,8 @@ file_output(ErlDrvData e, char* buf, int
 			      FILENAME_CHARSIZE);
 	
 	    FILENAME_COPY(d->b, name);
+	    dt_s1 = d->b;
+	    dt_utag = name + strlen(d->b) + 1;
 	    d->dir_handle = NULL;
 	    d->command = command;
 	    d->invoke = invoke_readdir;
@@ -2253,6 +2459,8 @@ file_output(ErlDrvData e, char* buf, int
 	    dir_handle = NULL;
 	    resbuf[0] = FILE_RESP_FNAME;
 	    resbufsize = RESBUFSIZE;
+	    dt_s1 = name;
+	    dt_utag = name + strlen(dt_s1) + 1;
 
 	    while (efile_readdir(&errInfo, name, &dir_handle,
 				 resbuf+1, &resbufsize)) {
@@ -2263,6 +2471,11 @@ file_output(ErlDrvData e, char* buf, int
 		reply_error(desc, &errInfo);
 		return;
 	    }
+#ifdef HAVE_DTRACE
+	    DTRACE11(efile_drv_entry, dt_priv->thread_num, dt_priv->tag++,
+		     dt_utag, command, name, dt_s2,
+		     dt_i1, dt_i2, dt_i3, dt_i4, desc->port_str);
+#endif
 	    TRACE_C('R');
 	    driver_output2(desc->port, resbuf, 1, NULL, 0);
 	    return;
@@ -2273,8 +2486,11 @@ file_output(ErlDrvData e, char* buf, int
 			      FILENAME_CHARSIZE);
 	
 	    d->flags = get_int32((uchar*)buf);
+	    dt_i1 = d->flags;
 	    name = buf+4;
 	    FILENAME_COPY(d->b, name);
+	    dt_s1 = d->b;
+	    dt_utag = name + strlen(d->b) + 1;
 	    d->command = command;
 	    d->invoke = invoke_open;
 	    d->free = free_data;
@@ -2286,7 +2502,9 @@ file_output(ErlDrvData e, char* buf, int
 	{
 	    d = EF_SAFE_ALLOC(sizeof(struct t_data));
 	    
+	    dt_utag = name;
 	    d->fd = fd;
+	    dt_i1 = fd;
 	    d->command = command;
 	    d->invoke = invoke_fdatasync;
 	    d->free = free_data;
@@ -2298,7 +2516,9 @@ file_output(ErlDrvData e, char* buf, int
 	{
 	    d = EF_SAFE_ALLOC(sizeof(struct t_data));
 	    
+	    dt_utag = name;
 	    d->fd = fd;
+	    dt_i1 = fd;
 	    d->command = command;
 	    d->invoke = invoke_fsync;
 	    d->free = free_data;
@@ -2314,7 +2534,13 @@ file_output(ErlDrvData e, char* buf, int
 			      FILENAME_CHARSIZE);
 	    
 	    FILENAME_COPY(d->b, name);
+	    dt_utag = name + strlen(d->b) + 1;
 	    d->fd = fd;
+	    if (command == FILE_LSTAT) {
+		dt_s1 = d->b;
+	    } else {
+		dt_i1 = fd;
+	    }
 	    d->command = command;
 	    d->invoke = invoke_flstat;
 	    d->free = free_data;
@@ -2326,8 +2552,11 @@ file_output(ErlDrvData e, char* buf, int
         {
 	    d = EF_SAFE_ALLOC(sizeof(struct t_data));
 	    
+	    dt_utag = name;
 	    d->flags = desc->flags;
 	    d->fd = fd;
+	    dt_i1 = fd;
+	    dt_i2 = d->flags;
 	    d->command = command;
 	    d->invoke = invoke_truncate;
 	    d->free = free_data;
@@ -2341,12 +2570,17 @@ file_output(ErlDrvData e, char* buf, int
 			      + FILENAME_BYTELEN(buf+21*4) + FILENAME_CHARSIZE);
 	    
 	    d->info.mode = get_int32(buf + 0 * 4);
+	    dt_i1 = d->info.mode;
 	    d->info.uid = get_int32(buf + 1 * 4);
+	    dt_i2 = d->info.uid;
 	    d->info.gid = get_int32(buf + 2 * 4);
+	    dt_i3 = d->info.gid;
 	    GET_TIME(d->info.accessTime, buf + 3 * 4);
 	    GET_TIME(d->info.modifyTime, buf + 9 * 4);
 	    GET_TIME(d->info.cTime, buf + 15 * 4);
 	    FILENAME_COPY(d->b, buf+21*4);
+	    dt_s1 = d->b;
+	    dt_utag = buf + 21 * 4 + strlen(d->b) + 1;
 	    d->command = command;
 	    d->invoke = invoke_write_info;
 	    d->free = free_data;
@@ -2359,6 +2593,8 @@ file_output(ErlDrvData e, char* buf, int
 	    d = EF_SAFE_ALLOC(sizeof(struct t_data) - 1 + RESBUFSIZE + 1);
 	
 	    FILENAME_COPY(d->b, name);
+	    dt_s1 = d->b;
+	    dt_utag = name + strlen(d->b) + 1;
 	    d->command = command;
 	    d->invoke = invoke_readlink;
 	    d->free = free_data;
@@ -2370,6 +2606,8 @@ file_output(ErlDrvData e, char* buf, int
 	{
 	    d = EF_SAFE_ALLOC(sizeof(struct t_data) - 1 + RESBUFSIZE + 1);
 	    FILENAME_COPY(d->b, name);
+	    dt_s1 = d->b;
+	    dt_utag = name + strlen(d->b) + 1;
 	    d->command = command;
 	    d->invoke = invoke_altname;
 	    d->free = free_data;
@@ -2389,7 +2627,10 @@ file_output(ErlDrvData e, char* buf, int
 			      + FILENAME_BYTELEN(new_name) + FILENAME_CHARSIZE);
 	
 	    FILENAME_COPY(d->b, name);
+	    dt_s1 = d->b;
 	    FILENAME_COPY(d->b + namelen, new_name);
+	    dt_s2 = d->b + namelen;
+	    dt_utag = buf + namelen + strlen(dt_s2) + 1;
 	    d->flags = desc->flags;
 	    d->fd = fd;
 	    d->command = command;
@@ -2410,7 +2651,10 @@ file_output(ErlDrvData e, char* buf, int
 			      + FILENAME_BYTELEN(new_name) + FILENAME_CHARSIZE);
 	
 	    FILENAME_COPY(d->b, name);
+	    dt_s1 = d->b;
 	    FILENAME_COPY(d->b + namelen, new_name);
+	    dt_s2 = d->b + namelen;
+	    dt_utag = buf + namelen + strlen(dt_s2) + 1;
 	    d->flags = desc->flags;
 	    d->fd = fd;
 	    d->command = command;
@@ -2425,13 +2669,18 @@ file_output(ErlDrvData e, char* buf, int
         d = EF_SAFE_ALLOC(sizeof(struct t_data));
 
         d->fd = fd;
+        dt_i1 = d->fd;
         d->command = command;
         d->invoke = invoke_fadvise;
         d->free = free_data;
         d->level = 2;
         d->c.fadvise.offset = get_int64((uchar*) buf);
+        dt_i2 = d->c.fadvise.offset;
         d->c.fadvise.length = get_int64(((uchar*) buf) + sizeof(Sint64));
+        dt_i3 = d->c.fadvise.length;
         d->c.fadvise.advise = get_int32(((uchar*) buf) + 2 * sizeof(Sint64));
+        dt_i4 = d->c.fadvise.advise;
+        dt_utag = buf + 3 * sizeof(Sint64);
         goto done;
     }
 
@@ -2445,6 +2694,22 @@ file_output(ErlDrvData e, char* buf, int
 
  done:
     if (d) {
+#ifdef HAVE_DTRACE
+	d->sched_i1 = dt_priv->thread_num;
+	d->sched_i2 = dt_priv->tag;
+	d->sched_utag[0] = '\0';
+	if (dt_utag != NULL) {
+	    if (dt_utag[0] == '\0') {
+		dt_utag = NULL;
+	    } else {
+		strncpy(d->sched_utag, dt_utag, sizeof(d->sched_utag) - 1);
+		d->sched_utag[sizeof(d->sched_utag) - 1] = '\0';
+	    }
+	}
+	DTRACE11(efile_drv_entry, dt_priv->thread_num, dt_priv->tag++,
+		 dt_utag, command, dt_s1, dt_s2,
+		 dt_i1, dt_i2, dt_i3, dt_i4, desc->port_str);
+#endif
 	cq_enq(desc, d);
     }
 }
@@ -2456,10 +2721,16 @@ static void 
 file_flush(ErlDrvData e) {
     file_descriptor *desc = (file_descriptor *)e;
     int r;
+#ifdef  HAVE_DTRACE
+    dt_private *dt_priv = get_dt_private(dt_driver_io_worker_base);
+#else
+    dt_private *dt_priv = NULL;
+#endif
 
     TRACE_C('f');
 
-    r = flush_write(desc, NULL);
+    r = flush_write(desc, NULL, dt_priv,
+                    (desc->d == NULL) ? NULL : desc->d->sched_utag);
     /* Only possible reason for bad return value is ENOMEM, and 
      * there is nobody to tell...
      */
@@ -2493,6 +2764,11 @@ static void 
 file_timeout(ErlDrvData e) {
     file_descriptor *desc = (file_descriptor *)e;
     enum e_timer timer_state = desc->timer_state;
+#ifdef  HAVE_DTRACE
+    dt_private *dt_priv = get_dt_private(dt_driver_io_worker_base);
+#else
+    dt_private *dt_priv = NULL;
+#endif
 
     TRACE_C('t');
 
@@ -2507,7 +2783,8 @@ file_timeout(ErlDrvData e) {
 	driver_async(desc->port, KEY(desc), desc->invoke, desc->d, desc->free);
 	break;
     case timer_write: {
-	int r = flush_write(desc, NULL);
+	int r = flush_write(desc, NULL, dt_priv,
+                            (desc->d == NULL) ? NULL : desc->d->sched_utag);
 	/* Only possible reason for bad return value is ENOMEM, and 
 	 * there is nobody to tell...
 	 */
@@ -2529,7 +2806,14 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
     char command;
     int p, q;
     int err;
-
+    struct t_data *d = NULL;
+    Sint64 dt_i1 = 0, dt_i2 = 0, dt_i3 = 0, dt_i4 = 0;
+    char *dt_utag = NULL, *dt_s1 = NULL;
+#ifdef  HAVE_DTRACE
+    dt_private *dt_priv = get_dt_private(dt_driver_io_worker_base);
+#else
+    dt_private *dt_priv = NULL;
+#endif
     TRACE_C('v');
 
     p = 0; q = 1;
@@ -2548,25 +2832,22 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
     switch (command) {
 
     case FILE_CLOSE: {
+	dt_utag = EV_CHAR_P(ev, p, q);
 	flush_read(desc);
-	if (flush_write_check_error(desc, &err) < 0) {
+	if (flush_write_check_error(desc, &err, dt_priv, dt_utag) < 0) {
 	    reply_posix_error(desc, err);
 	    goto done;
 	}
-	if (ev->size != 1) {
-	    /* Wrong command length */
-	    reply_posix_error(desc, EINVAL);
-	    goto done;
-	}
 	if (desc->fd != FILE_FD_INVALID) {
-	    struct t_data *d;
 	    if (! (d = EF_ALLOC(sizeof(struct t_data)))) {
 		reply_posix_error(desc, ENOMEM);
 	    } else {
 		d->command = command;
 		d->reply = !0;
 		d->fd = desc->fd;
+		dt_i1 = d->fd;
 		d->flags = desc->flags;
+		dt_i2 = d->flags;
 		d->invoke = invoke_close;
 		d->free = free_data;
 		d->level = 2;
@@ -2582,8 +2863,15 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
     case FILE_READ: {
 	Uint32 sizeH, sizeL;
 	size_t size, alloc_size;
-	struct t_data *d;
-	if (flush_write_check_error(desc, &err) < 0) {
+
+	if (!EV_GET_UINT32(ev, &sizeH, &p, &q)
+	    || !EV_GET_UINT32(ev, &sizeL, &p, &q)) {
+	    /* Wrong buffer length to contain the read count */
+	    reply_posix_error(desc, EINVAL);
+	    goto done;
+	}
+	dt_utag = EV_CHAR_P(ev, p, q);
+	if (flush_write_check_error(desc, &err, dt_priv, dt_utag) < 0) {
 	    reply_posix_error(desc, err);
 	    goto done;
 	}
@@ -2591,19 +2879,12 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
 	if (desc->read_bufsize == 0 && desc->read_binp != NULL && desc->read_size > 0) {
 	    /* We have allocated a buffer for line mode but should not really have a 
 	       read-ahead buffer... */
-	    if (lseek_flush_read(desc, &err) < 0) {
+	    if (lseek_flush_read(desc, &err, dt_priv) < 0) {
 		reply_posix_error(desc, err);
 		goto done;
 	    }
 	}
 #endif
-	if (ev->size != 1+8
-	    || !EV_GET_UINT32(ev, &sizeH, &p, &q)
-	    || !EV_GET_UINT32(ev, &sizeL, &p, &q)) {
-	    /* Wrong buffer length to contain the read count */
-	    reply_posix_error(desc, EINVAL);
-	    goto done;
-	}
 #if SIZEOF_SIZE_T == 4
 	if (sizeH != 0) {
 	    reply_posix_error(desc, EINVAL);
@@ -2674,11 +2955,14 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
 	d->command = command;
 	d->reply = !0;
 	d->fd = desc->fd;
+	dt_i1 = d->fd;
 	d->flags = desc->flags;
+	dt_i2 = d->flags;
 	d->c.read.binp = desc->read_binp;
 	d->c.read.bin_offset = desc->read_offset + desc->read_size;
 	d->c.read.bin_size = desc->read_binp->orig_size - d->c.read.bin_offset;
 	d->c.read.size = size;
+	dt_i3 = d->c.read.size;
 	driver_binary_inc_refc(d->c.read.binp);
 	d->invoke = invoke_read;
 	d->free = free_read;
@@ -2696,12 +2980,12 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
 	 *    allocated binary + dealing with offsets and lengts are done in file_async ready
 	 *    for this OP.
 	 */
-	struct t_data *d;
-	if (flush_write_check_error(desc, &err) < 0) {
+	dt_utag = EV_CHAR_P(ev, p, q);
+	if (flush_write_check_error(desc, &err, dt_priv, dt_utag) < 0) {
 	    reply_posix_error(desc, err);
 	    goto done;
 	}
-	if (ev->size != 1) {
+	if (ev->size != 1+strlen(dt_utag)+1) {
 	    /* Wrong command length */
 	    reply_posix_error(desc, EINVAL);
 	    goto done;
@@ -2753,13 +3037,17 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
 	d->command = command;
 	d->reply = !0;
 	d->fd = desc->fd;
+	dt_i1 = d->fd;
 	d->flags = desc->flags;
+	dt_i2 = d->flags;
 	d->c.read_line.binp = desc->read_binp;
 	d->c.read_line.read_offset = desc->read_offset;
 	d->c.read_line.read_size = desc->read_size;
+	dt_i3 = d->c.read_line.read_offset;
 #if !ALWAYS_READ_LINE_AHEAD
 	d->c.read_line.read_ahead = (desc->read_bufsize > 0);
 #endif 
+	dt_i4 = d->c.read_line.read_ahead;
 	driver_binary_inc_refc(d->c.read.binp);
 	d->invoke = invoke_read_line;
 	d->free = free_read_line;
@@ -2768,8 +3056,22 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
     } goto done;
     case FILE_WRITE: {
 	int skip = 1;
-	int size = ev->size - skip;
-	if (lseek_flush_read(desc, &err) < 0) {
+	int size;
+
+	dt_utag = EV_CHAR_P(ev, p, q);
+	skip += strlen(dt_utag) + 1;
+	size = ev->size - skip;
+	/*
+	 * Interesting dependency on using port # for key to async
+	 * I/O worker pool thread: lseek_flush_read() can enqueue a
+	 * lseek() op.  If that lseek() were scheduled on a different
+	 * thread than the write that we'll enqueue later in this case,
+	 * then Bad Things could happen.  This DTrace work is probably
+	 * well worthwhile to get a sense of how often there's head-of-
+	 * line blocking/unfairness during busy file I/O because of the
+	 * mapping of port #/key -> thread.
+	 */
+	if (lseek_flush_read(desc, &err, dt_priv, dt_utag) < 0) {
 	    reply_posix_error(desc, err);
 	    goto done;
 	}
@@ -2796,7 +3098,8 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
 		driver_set_timer(desc->port, desc->write_delay);
 	    }
 	} else {
-	    if (async_write(desc, &err, !0, size) != 0) {
+	    if ((d = async_write(desc, &err, !0, size,
+				 &dt_i1, &dt_i2, &dt_i3)) == NULL) {
 		MUTEX_UNLOCK(desc->q_mtx);
 		reply_posix_error(desc, err);
 		goto done;
@@ -2809,19 +3112,25 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
     case FILE_PWRITEV: {
 	Uint32 i, j, n; 
 	size_t total;
-	struct t_data *d;
-	if (lseek_flush_read(desc, &err) < 0) {
-	    reply_Uint_posix_error(desc, 0, err);
+	char tmp;
+	int dt_utag_bytes = 1;
+
+	dt_utag = EV_CHAR_P(ev, p, q);
+	while (EV_GET_CHAR(ev, &tmp, &p, &q) && tmp != '\0') {
+	    dt_utag_bytes++;
+	}
+	if (ev->size < 1+4+dt_utag_bytes
+	    || !EV_GET_UINT32(ev, &n, &p, &q)) {
+	    /* Buffer too short to contain even the number of pos/size specs */
+	    reply_Uint_posix_error(desc, 0, EINVAL);
 	    goto done;
 	}
-	if (flush_write_check_error(desc, &err) < 0) {
+	if (lseek_flush_read(desc, &err, dt_priv, dt_utag) < 0) {
 	    reply_Uint_posix_error(desc, 0, err);
 	    goto done;
 	}
-	if (ev->size < 1+4
-	    || !EV_GET_UINT32(ev, &n, &p, &q)) {
-	    /* Buffer too short to contain even the number of pos/size specs */
-	    reply_Uint_posix_error(desc, 0, EINVAL);
+	if (flush_write_check_error(desc, &err, dt_priv, dt_utag) < 0) {
+	    reply_Uint_posix_error(desc, 0, err);
 	    goto done;
 	}
 	if (n == 0) {
@@ -2833,7 +3142,7 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
 	    }
 	    goto done;
 	}
-	if (ev->size < 1+4+8*(2*n)) {
+	if (ev->size < 1+4+8*(2*n)+dt_utag_bytes) {
 	    /* Buffer too short to contain even the pos/size specs */
 	    reply_Uint_posix_error(desc, 0, EINVAL);
 	    goto done;
@@ -2847,7 +3156,9 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
 	d->command = command;
 	d->reply = !0;
 	d->fd = desc->fd;
+	dt_i1 = d->fd;
 	d->flags = desc->flags;
+	dt_i2 = d->flags;
 	d->c.pwritev.port = desc->port;
 	d->c.pwritev.q_mtx = desc->q_mtx;
 	d->c.pwritev.n = n;
@@ -2885,13 +3196,14 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
 	    }
 	}
 	d->c.pwritev.size = total;
+	dt_i3 = d->c.pwritev.size;
 	d->c.pwritev.free_size = 0;
 	if (j == 0) {
 	    /* Trivial case - nothing to write */
 	    EF_FREE(d);
 	    reply_Uint(desc, 0);
 	} else {
-	    size_t skip = 1 + 4 + 8*(2*n);
+	    size_t skip = 1 + 4 + 8*(2*n) + dt_utag_bytes;
 	    if (skip + total != ev->size) {
 		/* Actual amount of data does not match 
 		 * total of all pos/size specs
@@ -2915,24 +3227,30 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
     case FILE_PREADV: {
 	register void * void_ptr;
 	Uint32 i, n;
-	struct t_data *d;
 	ErlIOVec *res_ev;
-	if (lseek_flush_read(desc, &err) < 0) {
+	char tmp;
+	int dt_utag_bytes = 1;
+
+	dt_utag = EV_CHAR_P(ev, p, q);
+	while (EV_GET_CHAR(ev, &tmp, &p, &q) && tmp != '\0') {
+	    dt_utag_bytes++;
+	}
+	if (lseek_flush_read(desc, &err, dt_priv, dt_utag) < 0) {
 	    reply_posix_error(desc, err);
 	    goto done;
 	}
-	if (flush_write_check_error(desc, &err) < 0) {
+	if (flush_write_check_error(desc, &err, dt_priv, dt_utag) < 0) {
 	    reply_posix_error(desc, err);
 	    goto done;
 	}
-	if (ev->size < 1+8
+	if (ev->size < 1+8+dt_utag_bytes
 	    || !EV_GET_UINT32(ev, &n, &p, &q)
 	    || !EV_GET_UINT32(ev, &n, &p, &q)) {
 	    /* Buffer too short to contain even the number of pos/size specs */
 	    reply_posix_error(desc, EINVAL);
 	    goto done;
 	}
-	if (ev->size != 1+8+8*(2*n)) {
+	if (ev->size < 1+8+8*(2*n)+dt_utag_bytes) {
 	    /* Buffer wrong length to contain the pos/size specs */
 	    reply_posix_error(desc, EINVAL);
 	    goto done;
@@ -2951,7 +3269,9 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
 	d->command = command;
 	d->reply = !0;
 	d->fd = desc->fd;
+	dt_i1 = d->fd;
 	d->flags = desc->flags;
+	dt_i2 = d->flags;
 	d->c.preadv.n = n;
 	d->c.preadv.cnt = 0;
 	d->c.preadv.size = 0;
@@ -2979,6 +3299,7 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
 #else
 	    size = ((size_t)sizeH<<32) | sizeL;
 #endif
+	    dt_i3 += size;
 	    if (! (res_ev->binv[i] = driver_alloc_binary(size))) {
 		reply_posix_error(desc, ENOMEM);
 		break;
@@ -3025,31 +3346,33 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
     } goto done; /* case FILE_PREADV: */
 
     case FILE_LSEEK: {
-	Sint64 offset;          /* Offset for seek */
+	Sint64 offset;		/* Offset for seek */
 	Uint32 origin;		/* Origin of seek. */
-	if (lseek_flush_read(desc, &err) < 0) {
-	    reply_posix_error(desc, err);
+
+	if (ev->size < 1+8+4
+	    || !EV_GET_UINT64(ev, &offset, &p, &q)
+	    || !EV_GET_UINT32(ev, &origin, &p, &q)) {
+	    /* Wrong length of buffer to contain offset and origin */
+	    reply_posix_error(desc, EINVAL);
 	    goto done;
 	}
-	if (flush_write_check_error(desc, &err) < 0) {
+	dt_utag = EV_CHAR_P(ev, p, q);
+	if (lseek_flush_read(desc, &err, dt_priv, dt_utag) < 0) {
 	    reply_posix_error(desc, err);
 	    goto done;
 	}
-	if (ev->size != 1+8+4
-	    || !EV_GET_UINT64(ev, &offset, &p, &q)
-	    || !EV_GET_UINT32(ev, &origin, &p, &q)) {
-	    /* Wrong length of buffer to contain offset and origin */
-	    reply_posix_error(desc, EINVAL);
+	if (flush_write_check_error(desc, &err, dt_priv, dt_utag) < 0) {
+	    reply_posix_error(desc, err);
 	    goto done;
 	}
-	if (async_lseek(desc, &err, !0, offset, origin) < 0) {
+	if ((d = async_lseek(desc, &err, !0, offset, origin,
+			     &dt_i1, &dt_i2, &dt_i3)) == NULL) {
 	    reply_posix_error(desc, err);
 	    goto done;
 	}
     } goto done;
 
     case FILE_READ_FILE: {
-	struct t_data *d;
 	char *filename;
 	if (ev->size < 1+1) {
 	    /* Buffer contains empty name */
@@ -3071,6 +3394,8 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
 	d->reply = !0;
 	/* Copy name */
 	FILENAME_COPY(d->b, filename);
+	dt_s1 = d->b;
+	dt_utag = filename + strlen(d->b) + 1;
 	d->c.read_file.binp = NULL;
 	d->invoke = invoke_read_file;
 	d->free = free_read_file;
@@ -3090,7 +3415,6 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
 	char mode;
 	Sint64 hdr_offset;
 	Uint32 max_size;
-	struct t_data *d;
 	ErlIOVec *res_ev;
 	int vsize;
 	if (! EV_GET_CHAR(ev, &mode, &p, &q)) {
@@ -3102,14 +3426,6 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
 	    reply_posix_error(desc, EINVAL);
 	    goto done;
 	}
-	if (lseek_flush_read(desc, &err) < 0) {
-	    reply_posix_error(desc, err);
-	    goto done;
-	}
-	if (flush_write_check_error(desc, &err) < 0) {
-	    reply_posix_error(desc, err);
-	    goto done;
-	}
 	if (ev->size < 1+1+8+4
 	    || !EV_GET_UINT64(ev, &hdr_offset, &p, &q)
 	    || !EV_GET_UINT32(ev, &max_size, &p, &q)) {
@@ -3118,6 +3434,15 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
 	    reply_posix_error(desc, EINVAL);
 	    goto done;
 	}
+	dt_utag = EV_CHAR_P(ev, p, q);
+	if (lseek_flush_read(desc, &err, dt_priv, dt_utag) < 0) {
+	    reply_posix_error(desc, err);
+	    goto done;
+	}
+	if (flush_write_check_error(desc, &err, dt_priv, dt_utag) < 0) {
+	    reply_posix_error(desc, err);
+	    goto done;
+	}
 	/* Create the thread data structure with the contained ErlIOVec 
 	 * and corresponding binaries for the response 
 	 */
@@ -3131,9 +3456,13 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
 	d->command = command;
 	d->reply = !0;
 	d->fd = desc->fd;
+	dt_i1 = d->fd;
 	d->flags = desc->flags;
+	dt_i2 = d->flags;
 	d->c.preadv.offsets[0] = hdr_offset;
+	dt_i3 = d->c.preadv.offsets[0];
 	d->c.preadv.size = max_size;
+	dt_i4 = d->c.preadv.size;
 	res_ev = &d->c.preadv.eiov;
 	/* XXX possible alignment problems here for weird machines */
 	res_ev->iov = void_ptr = d + 1;
@@ -3148,16 +3477,19 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
 
     case FILE_SETOPT: {
 	char opt;
+
 	if (ev->size < 1+1
 	    || !EV_GET_CHAR(ev, &opt, &p, &q)) {
 	    /* Buffer too short to contain even the option type */
 	    reply_posix_error(desc, EINVAL);
 	    goto done;
 	}
+	dt_i1 = opt;
+	dt_utag = EV_CHAR_P(ev, p, q);
 	switch (opt) {
 	case FILE_OPT_DELAYED_WRITE: {
 	    Uint32 sizeH, sizeL, delayH, delayL;
-	    if (ev->size != 1+1+4*sizeof(Uint32)
+	    if (ev->size != 1+1+4*sizeof(Uint32)+strlen(dt_utag)+1
 		|| !EV_GET_UINT32(ev, &sizeH, &p, &q)
 		|| !EV_GET_UINT32(ev, &sizeL, &p, &q)
 		|| !EV_GET_UINT32(ev, &delayH, &p, &q)
@@ -3184,12 +3516,13 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
 #else
 	    desc->write_delay = ((unsigned long)delayH << 32) | delayL;
 #endif
+	    dt_i2 = desc->write_delay;
 	    TRACE_C('K');
 	    reply_ok(desc);
 	} goto done;
 	case FILE_OPT_READ_AHEAD: {
 	    Uint32 sizeH, sizeL;
-	    if (ev->size != 1+1+2*sizeof(Uint32)
+	    if (ev->size != 1+1+2*sizeof(Uint32)+strlen(dt_utag)+1
 		|| !EV_GET_UINT32(ev, &sizeH, &p, &q)
 		|| !EV_GET_UINT32(ev, &sizeL, &p, &q)) {
 		/* Buffer has wrong length to contain the option values */
@@ -3205,6 +3538,7 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
 #else
 	    desc->read_bufsize = ((size_t)sizeH << 32) | sizeL;
 #endif
+	    dt_i2 = desc->read_bufsize;
 	    TRACE_C('K');
 	    reply_ok(desc);
 	} goto done;
@@ -3216,11 +3550,11 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
     
     } /* switch(command) */
     
-    if (lseek_flush_read(desc, &err) < 0) {
+    if (lseek_flush_read(desc, &err, dt_priv, dt_utag) < 0) {
 	reply_posix_error(desc, err);
 	goto done;
     }
-    if (flush_write_check_error(desc, &err) < 0) {
+    if (flush_write_check_error(desc, &err, dt_priv, dt_utag) < 0) {
 	reply_posix_error(desc, err);
 	goto done;
     } else {
@@ -3238,5 +3572,46 @@ file_outputv(ErlDrvData e, ErlIOVec *ev)
     }
 
  done:
+    if (d != NULL) {
+#ifdef HAVE_DTRACE
+	/*
+	 * If d == NULL, then either:
+	 *    1). There was an error of some sort, or
+	 *    2). The command given to us is actually implemented
+	 *	  by file_output() instead.
+	 *
+	 * Case #1 is probably a TODO item, perhaps?
+	 * Case #2 we definitely don't want to activate a probe.
+	 */
+	d->sched_i1 = dt_priv->thread_num;
+	d->sched_i2 = dt_priv->tag;
+	d->sched_utag[0] = '\0';
+	if (dt_utag != NULL) {
+	    strncpy(d->sched_utag, dt_utag, sizeof(d->sched_utag) - 1);
+	    d->sched_utag[sizeof(d->sched_utag) - 1] = '\0';
+	}
+	DTRACE11(efile_drv_entry, dt_priv->thread_num, dt_priv->tag++,
+		 dt_utag, command, dt_s1, NULL, dt_i1, dt_i2, dt_i3, dt_i4,
+		 desc->port_str);
+#endif
+    }
     cq_execute(desc);
 }
+
+#ifdef  HAVE_DTRACE
+dt_private *
+get_dt_private(int base)
+{
+    dt_private *dt_priv = (dt_private *) pthread_getspecific(dt_driver_key);
+
+    if (dt_priv == NULL) {
+	dt_priv = EF_SAFE_ALLOC(sizeof(dt_private));
+	erts_mtx_lock(&dt_driver_mutex);
+	dt_priv->thread_num = (base + dt_driver_idnum++);
+	erts_mtx_unlock(&dt_driver_mutex);
+	dt_priv->tag = 0;
+	pthread_setspecific(dt_driver_key, dt_priv);
+    }
+    return dt_priv;
+}
+#endif  /* HAVE_DTRACE */
