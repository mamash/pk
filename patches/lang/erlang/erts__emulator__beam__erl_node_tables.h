$NetBSD$

--- erts/emulator/beam/erl_node_tables.h.orig	2011-10-03 18:12:07.000000000 +0000
+++ erts/emulator/beam/erl_node_tables.h
@@ -169,6 +169,7 @@ extern Sint erts_no_of_not_connected_dis
 
 extern DistEntry *erts_this_dist_entry;
 extern ErlNode *erts_this_node;
+extern char *erts_this_node_sysname; /* must match erl_node_tables.c */
 
 DistEntry *erts_channel_no_to_dist_entry(Uint);
 DistEntry *erts_sysname_to_connected_dist_entry(Eterm);
