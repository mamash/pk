$NetBSD$

--- erts/preloaded/src/prim_file.erl.orig	2011-10-03 18:12:07.000000000 +0000
+++ erts/preloaded/src/prim_file.erl
@@ -25,38 +25,46 @@
 %%% Interface towards a single file's contents. Uses ?FD_DRV.
 
 %% Generic file contents operations
--export([open/2, close/1, datasync/1, sync/1, advise/4, position/2, truncate/1,
-	 write/2, pwrite/2, pwrite/3, read/2, read_line/1, pread/2, pread/3, copy/3]).
+-export([open/2, open/3, close/1, close/2,
+         datasync/1, datasync/2, sync/1, sync/2,
+         advise/4, advise/5,
+         position/2, position/3, truncate/1, truncate/2,
+	 write/2, write/3, pwrite/2, pwrite/3, pwrite/4,
+         read/2, read/3, read_line/1, read_line/2,
+         pread/2, pread/3, pread/4, copy/3, copy/4]).
 
 %% Specialized file operations
--export([open/1, open/3]).
--export([read_file/1, read_file/2, write_file/2]).
--export([ipread_s32bu_p32bu/3]).
+-export([open/1]).
+-export([read_file/1, read_file/2, read_file/3, write_file/2, write_file/3]).
+-export([ipread_s32bu_p32bu/3, ipread_s32bu_p32bu/4]).
 
 
 
 %%% Interface towards file system and metadata. Uses ?DRV.
 
 %% Takes an optional port (opens a ?DRV port per default) as first argument.
--export([get_cwd/0, get_cwd/1, get_cwd/2, 
-	 set_cwd/1, set_cwd/2,
-	 delete/1, delete/2, 
-	 rename/2, rename/3, 
-	 make_dir/1, make_dir/2,
-	 del_dir/1, del_dir/2,
-	 read_file_info/1, read_file_info/2,
-	 altname/1, altname/2,
-	 write_file_info/2, write_file_info/3,
-	 make_link/2, make_link/3,
-	 make_symlink/2, make_symlink/3,
-	 read_link/1, read_link/2,
-	 read_link_info/1, read_link_info/2,
-	 list_dir/1, list_dir/2]).
+-export([get_cwd/0, get_cwd/1, get_cwd/3,
+	 set_cwd/1, set_cwd/3,
+	 delete/1, delete/2, delete/3,
+	 rename/2, rename/3, rename/4,
+	 make_dir/1, make_dir/3,
+	 del_dir/1, del_dir/3,
+	 read_file_info/1, read_file_info/2, read_file_info/3,
+	 altname/1, altname/3,
+	 write_file_info/2, write_file_info/4,
+	 make_link/2, make_link/3, make_link/4,
+	 make_symlink/2, make_symlink/3, make_symlink/4,
+	 read_link/1, read_link/3,
+	 read_link_info/1, read_link_info/3,
+	 list_dir/1, list_dir/3]).
 %% How to start and stop the ?DRV port.
 -export([start/0, stop/1]).
 
 %% Debug exports
--export([open_int/4, open_mode/1, open_mode/4]).
+-export([open_int/4, open_int/5, open_mode/1, open_mode/4]).
+
+%% For DTrace/Systemtap tracing
+-export([get_dtrace_utag/0]).
 
 %%%-----------------------------------------------------------------
 %%% Includes and defines
@@ -152,30 +160,21 @@
 %%% Supposed to be called by applications through module file.
 
 
-%% Opens a file using the driver port Port. Returns {error, Reason}
-%% | {ok, FileDescriptor}
-open(Port, File, ModeList) when is_port(Port), 
-                                (is_list(File) orelse is_binary(File)), 
-                                is_list(ModeList) ->
-    case open_mode(ModeList) of
-	{Mode, _Portopts, _Setopts} ->
-	    open_int(Port, File, Mode, []);
-	Reason ->
-	    {error, Reason}
-    end;
-open(_,_,_) ->
-    {error, badarg}.
-
 %% Opens a file. Returns {error, Reason} | {ok, FileDescriptor}.
-open(File, ModeList) when (is_list(File) orelse is_binary(File)), 
-			  is_list(ModeList) ->
+open(File, ModeList) ->
+    open(File, ModeList, get_dtrace_utag()).
+
+open(File, ModeList, DTraceUtag)
+  when (is_list(File) orelse is_binary(File)), 
+       is_list(ModeList),
+       (is_list(DTraceUtag) orelse is_binary(DTraceUtag)) ->
     case open_mode(ModeList) of
 	{Mode, Portopts, Setopts} ->
-	    open_int({?FD_DRV, Portopts},File, Mode, Setopts);
+	    open_int({?FD_DRV, Portopts}, File, Mode, Setopts, DTraceUtag);
 	Reason ->
 	    {error, Reason}
     end;
-open(_, _) ->
+open(_, _, _) ->
     {error, badarg}.
 
 %% Opens a port that can be used for open/3 or read_file/2.
@@ -190,29 +189,34 @@ open(Portopts) when is_list(Portopts) ->
 open(_) ->
     {error, badarg}.
 
-open_int({Driver, Portopts}, File, Mode, Setopts) ->
+open_int(Arg, File, Mode, Setopts) ->
+    open_int(Arg, File, Mode, Setopts, get_dtrace_utag()).
+
+open_int({Driver, Portopts}, File, Mode, Setopts, DTraceUtag) ->
+    %% TODO: add DTraceUtag to drv_open()?
     case drv_open(Driver, Portopts) of
 	{ok, Port} ->
-	    open_int(Port, File, Mode, Setopts);
+	    open_int(Port, File, Mode, Setopts, DTraceUtag);
 	{error, _} = Error ->
 	    Error
     end;
