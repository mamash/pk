$NetBSD$

--- lib/kernel/src/file_io_server.erl.orig	2011-10-03 18:12:07.000000000 +0000
+++ lib/kernel/src/file_io_server.erl
@@ -21,7 +21,7 @@
 %% A simple file server for io to one file instance per server instance.
 
 -export([format_error/1]).
--export([start/3, start_link/3]).
+-export([start/4, start_link/4]).
 
 -export([count_and_find/3]).
 
@@ -43,18 +43,18 @@ format_error({_Line, Mod, Reason}) ->
 format_error(ErrorId) ->
     erl_posix_msg:message(ErrorId).
 
-start(Owner, FileName, ModeList) 
+start(Owner, FileName, ModeList, DTraceUtag) 
   when is_pid(Owner), (is_list(FileName) orelse is_binary(FileName)), is_list(ModeList) ->
-    do_start(spawn, Owner, FileName, ModeList).
+    do_start(spawn, Owner, FileName, ModeList, DTraceUtag).
 
-start_link(Owner, FileName, ModeList) 
+start_link(Owner, FileName, ModeList, DTraceUtag) 
   when is_pid(Owner), (is_list(FileName) orelse is_binary(FileName)), is_list(ModeList) ->
-    do_start(spawn_link, Owner, FileName, ModeList).
+    do_start(spawn_link, Owner, FileName, ModeList, DTraceUtag).
 
 %%%-----------------------------------------------------------------
 %%% Server starter, dispatcher and helpers
 
-do_start(Spawn, Owner, FileName, ModeList) ->
+do_start(Spawn, Owner, FileName, ModeList, DTraceUtag) ->
     Self = self(),
     Ref = make_ref(),
     Pid = 
@@ -63,11 +63,12 @@ do_start(Spawn, Owner, FileName, ModeLis
 		  %% process_flag(trap_exit, true),
 		  case parse_options(ModeList) of
 		      {ReadMode, UnicodeMode, Opts} ->
-			  case ?PRIM_FILE:open(FileName, Opts) of
+			  case ?PRIM_FILE:open(FileName, Opts, DTraceUtag) of
 			      {error, Reason} = Error ->
 				  Self ! {Ref, Error},
 				  exit(Reason);
 			      {ok, Handle} ->
+                                  put(dtrace_utag, DTraceUtag), % TODO: API?
 				  %% XXX must I handle R6 nodes here?
 				  M = erlang:monitor(process, Owner),
 				  Self ! {Ref, ok},
