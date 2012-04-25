$NetBSD$

--- lib/kernel/src/file.erl.orig	2011-10-03 18:12:07.000000000 +0000
+++ lib/kernel/src/file.erl
@@ -37,7 +37,7 @@
 -export([ipread_s32bu_p32bu/3]).
 %% Generic file contents.
 -export([open/2, close/1, advise/4,
-	 read/2, write/2, 
+	 read/2, write/2,
 	 pread/2, pread/3, pwrite/2, pwrite/3,
 	 read_line/1,
 	 position/2, truncate/1, datasync/1, sync/1,
@@ -57,7 +57,7 @@
 
 %% Internal export to prim_file and ram_file until they implement
 %% an efficient copy themselves.
--export([copy_opened/3]).
+-export([copy_opened/3, copy_opened/4]).
 
 -export([ipread_s32bu_p32bu_int/3]).
 
@@ -65,6 +65,10 @@
 -export_type([date_time/0, fd/0, file_info/0, filename/0, io_device/0,
 	      name/0, posix/0]).
 
+%% Exports for DTrace-related functions.  They are defined here to avoid
+%% critical dependencies outside of the 'kernel' OTP application.
+-export([put_dtrace_utag/1, get_dtrace_utag/0]).
+
 %%% Includes and defines
 -include("file.hrl").
 
@@ -74,6 +78,8 @@
 -define(PRIM_FILE, prim_file).         % Module
 -define(RAM_FILE, ram_file).           % Module
 
+-define(DTRACE_UT_KEY, '_dtrace_utag_@_@'). % Match prim_file:get_dtrace_utag()!
+
 %% data types
 -type filename()  :: string() | binary().
 -type file_info() :: #file_info{}.
@@ -157,7 +163,7 @@ pid2name(Pid) when is_pid(Pid) ->
       Reason :: posix().
 
 get_cwd() ->
-    call(get_cwd, []).
+    call(get_cwd, [no_drive, get_dtrace_utag()]).
 
 -spec get_cwd(Drive) -> {ok, Dir} | {error, Reason} when
       Drive :: string(),
@@ -165,21 +171,21 @@ get_cwd() ->
       Reason :: posix() | badarg.
 
 get_cwd(Drive) ->
-    check_and_call(get_cwd, [file_name(Drive)]).
+    check_and_call(get_cwd, [file_name(Drive), get_dtrace_utag()]).
 
 -spec set_cwd(Dir) -> ok | {error, Reason} when
       Dir :: name(),
       Reason :: posix() | badarg.
 
 set_cwd(Dirname) -> 
-    check_and_call(set_cwd, [file_name(Dirname)]).
+    check_and_call(set_cwd, [file_name(Dirname), get_dtrace_utag()]).
 
 -spec delete(Filename) -> ok | {error, Reason} when
       Filename :: name(),
       Reason :: posix() | badarg.
 
 delete(Name) ->
-    check_and_call(delete, [file_name(Name)]).
+    check_and_call(delete, [file_name(Name), get_dtrace_utag()]).
 
 -spec rename(Source, Destination) -> ok | {error, Reason} when
       Source :: name(),
@@ -187,21 +193,21 @@ delete(Name) ->
       Reason :: posix() | badarg.
 
 rename(From, To) ->
-    check_and_call(rename, [file_name(From), file_name(To)]).
+    check_and_call(rename, [file_name(From), file_name(To), get_dtrace_utag()]).
 
 -spec make_dir(Dir) -> ok | {error, Reason} when
       Dir :: name(),
       Reason :: posix() | badarg.
 
 make_dir(Name) ->
-    check_and_call(make_dir, [file_name(Name)]).
+    check_and_call(make_dir, [file_name(Name), get_dtrace_utag()]).
 
 -spec del_dir(Dir) -> ok | {error, Reason} when
       Dir :: name(),
       Reason :: posix() | badarg.
 
 del_dir(Name) ->
-    check_and_call(del_dir, [file_name(Name)]).
+    check_and_call(del_dir, [file_name(Name), get_dtrace_utag()]).
 
 -spec read_file_info(Filename) -> {ok, FileInfo} | {error, Reason} when
       Filename :: name(),
@@ -209,12 +215,12 @@ del_dir(Name) ->
       Reason :: posix() | badarg.
 
 read_file_info(Name) ->
-    check_and_call(read_file_info, [file_name(Name)]).
+    check_and_call(read_file_info, [file_name(Name), get_dtrace_utag()]).
 
 -spec altname(Name :: name()) -> any().
 
 altname(Name) ->
-    check_and_call(altname, [file_name(Name)]).
+    check_and_call(altname, [file_name(Name), get_dtrace_utag()]).
 
 -spec read_link_info(Name) -> {ok, FileInfo} | {error, Reason} when
       Name :: name(),
@@ -222,7 +228,7 @@ altname(Name) ->
       Reason :: posix() | badarg.
 
 read_link_info(Name) ->
-    check_and_call(read_link_info, [file_name(Name)]).
+    check_and_call(read_link_info, [file_name(Name), get_dtrace_utag()]).
 
 -spec read_link(Name) -> {ok, Filename} | {error, Reason} when
       Name :: name(),
@@ -230,7 +236,7 @@ read_link_info(Name) ->
       Reason :: posix() | badarg.
 
 read_link(Name) ->
-    check_and_call(read_link, [file_name(Name)]).
+    check_and_call(read_link, [file_name(Name), get_dtrace_utag()]).
 
 -spec write_file_info(Filename, FileInfo) -> ok | {error, Reason} when
       Filename :: name(),
@@ -238,7 +244,7 @@ read_link(Name) ->
       Reason :: posix() | badarg.
 
 write_file_info(Name, Info = #file_info{}) ->
-    check_and_call(write_file_info, [file_name(Name), Info]).
+    check_and_call(write_file_info, [file_name(Name), Info, get_dtrace_utag()]).
 
 -spec list_dir(Dir) -> {ok, Filenames} | {error, Reason} when
       Dir :: name(),
@@ -246,7 +252,7 @@ write_file_info(Name, Info = #file_info{
       Reason :: posix() | badarg.
 
 list_dir(Name) ->
-    check_and_call(list_dir, [file_name(Name)]).
+    check_and_call(list_dir, [file_name(Name), get_dtrace_utag()]).
 
 -spec read_file(Filename) -> {ok, Binary} | {error, Reason} when
       Filename :: name(),
@@ -254,7 +260,7 @@ list_dir(Name) ->
       Reason :: posix() | badarg | terminated | system_limit.
 
 read_file(Name) ->
-    check_and_call(read_file, [file_name(Name)]).
+    check_and_call(read_file, [file_name(Name), get_dtrace_utag()]).
 
 -spec make_link(Existing, New) -> ok | {error, Reason} when
       Existing :: name(),
@@ -262,7 +268,7 @@ read_file(Name) ->
       Reason :: posix() | badarg.
 
 make_link(Old, New) ->
-    check_and_call(make_link, [file_name(Old), file_name(New)]).
+    check_and_call(make_link, [file_name(Old), file_name(New), get_dtrace_utag()]).
 
 -spec make_symlink(Name1, Name2) -> ok | {error, Reason} when
       Name1 :: name(),
@@ -270,7 +276,7 @@ make_link(Old, New) ->
       Reason :: posix() | badarg.
 
 make_symlink(Old, New) ->
-    check_and_call(make_symlink, [file_name(Old), file_name(New)]).
+    check_and_call(make_symlink, [file_name(Old), file_name(New), get_dtrace_utag()]).
 
 -spec write_file(Filename, Bytes) -> ok | {error, Reason} when
       Filename :: name(),
@@ -278,7 +284,7 @@ make_symlink(Old, New) ->
       Reason :: posix() | badarg | terminated | system_limit.
 
 write_file(Name, Bin) ->
-    check_and_call(write_file, [file_name(Name), make_binary(Bin)]).
+    check_and_call(write_file, [file_name(Name), make_binary(Bin), get_dtrace_utag()]).
 
 %% This whole operation should be moved to the file_server and prim_file
 %% when it is time to change file server protocol again.
@@ -363,7 +369,7 @@ open(Item, ModeList) when is_list(ModeLi
 			    [FileName | _] = Args,
 			    %% We rely on the returned Handle (in {ok, Handle})
 			    %% being a pid() or a #file_descriptor{}
-			    ?PRIM_FILE:open(FileName, ModeList);
+			    ?PRIM_FILE:open(FileName, ModeList, get_dtrace_utag());
 			Error ->
 			    Error
 		    end
@@ -384,7 +390,7 @@ open(Item, ModeList) when is_list(ModeLi
 		    case check_args(Args) of 
 			ok ->
 			    [FileName | _] = Args,
-			    call(open, [FileName, ModeList]);
+			    call(open, [FileName, ModeList, get_dtrace_utag()]);
 			Error ->
 			    Error
 		    end
@@ -429,7 +435,10 @@ close(_) ->
 advise(File, Offset, Length, Advise) when is_pid(File) ->
     R = file_request(File, {advise, Offset, Length, Advise}),
     wait_file_reply(File, R);
+advise(#file_descriptor{module = prim_file = Module} = Handle, Offset, Length, Advise) ->
+    Module:advise(Handle, Offset, Length, Advise, get_dtrace_utag());
 advise(#file_descriptor{module = Module} = Handle, Offset, Length, Advise) ->
+    %% DTrace TODO: ram_file and other file drivers not yet DTrace'ified.
     Module:advise(Handle, Offset, Length, Advise);
 advise(_, _, _, _) ->
     {error, badarg}.
@@ -440,17 +449,25 @@ advise(_, _, _, _) ->
       Data :: string() | binary(),
       Reason :: posix() | badarg | terminated.
 
-read(File, Sz) when (is_pid(File) orelse is_atom(File)), is_integer(Sz), Sz >= 0 ->
+read(File, Sz) ->
+    read(File, Sz, get_dtrace_utag()).
+
+read(File, Sz, _DTraceUtag)
+  when (is_pid(File) orelse is_atom(File)), is_integer(Sz), Sz >= 0 ->
     case io:request(File, {get_chars, '', Sz}) of
 	Data when is_list(Data); is_binary(Data) ->
 	    {ok, Data};
 	Other ->
 	    Other
     end;
-read(#file_descriptor{module = Module} = Handle, Sz) 
+read(#file_descriptor{module = prim_file = Module} = Handle, Sz, DTraceUtag)
+  when is_integer(Sz), Sz >= 0 ->
+    Module:read(Handle, Sz, DTraceUtag);
+read(#file_descriptor{module = Module} = Handle, Sz, _DTraceUtag)
   when is_integer(Sz), Sz >= 0 ->
+    %% DTrace TODO: ram_file and other file drivers not yet DTrace'ified.
     Module:read(Handle, Sz);
-read(_, _) ->
+read(_, _, _) ->
     {error, badarg}.
 
 -spec read_line(IoDevice) -> {ok, Data} | eof | {error, Reason} when
@@ -465,7 +482,10 @@ read_line(File) when (is_pid(File) orels
 	Other ->
 	    Other
     end;
+read_line(#file_descriptor{module = prim_file = Module} = Handle) ->
+    Module:read_line(Handle, get_dtrace_utag());
 read_line(#file_descriptor{module = Module} = Handle) ->
+    %% DTrace TODO: ram_file and other file drivers not yet DTrace'ified.
     Module:read_line(Handle);
 read_line(_) ->
     {error, badarg}.
@@ -479,7 +499,10 @@ read_line(_) ->
 
 pread(File, L) when is_pid(File), is_list(L) ->
     pread_int(File, L, []);
+pread(#file_descriptor{module = prim_file = Module} = Handle, L) when is_list(L) ->
+    Module:pread(Handle, L, get_dtrace_utag());
 pread(#file_descriptor{module = Module} = Handle, L) when is_list(L) ->
+    %% DTrace TODO: ram_file and other file drivers not yet DTrace'ified.
     Module:pread(Handle, L);
 pread(_, _) ->
     {error, badarg}.
@@ -511,6 +534,7 @@ pread(File, At, Sz) when is_pid(File), i
     wait_file_reply(File, R);
 pread(#file_descriptor{module = Module} = Handle, Offs, Sz) 
   when is_integer(Sz), Sz >= 0 ->
+    %% DTrace TODO: ram_file and other file drivers not yet DTrace'ified.
     Module:pread(Handle, Offs, Sz);
 pread(_, _, _) ->
     {error, badarg}.
@@ -520,16 +544,22 @@ pread(_, _, _) ->
       Bytes :: iodata(),
       Reason :: posix() | badarg | terminated.
 
-write(File, Bytes) when (is_pid(File) orelse is_atom(File)) ->
+write(File, Bytes) ->
+    write(File, Bytes, get_dtrace_utag()).
+
+write(File, Bytes, _DTraceUtag) when (is_pid(File) orelse is_atom(File)) ->
     case make_binary(Bytes) of
 	Bin when is_binary(Bin) ->
 	    io:request(File, {put_chars,Bin});
 	Error ->
 	    Error
     end;
-write(#file_descriptor{module = Module} = Handle, Bytes) ->
+write(#file_descriptor{module = prim_file = Module} = Handle, Bytes, DTraceUtag) ->
+    Module:write(Handle, Bytes, DTraceUtag);
+write(#file_descriptor{module = Module} = Handle, Bytes, _DTraceUtag) ->
+    %% DTrace TODO: ram_file and other file drivers not yet DTrace'ified.
     Module:write(Handle, Bytes);
-write(_, _) ->
+write(_, _, _) ->
     {error, badarg}.
 
 -spec pwrite(IoDevice, LocBytes) -> ok | {error, {N, Reason}} when
@@ -540,7 +570,10 @@ write(_, _) ->
 
 pwrite(File, L) when is_pid(File), is_list(L) ->
     pwrite_int(File, L, 0);
+pwrite(#file_descriptor{module = prim_file = Module} = Handle, L) when is_list(L) ->
+    Module:pwrite(Handle, L, get_dtrace_utag());
 pwrite(#file_descriptor{module = Module} = Handle, L) when is_list(L) ->
+    %% DTrace TODO: ram_file and other file drivers not yet DTrace'ified.
     Module:pwrite(Handle, L);
 pwrite(_, _) ->
     {error, badarg}.
@@ -567,6 +600,7 @@ pwrite(File, At, Bytes) when is_pid(File
     R = file_request(File, {pwrite, At, Bytes}),
     wait_file_reply(File, R);
 pwrite(#file_descriptor{module = Module} = Handle, Offs, Bytes) ->
+    %% DTrace TODO: ram_file and other file drivers not yet DTrace'ified.
     Module:pwrite(Handle, Offs, Bytes);
 pwrite(_, _, _) ->
     {error, badarg}.
@@ -578,7 +612,10 @@ pwrite(_, _, _) ->
 datasync(File) when is_pid(File) ->
     R = file_request(File, datasync),
     wait_file_reply(File, R);
+datasync(#file_descriptor{module = prim_file = Module} = Handle) ->
+    Module:datasync(Handle, get_dtrace_utag());
 datasync(#file_descriptor{module = Module} = Handle) ->
+    %% DTrace TODO: ram_file and other file drivers not yet DTrace'ified.
     Module:datasync(Handle);
 datasync(_) ->
     {error, badarg}.
@@ -590,7 +627,10 @@ datasync(_) ->
 sync(File) when is_pid(File) ->
     R = file_request(File, sync),
     wait_file_reply(File, R);
+sync(#file_descriptor{module = prim_file = Module} = Handle) ->
+    Module:sync(Handle, get_dtrace_utag());
 sync(#file_descriptor{module = Module} = Handle) ->
+    %% DTrace TODO: ram_file and other file drivers not yet DTrace'ified.
     Module:sync(Handle);
 sync(_) ->
     {error, badarg}.
@@ -604,7 +644,10 @@ sync(_) ->
 position(File, At) when is_pid(File) ->
     R = file_request(File, {position,At}),
     wait_file_reply(File, R);
+position(#file_descriptor{module = prim_file = Module} = Handle, At) ->
+    Module:position(Handle, At, get_dtrace_utag());
 position(#file_descriptor{module = Module} = Handle, At) ->
+    %% DTrace TODO: ram_file and other file drivers not yet DTrace'ified.
     Module:position(Handle, At);
 position(_, _) ->
     {error, badarg}.
@@ -616,7 +659,10 @@ position(_, _) ->
 truncate(File) when is_pid(File) ->
     R = file_request(File, truncate),
     wait_file_reply(File, R);
+truncate(#file_descriptor{module = prim_file = Module} = Handle) ->
+    Module:truncate(Handle, get_dtrace_utag());
 truncate(#file_descriptor{module = Module} = Handle) ->
+    %% DTrace TODO: ram_file and other file drivers not yet DTrace'ified.
     Module:truncate(Handle);
 truncate(_) ->
     {error, badarg}.
@@ -657,7 +703,7 @@ copy_int(Source, Dest, Length) 
   when is_pid(Source), is_pid(Dest);
        is_pid(Source), is_record(Dest, file_descriptor);
        is_record(Source, file_descriptor), is_pid(Dest) ->
-    copy_opened_int(Source, Dest, Length, 0);
+    copy_opened_int(Source, Dest, Length, get_dtrace_utag());
 %% Copy between open raw files, both handled by the same module
 copy_int(#file_descriptor{module = Module} = Source,
 	 #file_descriptor{module = Module} = Dest,
@@ -666,14 +712,14 @@ copy_int(#file_descriptor{module = Modul
 %% Copy between open raw files of different modules
 copy_int(#file_descriptor{} = Source, 
 	 #file_descriptor{} = Dest, Length) ->
-    copy_opened_int(Source, Dest, Length, 0);
+    copy_opened_int(Source, Dest, Length, get_dtrace_utag());
 %% Copy between filenames, let the server do the copy
 copy_int({SourceName, SourceOpts}, {DestName, DestOpts}, Length) 
   when is_list(SourceOpts), is_list(DestOpts) ->
     check_and_call(copy, 
 		   [file_name(SourceName), SourceOpts,
 		    file_name(DestName), DestOpts,
-		    Length]);
+		    Length, get_dtrace_utag()]);
 %% Filename -> open file; must open Source and do client copy
 copy_int({SourceName, SourceOpts}, Dest, Length) 
   when is_list(SourceOpts), is_pid(Dest);
@@ -684,7 +730,8 @@ copy_int({SourceName, SourceOpts}, Dest,
 	Source ->
 	    case open(Source, [read | SourceOpts]) of
 		{ok, Handle} ->
-		    Result = copy_opened_int(Handle, Dest, Length, 0),
+		    Result = copy_opened_int(Handle, Dest, Length,
+                                             get_dtrace_utag()),
 		    close(Handle),
 		    Result;
 		{error, _} = Error ->
@@ -701,7 +748,8 @@ copy_int(Source, {DestName, DestOpts}, L
 	Dest ->
 	    case open(Dest, [write | DestOpts]) of
 		{ok, Handle} ->
-		    Result = copy_opened_int(Source, Handle, Length, 0),
+		    Result = copy_opened_int(Source, Handle, Length,
+                                             get_dtrace_utag()),
 		    close(Handle),
 		    Result;
 		{error, _} = Error ->
@@ -736,45 +784,49 @@ copy_int(Source, Dest, Length) ->
 
 
 
-copy_opened(Source, Dest, Length)
+copy_opened(Source, Dest, Length) ->
+    copy_opened(Source, Dest, Length, get_dtrace_utag()).
+
+copy_opened(Source, Dest, Length, DTraceUtag)
   when is_integer(Length), Length >= 0;
        is_atom(Length) ->
-    copy_opened_int(Source, Dest, Length);
-copy_opened(_, _, _) ->
+    copy_opened_int(Source, Dest, Length, DTraceUtag);
+copy_opened(_, _, _, _) ->
     {error, badarg}.
 
 %% Here we know that Length is either an atom or an integer >= 0
 %% (by the way, atoms > integers)
 
-copy_opened_int(Source, Dest, Length)
+copy_opened_int(Source, Dest, Length, DTraceUtag)
   when is_pid(Source), is_pid(Dest) ->
-    copy_opened_int(Source, Dest, Length, 0);
-copy_opened_int(Source, Dest, Length)
+    copy_opened_int(Source, Dest, Length, 0, DTraceUtag);
+copy_opened_int(Source, Dest, Length, DTraceUtag)
   when is_pid(Source), is_record(Dest, file_descriptor) ->
-    copy_opened_int(Source, Dest, Length, 0);
-copy_opened_int(Source, Dest, Length)
+    copy_opened_int(Source, Dest, Length, 0, DTraceUtag);
+copy_opened_int(Source, Dest, Length, DTraceUtag)
   when is_record(Source, file_descriptor), is_pid(Dest) ->
-    copy_opened_int(Source, Dest, Length, 0);
-copy_opened_int(Source, Dest, Length)
+    copy_opened_int(Source, Dest, Length, 0, DTraceUtag);
+copy_opened_int(Source, Dest, Length, DTraceUtag)
   when is_record(Source, file_descriptor), is_record(Dest, file_descriptor) ->
-    copy_opened_int(Source, Dest, Length, 0);
-copy_opened_int(_, _, _) ->
+    copy_opened_int(Source, Dest, Length, 0, DTraceUtag);
+copy_opened_int(_, _, _, _) ->
     {error, badarg}.
 
 %% Here we know that Source and Dest are handles to open files, Length is
 %% as above, and Copied is an integer >= 0
 
 %% Copy loop in client process
-copy_opened_int(_, _, Length, Copied) when Length =< 0 -> % atom() > integer()
+copy_opened_int(_, _, Length, Copied, _DTraceUtag)
+  when Length =< 0 -> % atom() > integer()
     {ok, Copied};
-copy_opened_int(Source, Dest, Length, Copied) ->
+copy_opened_int(Source, Dest, Length, Copied, DTraceUtag) ->
     N = if Length > 65536 -> 65536; true -> Length end, % atom() > integer() !
-    case read(Source, N) of
+    case read(Source, N, DTraceUtag) of
 	{ok, Data} ->
 	    M = if is_binary(Data) -> byte_size(Data);
 		   is_list(Data)   -> length(Data)
 		end,
-	    case write(Dest, Data) of
+	    case write(Dest, Data, DTraceUtag) of
 		ok ->
 		    if M < N ->
 			    %% Got less than asked for - must be end of file
@@ -784,7 +836,8 @@ copy_opened_int(Source, Dest, Length, Co
 			    NewLength = if is_atom(Length) -> Length;
 					   true         -> Length-M
 					end,
-			    copy_opened_int(Source, Dest, NewLength, Copied+M)
+			    copy_opened_int(Source, Dest, NewLength, Copied+M,
+                                            DTraceUtag)
 		    end;
 		{error, _} = Error ->
 		    Error
@@ -804,6 +857,8 @@ copy_opened_int(Source, Dest, Length, Co
 
 ipread_s32bu_p32bu(File, Pos, MaxSize) when is_pid(File) ->
     ipread_s32bu_p32bu_int(File, Pos, MaxSize);
+ipread_s32bu_p32bu(#file_descriptor{module = prim_file = Module} = Handle, Pos, MaxSize) ->
+    Module:ipread_s32bu_p32bu(Handle, Pos, MaxSize, get_dtrace_utag());
 ipread_s32bu_p32bu(#file_descriptor{module = Module} = Handle, Pos, MaxSize) ->
     Module:ipread_s32bu_p32bu(Handle, Pos, MaxSize);
 ipread_s32bu_p32bu(_, _, _) ->
@@ -1273,3 +1328,24 @@ wait_file_reply(From, Ref) ->
 	    %% receive {'EXIT', From, _} -> ok after 0 -> ok end,
 	    {error, terminated}
     end.
+
+-spec put_dtrace_utag(undefined | iolist() | binary()) -> ok.
+
+put_dtrace_utag(undefined) ->
+    put_dtrace_utag(<<>>);
+put_dtrace_utag(T) when is_binary(T) ->
+    put(?DTRACE_UT_KEY, T),
+    ok;
+put_dtrace_utag(T) when is_list(T) ->
+    put(?DTRACE_UT_KEY, iolist_to_binary(T)),
+    ok.
+
+-spec get_dtrace_utag() -> binary().
+
+get_dtrace_utag() ->
+    case get(?DTRACE_UT_KEY) of
+        undefined ->
+            <<>>;
+        X ->
+            X
+    end.
