$NetBSD$

--- erts/emulator/beam/erl_node_tables.c.orig	2011-10-03 18:12:07.000000000 +0000
+++ erts/emulator/beam/erl_node_tables.c
@@ -26,6 +26,7 @@
 #include "dist.h"
 #include "big.h"
 #include "error.h"
+#include "dtrace-wrapper.h"
 
 Hash erts_dist_table;
 Hash erts_node_table;
@@ -41,6 +42,8 @@ Sint erts_no_of_not_connected_dist_entri
 
 DistEntry *erts_this_dist_entry;
 ErlNode *erts_this_node;
+char erts_this_node_sysname_BUFFER[256],
+    *erts_this_node_sysname = "uninitialized yet";
 
 static Uint node_entries;
 static Uint dist_entries;
@@ -701,6 +704,9 @@ erts_set_this_node(Eterm sysname, Uint c
     (void) hash_erase(&erts_node_table, (void *) erts_this_node);
     erts_this_node->sysname = sysname;
     erts_this_node->creation = creation;
+    erts_this_node_sysname = erts_this_node_sysname_BUFFER;
+    erts_snprintf(erts_this_node_sysname, sizeof(erts_this_node_sysname),
+                  "%T", sysname);
     (void) hash_put(&erts_node_table, (void *) erts_this_node);
 
     erts_smp_rwmtx_rwunlock(&erts_dist_table_rwmtx);
@@ -788,6 +794,9 @@ void erts_init_node_tables(void)
     erts_this_node->sysname			= am_Noname;
     erts_this_node->creation			= 0;
     erts_this_node->dist_entry			= erts_this_dist_entry;
+    erts_this_node_sysname = erts_this_node_sysname_BUFFER;
+    erts_snprintf(erts_this_node_sysname, sizeof(erts_this_node_sysname),
+                  "%T", erts_this_node->sysname);
 
     (void) hash_put(&erts_node_table, (void *) erts_this_node);
 
