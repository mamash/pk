$NetBSD$

--- lib/kernel/src/file_server.erl.orig	2011-05-24 11:16:43.000000000 +0000
+++ lib/kernel/src/file_server.erl
@@ -76,6 +76,7 @@ stop() -> 
 
 init([]) ->
     process_flag(trap_exit, true),
+    put(dtrace_utag, atom_to_list(?FILE_SERVER)),
     case ?PRIM_FILE:start() of
 	{ok, Handle} ->
 	    ets:new(?FILE_IO_SERVER_TABLE, [named_table]),
@@ -99,9 +100,9 @@ init([]) ->
 	{'reply', 'eof' | 'ok' | {'error', term()} | {'ok', term()}, state()} |
 	{'stop', 'normal', 'stopped', state()}.
 
-handle_call({open, Name, ModeList}, {Pid, _Tag} = _From, Handle)
+handle_call({open, Name, ModeList, DTraceUtag}, {Pid, _Tag} = _From, Handle)
   when is_list(ModeList) ->
-    Child = ?FILE_IO_SERVER:start_link(Pid, Name, ModeList),
+    Child = ?FILE_IO_SERVER:start_link(Pid, Name, ModeList, DTraceUtag),
     case Child of
 	{ok, P} when is_pid(P) ->
 	    ets:insert(?FILE_IO_SERVER_TABLE, {P, Name});