-open_int(Port, File, Mode, Setopts) ->
+open_int(Port, File, Mode, Setopts, DTraceUtag) ->
     M = Mode band ?EFILE_MODE_MASK,
-    case drv_command(Port, [<<?FILE_OPEN, M:32>>, pathname(File)]) of
+    case drv_command(Port, [<<?FILE_OPEN, M:32>>,
+                            pathname(File), enc_utag(DTraceUtag)]) of
 	{ok, Number} ->
-	    open_int_setopts(Port, Number, Setopts);
+	    open_int_setopts(Port, Number, Setopts, DTraceUtag);
 	Error ->
 	    drv_close(Port),
 	    Error
     end.
 
-open_int_setopts(Port, Number, []) ->
+open_int_setopts(Port, Number, [], _DTraceUtag) ->
     {ok, #file_descriptor{module = ?MODULE, data = {Port, Number}}};    
-open_int_setopts(Port, Number, [Cmd | Tail]) ->
-    case drv_command(Port, Cmd) of
+open_int_setopts(Port, Number, [Cmd | Tail], DTraceUtag) ->
+    case drv_command(Port, [Cmd, enc_utag(DTraceUtag)]) of
 	ok ->
-	    open_int_setopts(Port, Number, Tail);
+	    open_int_setopts(Port, Number, Tail, DTraceUtag);
 	Error ->
 	    drv_close(Port),
 	    Error
@@ -222,50 +226,64 @@ open_int_setopts(Port, Number, [Cmd | Ta
 
 %% Returns ok.
 
-close(#file_descriptor{module = ?MODULE, data = {Port, _}}) ->
-    case drv_command(Port, <<?FILE_CLOSE>>) of
+close(Arg) ->
+    close(Arg, get_dtrace_utag()).
+
+close(#file_descriptor{module = ?MODULE, data = {Port, _}}, DTraceUtag)
+  when (is_list(DTraceUtag) orelse is_binary(DTraceUtag)) ->
+    case drv_command(Port, [<<?FILE_CLOSE>>, enc_utag(DTraceUtag)]) of
 	ok ->
 	    drv_close(Port);
 	Error ->
 	    Error
     end;
 %% Closes a port opened with open/1.
-close(Port) when is_port(Port) ->
+close(Port, _DTraceUtag) when is_port(Port) ->
     drv_close(Port).
 
--define(ADVISE(Offs, Len, Adv),
+-define(ADVISE(Offs, Len, Adv, BUtag),
 	<<?FILE_ADVISE, Offs:64/signed, Len:64/signed,
-	  Adv:32/signed>>).
+	  Adv:32/signed, BUtag/binary>>).
 
 %% Returns {error, Reason} | ok.
+advise(FD, Offset, Length, Advise) ->
+    advise(FD, Offset, Length, Advise, get_dtrace_utag()).
+
 advise(#file_descriptor{module = ?MODULE, data = {Port, _}},
-       Offset, Length, Advise) ->
+       Offset, Length, Advise, DTraceUtag)
+  when (is_list(DTraceUtag) orelse is_binary(DTraceUtag)) ->
+    BUtag = term_to_binary(enc_utag(DTraceUtag)),
     case Advise of
 	normal ->
-	    Cmd = ?ADVISE(Offset, Length, ?POSIX_FADV_NORMAL),
+	    Cmd = ?ADVISE(Offset, Length, ?POSIX_FADV_NORMAL, BUtag),
 	    drv_command(Port, Cmd);
 	random ->
-	    Cmd = ?ADVISE(Offset, Length, ?POSIX_FADV_RANDOM),
+	    Cmd = ?ADVISE(Offset, Length, ?POSIX_FADV_RANDOM, BUtag),
 	    drv_command(Port, Cmd);
 	sequential ->
-	    Cmd = ?ADVISE(Offset, Length, ?POSIX_FADV_SEQUENTIAL),
+	    Cmd = ?ADVISE(Offset, Length, ?POSIX_FADV_SEQUENTIAL, BUtag),
 	    drv_command(Port, Cmd);
 	will_need ->
-	    Cmd = ?ADVISE(Offset, Length, ?POSIX_FADV_WILLNEED),
+	    Cmd = ?ADVISE(Offset, Length, ?POSIX_FADV_WILLNEED, BUtag),
 	    drv_command(Port, Cmd);
 	dont_need ->
-	    Cmd = ?ADVISE(Offset, Length, ?POSIX_FADV_DONTNEED),
+	    Cmd = ?ADVISE(Offset, Length, ?POSIX_FADV_DONTNEED, BUtag),
 	    drv_command(Port, Cmd);
 	no_reuse ->
-	    Cmd = ?ADVISE(Offset, Length, ?POSIX_FADV_NOREUSE),
+	    Cmd = ?ADVISE(Offset, Length, ?POSIX_FADV_NOREUSE, BUtag),
 	    drv_command(Port, Cmd);
 	_ ->
 	    {error, einval}
     end.
 
 %% Returns {error, Reason} | ok.
-write(#file_descriptor{module = ?MODULE, data = {Port, _}}, Bytes) ->
-    case drv_command(Port, [?FILE_WRITE,Bytes]) of
+write(Desc, Bytes) ->
+    write(Desc, Bytes, get_dtrace_utag()).
+
+write(#file_descriptor{module = ?MODULE, data = {Port, _}}, Bytes, DTraceUtag)
+  when (is_list(DTraceUtag) orelse is_binary(DTraceUtag)) ->
+    %% This is rare case where DTraceUtag is not at end of command list.
+    case drv_command(Port, [?FILE_WRITE,enc_utag(DTraceUtag),Bytes]) of
 	{ok, _Size} ->
 	    ok;
 	Error ->
@@ -275,39 +293,40 @@ write(#file_descriptor{module = ?MODULE,
 %% Returns ok | {error, {WrittenCount, Reason}}
 pwrite(#file_descriptor{module = ?MODULE, data = {Port, _}}, L)
   when is_list(L) ->
-    pwrite_int(Port, L, 0, [], []).
+    pwrite_int(Port, L, 0, [], [], get_dtrace_utag()).
 
-pwrite_int(_, [], 0, [], []) ->
+pwrite_int(_, [], 0, [], [], _DTraceUtag) ->
     ok;
-pwrite_int(Port, [], N, Spec, Data) ->
-    Header = list_to_binary([<<?FILE_PWRITEV, N:32>> | reverse(Spec)]),
+pwrite_int(Port, [], N, Spec, Data, DTraceUtag) ->
+    Header = list_to_binary([<<?FILE_PWRITEV>>, enc_utag(DTraceUtag),
+                             <<N:32>>, reverse(Spec)]),
     case drv_command_raw(Port, [Header | reverse(Data)]) of
 	{ok, _Size} ->
 	    ok;
 	Error ->
 	    Error
     end;
-pwrite_int(Port, [{Offs, Bytes} | T], N, Spec, Data)
+pwrite_int(Port, [{Offs, Bytes} | T], N, Spec, Data, DTraceUtag)
   when is_integer(Offs) ->
     if
 	-(?LARGEFILESIZE) =< Offs, Offs < ?LARGEFILESIZE ->
-	    pwrite_int(Port, T, N, Spec, Data, Offs, Bytes);
+	    pwrite_int(Port, T, N, Spec, Data, Offs, Bytes, DTraceUtag);
 	true ->
 	    {error, einval}
     end;
-pwrite_int(_, [_|_], _N, _Spec, _Data) ->
+pwrite_int(_, [_|_], _N, _Spec, _Data, _DTraceUtag) ->
     {error, badarg}.
 
-pwrite_int(Port, T, N, Spec, Data, Offs, Bin)
+pwrite_int(Port, T, N, Spec, Data, Offs, Bin, DTraceUtag)
   when is_binary(Bin) ->
     Size = byte_size(Bin),
     pwrite_int(Port, T, N+1, 
 	       [<<Offs:64/signed, Size:64>> | Spec], 
-	       [Bin | Data]);
-pwrite_int(Port, T, N, Spec, Data, Offs, Bytes) ->
+	       [Bin | Data], DTraceUtag);
+pwrite_int(Port, T, N, Spec, Data, Offs, Bytes, DTraceUtag) ->
     try list_to_binary(Bytes) of
 	Bin ->
-	    pwrite_int(Port, T, N, Spec, Data, Offs, Bin)
+	    pwrite_int(Port, T, N, Spec, Data, Offs, Bin, DTraceUtag)
     catch
 	error:Reason ->
 	    {error, Reason}
@@ -316,11 +335,28 @@ pwrite_int(Port, T, N, Spec, Data, Offs,
 
 
 %% Returns {error, Reason} | ok.
-pwrite(#file_descriptor{module = ?MODULE, data = {Port, _}}, Offs, Bytes) 
+pwrite(#file_descriptor{module = ?MODULE, data = {Port, _}}, L, DTraceUtag)
+  when is_list(L),
+       (is_list(DTraceUtag) orelse is_binary(DTraceUtag)) ->
+    pwrite_int(Port, L, 0, [], [], DTraceUtag);
+
+pwrite(#file_descriptor{module = ?MODULE, data = {Port, _}}, Offs, Bytes)
   when is_integer(Offs) ->
+    pwrite_int2(Port, Offs, Bytes, get_dtrace_utag());
+pwrite(#file_descriptor{module = ?MODULE}, _, _) ->
+    {error, badarg}.
+
+pwrite(#file_descriptor{module = ?MODULE, data = {Port, _}}, Offs, Bytes, DTraceUtag) 
+  when is_integer(Offs),
+       (is_list(DTraceUtag) orelse is_binary(DTraceUtag)) ->
+    pwrite_int2(Port, Offs, Bytes, DTraceUtag);
+pwrite(#file_descriptor{module = ?MODULE}, _, _, _DTraceUtag) ->
+    {error, badarg}.
+
+pwrite_int2(Port, Offs, Bytes, DTraceUtag) ->
     if
 	-(?LARGEFILESIZE) =< Offs, Offs < ?LARGEFILESIZE ->
-	    case pwrite_int(Port, [], 0, [], [], Offs, Bytes) of
+	    case pwrite_int(Port, [], 0, [], [], Offs, Bytes, DTraceUtag) of
 		{error, {_, Reason}} ->
 		    {error, Reason};
 		Result ->
@@ -328,22 +364,30 @@ pwrite(#file_descriptor{module = ?MODULE
 	    end;
 	true ->
 	    {error, einval}
-    end;
-pwrite(#file_descriptor{module = ?MODULE}, _, _) ->
-    {error, badarg}.
-
+    end.
 
 %% Returns {error, Reason} | ok.
-datasync(#file_descriptor{module = ?MODULE, data = {Port, _}}) ->
-    drv_command(Port, [?FILE_FDATASYNC]).
+datasync(FD) ->
+    datasync(FD, get_dtrace_utag()).
+
+datasync(#file_descriptor{module = ?MODULE, data = {Port, _}}, DTraceUtag)
+  when (is_list(DTraceUtag) orelse is_binary(DTraceUtag)) ->
+    drv_command(Port, [?FILE_FDATASYNC, enc_utag(DTraceUtag)]).
 
 %% Returns {error, Reason} | ok.
-sync(#file_descriptor{module = ?MODULE, data = {Port, _}}) ->
-    drv_command(Port, [?FILE_FSYNC]).
+sync(FD) ->
+    sync(FD, get_dtrace_utag()).
+
+sync(#file_descriptor{module = ?MODULE, data = {Port, _}}, DTraceUtag)
+  when (is_list(DTraceUtag) orelse is_binary(DTraceUtag)) ->
+    drv_command(Port, [?FILE_FSYNC, enc_utag(DTraceUtag)]).
 
 %% Returns {ok, Data} | eof | {error, Reason}.
-read_line(#file_descriptor{module = ?MODULE, data = {Port, _}}) ->
-    case drv_command(Port, <<?FILE_READ_LINE>>) of
+read_line(FD) ->
+    read_line(FD, get_dtrace_utag()).
+
+read_line(#file_descriptor{module = ?MODULE, data = {Port, _}}, DTraceUtag) ->
+    case drv_command(Port, [<<?FILE_READ_LINE>>, enc_utag(DTraceUtag)]) of
 	{ok, {0, _Data}} ->
 	    eof;
 	{ok, {_Size, Data}} ->
@@ -363,11 +407,17 @@ read_line(#file_descriptor{module = ?MOD
     end.
 	
 %% Returns {ok, Data} | eof | {error, Reason}.
-read(#file_descriptor{module = ?MODULE, data = {Port, _}}, Size)
-  when is_integer(Size), 0 =< Size ->
+read(FD, Size) ->
+    read(FD, Size, get_dtrace_utag()).
+
+read(#file_descriptor{module = ?MODULE, data = {Port, _}}, Size, DTraceUtag)
+  when is_integer(Size),
+       0 =< Size,
+       (is_list(DTraceUtag) orelse is_binary(DTraceUtag)) ->
     if
 	Size < ?LARGEFILESIZE ->
-	    case drv_command(Port, <<?FILE_READ, Size:64>>) of
+	    case drv_command(Port, [<<?FILE_READ, Size:64>>,
+                                    enc_utag(DTraceUtag)]) of
 		{ok, {0, _Data}} when Size =/= 0 ->
 		    eof;
 		{ok, {_Size, Data}} ->
@@ -376,7 +426,8 @@ read(#file_descriptor{module = ?MODULE, 
 		    %% Garbage collecting here might help if
 		    %% the current processes have some old binaries left.
 		    erlang:garbage_collect(),
-		    case drv_command(Port, <<?FILE_READ, Size:64>>) of
+		    case drv_command(Port, [<<?FILE_READ, Size:64>>,
+                                            enc_utag(DTraceUtag)]) of
 			{ok, {0, _Data}} when Size =/= 0 ->
 			    eof;
 			{ok, {_Size, Data}} ->
@@ -394,35 +445,43 @@ read(#file_descriptor{module = ?MODULE, 
 %% Returns {ok, [Data|eof, ...]} | {error, Reason}
 pread(#file_descriptor{module = ?MODULE, data = {Port, _}}, L)
   when is_list(L) ->
-    pread_int(Port, L, 0, []).
+    pread_int(Port, L, 0, [], get_dtrace_utag()).
 
-pread_int(_, [], 0, []) ->
+pread_int(_, [], 0, [], _DTraceUtag) ->
     {ok, []};
-pread_int(Port, [], N, Spec) ->
-    drv_command(Port, [<<?FILE_PREADV, 0:32, N:32>> | reverse(Spec)]);
-pread_int(Port, [{Offs, Size} | T], N, Spec)
+pread_int(Port, [], N, Spec, DTraceUtag) ->
+    drv_command(Port, [<<?FILE_PREADV>>, enc_utag(DTraceUtag),
+                       <<0:32, N:32>>, reverse(Spec)]);
+pread_int(Port, [{Offs, Size} | T], N, Spec, DTraceUtag)
   when is_integer(Offs), is_integer(Size), 0 =< Size ->
     if
 	-(?LARGEFILESIZE) =< Offs, Offs < ?LARGEFILESIZE,
 	Size < ?LARGEFILESIZE ->
-	    pread_int(Port, T, N+1, [<<Offs:64/signed, Size:64>> | Spec]);
+	    pread_int(Port, T, N+1, [<<Offs:64/signed, Size:64>> | Spec],
+                      DTraceUtag);
 	true ->
 	    {error, einval}
     end;
-pread_int(_, [_|_], _N, _Spec) ->
+pread_int(_, [_|_], _N, _Spec, _DTraceUtag) ->
     {error, badarg}.
 
-
-
 %% Returns {ok, Data} | eof | {error, Reason}.
-pread(#file_descriptor{module = ?MODULE, data = {Port, _}}, Offs, Size) 
+pread(#file_descriptor{module = ?MODULE, data = {Port, _}}, L, DTraceUtag)
+  when is_list(L),
+       (is_list(DTraceUtag) orelse is_binary(DTraceUtag)) ->
+    pread_int(Port, L, 0, [], get_dtrace_utag());
+pread(FD, Offs, Size) 
   when is_integer(Offs), is_integer(Size), 0 =< Size ->
+    pread(FD, Offs, Size, get_dtrace_utag()).
+
+pread(#file_descriptor{module = ?MODULE, data = {Port, _}}, Offs, Size, DTraceUtag)
+    when (is_list(DTraceUtag) orelse is_binary(DTraceUtag)) ->
     if
 	-(?LARGEFILESIZE) =< Offs, Offs < ?LARGEFILESIZE,
 	Size < ?LARGEFILESIZE ->
 	    case drv_command(Port, 
-			     <<?FILE_PREADV, 0:32, 1:32,
-			      Offs:64/signed, Size:64>>) of
+			     [<<?FILE_PREADV>>, enc_utag(DTraceUtag),
+                              <<0:32, 1:32, Offs:64/signed, Size:64>>]) of
 		{ok, [eof]} ->
 		    eof;
 		{ok, [Data]} ->
@@ -433,17 +492,22 @@ pread(#file_descriptor{module = ?MODULE,
 	true ->
 	    {error, einval}
     end;
-pread(#file_descriptor{module = ?MODULE, data = {_, _}}, _, _) ->
+pread(_, _, _, _) ->
     {error, badarg}.
 
 
 
 %% Returns {ok, Position} | {error, Reason}.
-position(#file_descriptor{module = ?MODULE, data = {Port, _}}, At) ->
+position(FD, At) ->
+    position(FD, At, get_dtrace_utag()).
+
+position(#file_descriptor{module = ?MODULE, data = {Port, _}}, At, DTraceUtag)
+  when (is_list(DTraceUtag) orelse is_binary(DTraceUtag)) ->
     case lseek_position(At) of
 	{Offs, Whence}
 	when -(?LARGEFILESIZE) =< Offs, Offs < ?LARGEFILESIZE ->
-	    drv_command(Port, <<?FILE_LSEEK, Offs:64/signed, Whence:32>>);
+	    drv_command(Port, [<<?FILE_LSEEK, Offs:64/signed, Whence:32>>,
+                               enc_utag(DTraceUtag)]);
 	{_, _} ->
 	    {error, einval};
 	Reason ->
@@ -451,63 +515,89 @@ position(#file_descriptor{module = ?MODU
     end.
 
 %% Returns {error, Reaseon} | ok.
-truncate(#file_descriptor{module = ?MODULE, data = {Port, _}}) ->
-    drv_command(Port, <<?FILE_TRUNCATE>>).
+truncate(FD) ->
+    truncate(FD, get_dtrace_utag()).
+
+truncate(#file_descriptor{module = ?MODULE, data = {Port, _}}, DTraceUtag)
+  when (is_list(DTraceUtag) orelse is_binary(DTraceUtag)) ->
+    drv_command(Port, [<<?FILE_TRUNCATE>>, enc_utag(DTraceUtag)]).
 
 
 
 %% Returns {error, Reason} | {ok, BytesCopied}
+copy(Source, Dest, Length) ->
+    copy(Source, Dest, Length, get_dtrace_utag()).
+
 copy(#file_descriptor{module = ?MODULE} = Source,
      #file_descriptor{module = ?MODULE} = Dest,
-     Length)
+     Length, DTraceUtag)
   when is_integer(Length), Length >= 0;
-       is_atom(Length) ->
+       is_atom(Length),
+       (is_list(DTraceUtag) orelse is_binary(DTraceUtag)) ->
     %% XXX Should be moved down to the driver for optimization.
-    file:copy_opened(Source, Dest, Length).
+    file:copy_opened(Source, Dest, Length, DTraceUtag).
+
 
 
+ipread_s32bu_p32bu(FD, Offs, Arg) ->
+    ipread_s32bu_p32bu(FD, Offs, Arg, get_dtrace_utag()).
 
 ipread_s32bu_p32bu(#file_descriptor{module = ?MODULE,
 				    data = {_, _}} = Handle,
 		   Offs,
-		   Infinity) when is_atom(Infinity) ->
+		   Infinity,
+                   DTraceUtag)
+  when is_atom(Infinity),
+       (is_list(DTraceUtag) orelse is_binary(DTraceUtag)) ->
     ipread_s32bu_p32bu(Handle, Offs, (1 bsl 31)-1);
 ipread_s32bu_p32bu(#file_descriptor{module = ?MODULE, data = {Port, _}},
 		   Offs,
-		   MaxSize)
-  when is_integer(Offs), is_integer(MaxSize) ->
+		   MaxSize,
+                   DTraceUtag)
+  when is_integer(Offs),
+       is_integer(MaxSize),
+       (is_list(DTraceUtag) orelse is_binary(DTraceUtag)) ->
     if
 	-(?LARGEFILESIZE) =< Offs, Offs < ?LARGEFILESIZE,
 	0 =< MaxSize, MaxSize < (1 bsl 31) ->
-	    drv_command(Port, <<?FILE_IPREAD, ?IPREAD_S32BU_P32BU,
-			       Offs:64, MaxSize:32>>);
+	    drv_command(Port, [<<?FILE_IPREAD, ?IPREAD_S32BU_P32BU,
+                                 Offs:64, MaxSize:32>>, enc_utag(DTraceUtag)]);
 	true ->
 	    {error, einval}
     end;
 ipread_s32bu_p32bu(#file_descriptor{module = ?MODULE, data = {_, _}},
 		   _Offs,
-		   _MaxSize) ->
+		   _MaxSize,
+                   _DTraceUtag) ->
     {error, badarg}.
 
 
 
 %% Returns {ok, Contents} | {error, Reason}
 read_file(File) when (is_list(File) orelse is_binary(File)) ->
+    read_file(File, get_dtrace_utag());
+read_file(_) ->
+    {error, badarg}.
+
+read_file(File, DTraceUtag)
+  when (is_list(File) orelse is_binary(File)),
+       (is_list(DTraceUtag) orelse is_binary(DTraceUtag))->
     case drv_open(?FD_DRV, [binary]) of
 	{ok, Port} ->
-	    Result = read_file(Port, File),
+	    Result = read_file(Port, File, DTraceUtag),
 	    close(Port),
 	    Result;
 	{error, _} = Error ->
 	    Error
     end;
-read_file(_) ->
+read_file(_, _) ->
     {error, badarg}.
 
 %% Takes a Port opened with open/1.
-read_file(Port, File) when is_port(Port),
+read_file(Port, File, DTraceUtag) when is_port(Port),
 			   (is_list(File) orelse is_binary(File)) ->
-    Cmd = [?FILE_READ_FILE | pathname(File)],
+    Cmd = [?FILE_READ_FILE |
+           list_to_binary([pathname(File), enc_utag(DTraceUtag)])],
     case drv_command(Port, Cmd) of
 	{error, enomem} ->
 	    %% It could possibly help to do a 
@@ -519,22 +609,30 @@ read_file(Port, File) when is_port(Port)
 	Result ->
 	    Result
     end;
-read_file(_,_) ->
+read_file(_,_,_) ->
     {error, badarg}.
 
     
 
 %% Returns {error, Reason} | ok.
-write_file(File, Bin) when (is_list(File) orelse is_binary(File)) ->
+write_file(File, Bin) ->
+    write_file(File, Bin, get_dtrace_utag()).
+
+write_file(File, Bin, DTraceUtag)
+  when (is_list(File) orelse is_binary(File)),
+       (is_list(DTraceUtag) orelse is_binary(DTraceUtag)) ->
+    OldUtag = put(dtrace_utag, DTraceUtag),     % TODO: API?
     case open(File, [binary, write]) of
 	{ok, Handle} ->
 	    Result = write(Handle, Bin),
 	    close(Handle),
+            put(dtrace_utag, OldUtag),
 	    Result;
 	Error ->
+            put(dtrace_utag, OldUtag),
 	    Error
     end;
-write_file(_, _) -> 
+write_file(_, _, _) ->
     {error, badarg}.
     
 
@@ -574,54 +672,56 @@ stop(Port) when is_port(Port) ->
 
 
 
-%% get_cwd/{0,1,2}
+%% get_cwd/{0,1,3}
 
 get_cwd() ->
-    get_cwd_int(0).
+    get_cwd_int(0, get_dtrace_utag()).
 
 get_cwd(Port) when is_port(Port) ->
-    get_cwd_int(Port, 0);
+    get_cwd_int(Port, 0, get_dtrace_utag());
 get_cwd([]) ->
-    get_cwd_int(0);
+    get_cwd_int(0, get_dtrace_utag());
 get_cwd([Letter, $: | _]) when $a =< Letter, Letter =< $z ->
-    get_cwd_int(Letter - $a + 1);
+    get_cwd_int(Letter - $a + 1, get_dtrace_utag());
 get_cwd([Letter, $: | _]) when $A =< Letter, Letter =< $Z ->
-    get_cwd_int(Letter - $A + 1);
+    get_cwd_int(Letter - $A + 1, get_dtrace_utag());
 get_cwd([_|_]) ->
     {error, einval};
 get_cwd(_) ->
     {error, badarg}.
 
-get_cwd(Port, []) when is_port(Port) ->
-    get_cwd_int(Port, 0);
-get_cwd(Port, [Letter, $: | _])
+get_cwd(Port, [], DTraceUtag) when is_port(Port) ->
+    get_cwd_int(Port, 0, DTraceUtag);
+get_cwd(Port, no_drive, DTraceUtag) when is_port(Port) ->
+    get_cwd_int(Port, 0, DTraceUtag);
+get_cwd(Port, [Letter, $: | _], DTraceUtag)
   when is_port(Port), $a =< Letter, Letter =< $z ->
-    get_cwd_int(Port, Letter - $a + 1);
-get_cwd(Port, [Letter, $: | _])
+    get_cwd_int(Port, Letter - $a + 1, DTraceUtag);
+get_cwd(Port, [Letter, $: | _], DTraceUtag)
   when is_port(Port), $A =< Letter, Letter =< $Z ->
-    get_cwd_int(Port, Letter - $A + 1);
-get_cwd(Port, [_|_]) when is_port(Port) ->
+    get_cwd_int(Port, Letter - $A + 1, DTraceUtag);
+get_cwd(Port, [_|_], _DTraceUtag) when is_port(Port) ->
     {error, einval};
-get_cwd(_, _) ->
+get_cwd(_, _, _DTraceUtag) ->
     {error, badarg}.
 
-get_cwd_int(Drive) ->
-    get_cwd_int({?DRV, [binary]}, Drive).
+get_cwd_int(Drive, DTraceUtag) ->
+    get_cwd_int({?DRV, [binary]}, Drive, DTraceUtag).
 
-get_cwd_int(Port, Drive) ->
-    drv_command(Port, <<?FILE_PWD, Drive>>).
+get_cwd_int(Port, Drive, DTraceUtag) ->
+    drv_command(Port, list_to_binary([?FILE_PWD, Drive, enc_utag(DTraceUtag)])).
 
 
 
-%% set_cwd/{1,2}
+%% set_cwd/{1,3}
 
 set_cwd(Dir) ->
-    set_cwd_int({?DRV, [binary]}, Dir).
+    set_cwd_int({?DRV, [binary]}, Dir, get_dtrace_utag()).
 
-set_cwd(Port, Dir) when is_port(Port) ->
-    set_cwd_int(Port, Dir).
+set_cwd(Port, Dir, DTraceUtag) when is_port(Port) ->
+    set_cwd_int(Port, Dir, DTraceUtag).
 
-set_cwd_int(Port, Dir0) ->
+set_cwd_int(Port, Dir0, DTraceUtag) ->
     Dir = 
 	(catch
 	 case os:type() of
@@ -631,7 +731,7 @@ set_cwd_int(Port, Dir0) ->
 		 %% must call get_cwd from here and use
 		 %% absname/2, since
 		 %% absname/1 uses file:get_cwd ...
-		 case get_cwd_int(Port, 0) of
+		 case get_cwd_int(Port, 0, "") of
 		     {ok, AbsPath} ->
 			 filename:absname(Dir0, AbsPath);
 		     _Badcwd ->
@@ -642,91 +742,99 @@ set_cwd_int(Port, Dir0) ->
 	 end),
     %% Dir is now either a string or an EXIT tuple.
     %% An EXIT tuple will fail in the following catch.
-    drv_command(Port, [?FILE_CHDIR, pathname(Dir)]).
+    drv_command(Port, [?FILE_CHDIR, pathname(Dir), enc_utag(DTraceUtag)]).
 
 
 
-%% delete/{1,2}
+%% delete/{1,2,3}
 
 delete(File) ->
-    delete_int({?DRV, [binary]}, File).
+    delete_int({?DRV, [binary]}, File, get_dtrace_utag()).
 
 delete(Port, File) when is_port(Port) ->
-    delete_int(Port, File).
+    delete_int(Port, File, get_dtrace_utag()).
+
+delete(Port, File, DTraceUtag) when is_port(Port) ->
+    delete_int(Port, File, DTraceUtag).
 
-delete_int(Port, File) ->
-    drv_command(Port, [?FILE_DELETE, pathname(File)]).
+delete_int(Port, File, DTraceUtag) ->
+    drv_command(Port, [?FILE_DELETE, pathname(File), enc_utag(DTraceUtag)]).
 
 
 
-%% rename/{2,3}
+%% rename/{2,3,4}
 
 rename(From, To) ->
-    rename_int({?DRV, [binary]}, From, To).
+    rename_int({?DRV, [binary]}, From, To, get_dtrace_utag()).
 
 rename(Port, From, To) when is_port(Port) ->
-    rename_int(Port, From, To).
+    rename_int(Port, From, To, get_dtrace_utag()).
 
-rename_int(Port, From, To) ->
-    drv_command(Port, [?FILE_RENAME, pathname(From), pathname(To)]).
+rename(Port, From, To, DTraceUtag) when is_port(Port) ->
+    rename_int(Port, From, To, DTraceUtag).
 
+rename_int(Port, From, To, DTraceUtag) ->
+    drv_command(Port, [?FILE_RENAME, pathname(From), pathname(To),
+                       enc_utag(DTraceUtag)]).
 
 
-%% make_dir/{1,2}
+
+%% make_dir/{1,3}
 
 make_dir(Dir) ->
-    make_dir_int({?DRV, [binary]}, Dir).
+    make_dir_int({?DRV, [binary]}, Dir, get_dtrace_utag()).
 
-make_dir(Port, Dir) when is_port(Port) ->
-    make_dir_int(Port, Dir).
+make_dir(Port, Dir, DTraceUtag) when is_port(Port) ->
+    make_dir_int(Port, Dir, DTraceUtag).
 
-make_dir_int(Port, Dir) ->
-    drv_command(Port, [?FILE_MKDIR, pathname(Dir)]).
+make_dir_int(Port, Dir, DTraceUtag) ->
+    drv_command(Port, [?FILE_MKDIR, pathname(Dir), enc_utag(DTraceUtag)]).
 
 
 
-%% del_dir/{1,2}
+%% del_dir/{1,3}
 
 del_dir(Dir) ->
-    del_dir_int({?DRV, [binary]}, Dir).
-
-del_dir(Port, Dir) when is_port(Port) ->
-    del_dir_int(Port, Dir).
-
-del_dir_int(Port, Dir) ->
-    drv_command(Port, [?FILE_RMDIR, pathname(Dir)]).
+    del_dir_int({?DRV, [binary]}, Dir, get_dtrace_utag()).
 
+del_dir(Port, Dir, DTraceUtag) when is_port(Port) ->
+    del_dir_int(Port, Dir, DTraceUtag).
 
+del_dir_int(Port, Dir, DTraceUtag) ->
+    drv_command(Port, [?FILE_RMDIR, pathname(Dir), enc_utag(DTraceUtag)]).
 
-%% read_file_info/{1,2}
+%% read_file_info/{1,2,3}
 
 read_file_info(File) ->
-    read_file_info_int({?DRV, [binary]}, File).
+    read_file_info_int({?DRV, [binary]}, File, get_dtrace_utag()).
 
 read_file_info(Port, File) when is_port(Port) ->
-    read_file_info_int(Port, File).
+    read_file_info_int(Port, File, get_dtrace_utag()).
 
-read_file_info_int(Port, File) ->
-    drv_command(Port, [?FILE_FSTAT, pathname(File)]).
+read_file_info(Port, File, DTraceUtag) when is_port(Port) ->
+    read_file_info_int(Port, File, DTraceUtag).
 
-%% altname/{1,2}
+read_file_info_int(Port, File, DTraceUtag) ->
+    drv_command(Port, [?FILE_FSTAT, pathname(File), enc_utag(DTraceUtag)]).
+
+%% altname/{1,3}
 
 altname(File) ->
-    altname_int({?DRV, [binary]}, File).
+    altname_int({?DRV, [binary]}, File, get_dtrace_utag()).
 
-altname(Port, File) when is_port(Port) ->
-    altname_int(Port, File).
+altname(Port, File, DTraceUtag) when is_port(Port) ->
+    altname_int(Port, File, DTraceUtag).
 
-altname_int(Port, File) ->
-    drv_command(Port, [?FILE_ALTNAME, pathname(File)]).
+altname_int(Port, File, DTraceUtag) ->
+    drv_command(Port, [?FILE_ALTNAME, pathname(File), enc_utag(DTraceUtag)]).
 
-%% write_file_info/{2,3}
+%% write_file_info/{2,4}
 
 write_file_info(File, Info) ->
-    write_file_info_int({?DRV, [binary]}, File, Info).
+    write_file_info_int({?DRV, [binary]}, File, Info, get_dtrace_utag()).
 
-write_file_info(Port, File, Info) when is_port(Port) ->
-    write_file_info_int(Port, File, Info).
+write_file_info(Port, File, Info, DTraceUtag) when is_port(Port) ->
+    write_file_info_int(Port, File, Info, DTraceUtag).
 
 write_file_info_int(Port, 
 		    File, 
@@ -735,7 +843,8 @@ write_file_info_int(Port, 
 			       gid=Gid,
 			       atime=Atime0, 
 			       mtime=Mtime0, 
-			       ctime=Ctime}) ->
+			       ctime=Ctime},
+                    DTraceUtag) ->
     {Atime, Mtime} =
 	case {Atime0, Mtime0} of
 	    {undefined, Mtime0} -> {erlang:localtime(), Mtime0};
@@ -749,72 +858,81 @@ write_file_info_int(Port, 
 			date_to_bytes(Atime), 
 			date_to_bytes(Mtime), 
 			date_to_bytes(Ctime),
-			pathname(File)]).
+			pathname(File),
+                        enc_utag(DTraceUtag)]).
 
 
 
-%% make_link/{2,3}
+%% make_link/{2,3,4}
 
 make_link(Old, New) ->
-    make_link_int({?DRV, [binary]}, Old, New).
+    make_link_int({?DRV, [binary]}, Old, New, get_dtrace_utag()).
 
 make_link(Port, Old, New) when is_port(Port) ->
-    make_link_int(Port, Old, New).
+    make_link_int(Port, Old, New, get_dtrace_utag()).
+
+make_link(Port, Old, New, DTraceUtag) when is_port(Port) ->
+    make_link_int(Port, Old, New, DTraceUtag).
 
-make_link_int(Port, Old, New) ->
-    drv_command(Port, [?FILE_LINK, pathname(Old), pathname(New)]).
+make_link_int(Port, Old, New, DTraceUtag) ->
+    drv_command(Port, [?FILE_LINK, pathname(Old), pathname(New),
+                       enc_utag(DTraceUtag)]).
 
 
 
-%% make_symlink/{2,3}
+%% make_symlink/{2,3,4}
 
 make_symlink(Old, New) ->
-    make_symlink_int({?DRV, [binary]}, Old, New).
+    make_symlink_int({?DRV, [binary]}, Old, New, get_dtrace_utag()).
 
 make_symlink(Port, Old, New) when is_port(Port) ->
-    make_symlink_int(Port, Old, New).
+    make_symlink_int(Port, Old, New, get_dtrace_utag()).
+
+make_symlink(Port, Old, New, DTraceUtag) when is_port(Port) ->
+    make_symlink_int(Port, Old, New, DTraceUtag).
 
-make_symlink_int(Port, Old, New) ->
-    drv_command(Port, [?FILE_SYMLINK, pathname(Old), pathname(New)]).
+make_symlink_int(Port, Old, New, DTraceUtag) ->
+    drv_command(Port, [?FILE_SYMLINK, pathname(Old), pathname(New),
+                       enc_utag(DTraceUtag)]).
 
 
 
-%% read_link/{2,3}
+%% read_link/{1,3}
 
 read_link(Link) ->
-    read_link_int({?DRV, [binary]}, Link).
+    read_link_int({?DRV, [binary]}, Link, get_dtrace_utag()).
 
-read_link(Port, Link) when is_port(Port) ->
-    read_link_int(Port, Link).
+read_link(Port, Link, DTraceUtag) when is_port(Port) ->
+    read_link_int(Port, Link, DTraceUtag).
 
-read_link_int(Port, Link) ->
-    drv_command(Port, [?FILE_READLINK, pathname(Link)]).
+read_link_int(Port, Link, DTraceUtag) ->
+    drv_command(Port, [?FILE_READLINK, pathname(Link), enc_utag(DTraceUtag)]).
 
 
 
-%% read_link_info/{2,3}
+%% read_link_info/{1,3}
 
 read_link_info(Link) ->
-    read_link_info_int({?DRV, [binary]}, Link).
+    read_link_info_int({?DRV, [binary]}, Link, get_dtrace_utag()).
 
-read_link_info(Port, Link) when is_port(Port) ->
-    read_link_info_int(Port, Link).
+read_link_info(Port, Link, DTraceUtag) when is_port(Port) ->
+    read_link_info_int(Port, Link, DTraceUtag).
 
-read_link_info_int(Port, Link) ->
-    drv_command(Port, [?FILE_LSTAT, pathname(Link)]).
+read_link_info_int(Port, Link, DTraceUtag) ->
+    drv_command(Port, [?FILE_LSTAT, pathname(Link), enc_utag(DTraceUtag)]).
 
 
 
-%% list_dir/{1,2}
+%% list_dir/{1,3}
 
 list_dir(Dir) ->
-    list_dir_int({?DRV, [binary]}, Dir).
+    list_dir_int({?DRV, [binary]}, Dir, get_dtrace_utag()).
 
-list_dir(Port, Dir) when is_port(Port) ->
-    list_dir_int(Port, Dir).
+list_dir(Port, Dir, DTraceUtag) when is_port(Port) ->
+    list_dir_int(Port, Dir, DTraceUtag).
 
-list_dir_int(Port, Dir) ->
-    drv_command(Port, [?FILE_READDIR, pathname(Dir)], []).
+list_dir_int(Port, Dir, DTraceUtag) ->
+    drv_command(Port, [?FILE_READDIR, pathname(Dir), enc_utag(DTraceUtag)], []).
 
 
 
@@ -1230,3 +1348,16 @@ reverse(L, T) -> lists:reverse(L, T).
 % in list_to_binary, which is caught and generates the {error,badarg} return
 pathname(File) ->
     (catch prim_file:internal_name2native(File)).
+
+get_dtrace_utag() ->
+    %% We cannot call file:get_dtrace_utag() because this is prim_file.erl.
+    %% We must reimplement it here.
+    case get('_dtrace_utag_@_@') of
+        undefined ->
+            <<>>;
+        X ->
+            X
+    end.
+
+enc_utag(UTag) ->
+    [UTag, 0].