@@ -110,78 +111,80 @@ handle_call({open, Name, ModeList}, {Pid
     end,
     {reply, Child, Handle};
 
-handle_call({open, _Name, _Mode}, _From, Handle) ->
+handle_call({open, _Name, _Mode, _DTraceUtag}, _From, Handle) ->
     {reply, {error, einval}, Handle};
 
-handle_call({read_file, Name}, _From, Handle) ->
-    {reply, ?PRIM_FILE:read_file(Name), Handle};
+handle_call({read_file, Name, DTraceUtag}, _From, Handle) ->
+    {reply, ?PRIM_FILE:read_file(Name, DTraceUtag), Handle};
 
-handle_call({write_file, Name, Bin}, _From, Handle) ->
-    {reply, ?PRIM_FILE:write_file(Name, Bin), Handle};
+handle_call({write_file, Name, Bin, DTraceUtag}, _From, Handle) ->
+    {reply, ?PRIM_FILE:write_file(Name, Bin, DTraceUtag), Handle};
 
-handle_call({set_cwd, Name}, _From, Handle) ->
-    {reply, ?PRIM_FILE:set_cwd(Handle, Name), Handle};
+handle_call({set_cwd, Name, DTraceUtag}, _From, Handle) ->
+    {reply, ?PRIM_FILE:set_cwd(Handle, Name, DTraceUtag), Handle};
 
-handle_call({delete, Name}, _From, Handle) ->
-    {reply, ?PRIM_FILE:delete(Handle, Name), Handle};
+handle_call({delete, Name, DTraceUtag}, _From, Handle) ->
+    {reply, ?PRIM_FILE:delete(Handle, Name, DTraceUtag), Handle};
 
-handle_call({rename, Fr, To}, _From, Handle) ->
-    {reply, ?PRIM_FILE:rename(Handle, Fr, To), Handle};
+handle_call({rename, Fr, To, DTraceUtag}, _From, Handle) ->
+    {reply, ?PRIM_FILE:rename(Handle, Fr, To, DTraceUtag), Handle};
 
-handle_call({make_dir, Name}, _From, Handle) ->
-    {reply, ?PRIM_FILE:make_dir(Handle, Name), Handle};
+handle_call({make_dir, Name, DTraceUtag}, _From, Handle) ->
+    {reply, ?PRIM_FILE:make_dir(Handle, Name, DTraceUtag), Handle};
 
-handle_call({del_dir, Name}, _From, Handle) ->
-    {reply, ?PRIM_FILE:del_dir(Handle, Name), Handle};
+handle_call({del_dir, Name, DTraceUtag}, _From, Handle) ->
+    {reply, ?PRIM_FILE:del_dir(Handle, Name, DTraceUtag), Handle};
 
-handle_call({list_dir, Name}, _From, Handle) ->
-    {reply, ?PRIM_FILE:list_dir(Handle, Name), Handle};
+handle_call({list_dir, Name, DTraceUtag}, _From, Handle) ->
+    {reply, ?PRIM_FILE:list_dir(Handle, Name, DTraceUtag), Handle};
 
 handle_call(get_cwd, _From, Handle) ->
-    {reply, ?PRIM_FILE:get_cwd(Handle), Handle};
-handle_call({get_cwd}, _From, Handle) ->
-    {reply, ?PRIM_FILE:get_cwd(Handle), Handle};
-handle_call({get_cwd, Name}, _From, Handle) ->
-    {reply, ?PRIM_FILE:get_cwd(Handle, Name), Handle};
+    {reply, ?PRIM_FILE:get_cwd(Handle, no_drive, "TODO-fixme"), Handle};
+handle_call({get_cwd, no_drive, DTraceUtag}, _From, Handle) ->
+    {reply, ?PRIM_FILE:get_cwd(Handle, no_drive, DTraceUtag), Handle};
+handle_call({get_cwd, Name, DTraceUtag}, _From, Handle) ->
+    {reply, ?PRIM_FILE:get_cwd(Handle, Name, DTraceUtag), Handle};
 
-handle_call({read_file_info, Name}, _From, Handle) ->
-    {reply, ?PRIM_FILE:read_file_info(Handle, Name), Handle};
+handle_call({read_file_info, Name, DTraceUtag}, _From, Handle) ->
+    {reply, ?PRIM_FILE:read_file_info(Handle, Name, DTraceUtag), Handle};
 
-handle_call({altname, Name}, _From, Handle) ->
-    {reply, ?PRIM_FILE:altname(Handle, Name), Handle};
+handle_call({altname, Name, DTraceUtag}, _From, Handle) ->
+    {reply, ?PRIM_FILE:altname(Handle, Name, DTraceUtag), Handle};
 
-handle_call({write_file_info, Name, Info}, _From, Handle) ->
-    {reply, ?PRIM_FILE:write_file_info(Handle, Name, Info), Handle};
+handle_call({write_file_info, Name, Info, DTraceUtag}, _From, Handle) ->
+    {reply, ?PRIM_FILE:write_file_info(Handle, Name, Info, DTraceUtag), Handle};
 
-handle_call({read_link_info, Name}, _From, Handle) ->
-    {reply, ?PRIM_FILE:read_link_info(Handle, Name), Handle};
+handle_call({read_link_info, Name, DTraceUtag}, _From, Handle) ->
+    {reply, ?PRIM_FILE:read_link_info(Handle, Name, DTraceUtag), Handle};
 
-handle_call({read_link, Name}, _From, Handle) ->
-    {reply, ?PRIM_FILE:read_link(Handle, Name), Handle};
+handle_call({read_link, Name, DTraceUtag}, _From, Handle) ->
+    {reply, ?PRIM_FILE:read_link(Handle, Name, DTraceUtag), Handle};
 
-handle_call({make_link, Old, New}, _From, Handle) ->
-    {reply, ?PRIM_FILE:make_link(Handle, Old, New), Handle};
+handle_call({make_link, Old, New, DTraceUtag}, _From, Handle) ->
+    {reply, ?PRIM_FILE:make_link(Handle, Old, New, DTraceUtag), Handle};
 
-handle_call({make_symlink, Old, New}, _From, Handle) ->
-    {reply, ?PRIM_FILE:make_symlink(Handle, Old, New), Handle};
+handle_call({make_symlink, Old, New, DTraceUtag}, _From, Handle) ->
+    {reply, ?PRIM_FILE:make_symlink(Handle, Old, New, DTraceUtag), Handle};
 
-handle_call({copy, SourceName, SourceOpts, DestName, DestOpts, Length},
+handle_call({copy, SourceName, SourceOpts, DestName, DestOpts, Length, DTraceUtag},
 	    _From, Handle) ->
     Reply = 
-	case ?PRIM_FILE:open(SourceName, [read, binary | SourceOpts]) of
+	case ?PRIM_FILE:open(SourceName, [read, binary | SourceOpts],
+                             DTraceUtag) of
 	    {ok, Source} ->
 		SourceReply = 
 		    case ?PRIM_FILE:open(DestName, 
-					 [write, binary | DestOpts]) of
+					 [write, binary | DestOpts],
+                                         DTraceUtag) of
 			{ok, Dest} ->
 			    DestReply = 
-				?PRIM_FILE:copy(Source, Dest, Length),
-			    ?PRIM_FILE:close(Dest),
+				?PRIM_FILE:copy(Source, Dest, Length, DTraceUtag),
+			    ?PRIM_FILE:close(Dest, DTraceUtag),
 			    DestReply;
 			{error, _} = Error ->
 			    Error
 		    end,
-		?PRIM_FILE:close(Source),
+		?PRIM_FILE:close(Source, DTraceUtag),
 		SourceReply;
 	    {error, _} = Error ->
 		Error
