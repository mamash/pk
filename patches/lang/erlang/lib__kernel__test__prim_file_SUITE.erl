$NetBSD$

--- lib/kernel/test/prim_file_SUITE.erl.orig	2011-05-24 11:16:43.000000000 +0000
+++ lib/kernel/test/prim_file_SUITE.erl
@@ -63,7 +63,7 @@
 %% compile time.
 -define(PRIM_FILE_call(F, H, A), 
 	case H of
-	    [] -> apply(?PRIM_FILE, F, A);
+	    [] -> apply(?PRIM_FILE, F, A -- ["utag"]);
 	    _ ->  apply(?PRIM_FILE, F, [H | A])
 	end).
 
@@ -245,31 +245,31 @@ make_del_dir(Config, Handle, Suffix) ->
     ?line NewDir = filename:join(RootDir, 
 				 atom_to_list(?MODULE)
 				 ++"_mk-dir"++Suffix),
-    ?line ok = ?PRIM_FILE_call(make_dir, Handle, [NewDir]),
-    ?line {error, eexist} = ?PRIM_FILE_call(make_dir, Handle, [NewDir]),
-    ?line ok = ?PRIM_FILE_call(del_dir, Handle, [NewDir]),
-    ?line {error, enoent} = ?PRIM_FILE_call(del_dir, Handle, [NewDir]),
+    ?line ok = ?PRIM_FILE_call(make_dir, Handle, [NewDir, "utag"]),
+    ?line {error, eexist} = ?PRIM_FILE_call(make_dir, Handle, [NewDir, "utag"]),
+    ?line ok = ?PRIM_FILE_call(del_dir, Handle, [NewDir, "utag"]),
+    ?line {error, enoent} = ?PRIM_FILE_call(del_dir, Handle, [NewDir, "utag"]),
 
     % Make sure we are not in a directory directly under test_server
     % as that would result in eacess errors when trying to delere '..',
     % because there are processes having that directory as current.
-    ?line ok = ?PRIM_FILE_call(make_dir, Handle, [NewDir]),
+    ?line ok = ?PRIM_FILE_call(make_dir, Handle, [NewDir, "utag"]),
     ?line {ok, CurrentDir} = ?PRIM_FILE_call(get_cwd, Handle, []),
-    ?line ok = ?PRIM_FILE_call(set_cwd, Handle, [NewDir]),
+    ?line ok = ?PRIM_FILE_call(set_cwd, Handle, [NewDir, "utag"]),
     try
 	%% Check that we get an error when trying to create...
 	%% a deep directory
 	?line NewDir2 = filename:join(RootDir, 
 				      atom_to_list(?MODULE)
 				      ++"_mk-dir-noexist/foo"),
-	?line {error, enoent} = ?PRIM_FILE_call(make_dir, Handle, [NewDir2]),
+	?line {error, enoent} = ?PRIM_FILE_call(make_dir, Handle, [NewDir2, "utag"]),
 	%% a nameless directory
-	?line {error, enoent} = ?PRIM_FILE_call(make_dir, Handle, [""]),
+	?line {error, enoent} = ?PRIM_FILE_call(make_dir, Handle, ["", "utag"]),
 	%% a directory with illegal name
-	?line {error, badarg} = ?PRIM_FILE_call(make_dir, Handle, ['mk-dir']),
+	?line {error, badarg} = ?PRIM_FILE_call(make_dir, Handle, ['mk-dir', "utag"]),
 	
 	%% a directory with illegal name, even if it's a (bad) list
-	?line {error, badarg} = ?PRIM_FILE_call(make_dir, Handle, [[1,2,3,{}]]),
+	?line {error, badarg} = ?PRIM_FILE_call(make_dir, Handle, [[1,2,3,{}], "utag"]),
 	
 	%% Maybe this isn't an error, exactly, but worth mentioning anyway:
 	%% ok = ?PRIM_FILE:make_dir([$f,$o,$o,0,$b,$a,$r])),
@@ -282,16 +282,16 @@ make_del_dir(Config, Handle, Suffix) ->
 	%% Try deleting some bad directories
 	%% Deleting the parent directory to the current, sounds dangerous, huh?
 	%% Don't worry ;-) the parent directory should never be empty, right?
-	?line case ?PRIM_FILE_call(del_dir, Handle, [".."]) of
+	?line case ?PRIM_FILE_call(del_dir, Handle, ["..", "utag"]) of
 		  {error, eexist} -> ok;
 		  {error, einval} -> ok		%FreeBSD
 	      end,
-	?line {error, enoent} = ?PRIM_FILE_call(del_dir, Handle, [""]),
-	?line {error, badarg} = ?PRIM_FILE_call(del_dir, Handle, [[3,2,1,{}]]),
+	?line {error, enoent} = ?PRIM_FILE_call(del_dir, Handle, ["", "utag"]),
+	?line {error, badarg} = ?PRIM_FILE_call(del_dir, Handle, [[3,2,1,{}], "utag"]),
 
 	?line test_server:timetrap_cancel(Dog)
     after
-	?line ok = ?PRIM_FILE_call(set_cwd, Handle, [CurrentDir])
+	?line ok = ?PRIM_FILE_call(set_cwd, Handle, [CurrentDir, "utag"])
     end,
     ok.
 
@@ -313,7 +313,7 @@ cur_dir_0(Config, Handle) ->
     %% Find out the current dir, and cd to it ;-)
     ?line {ok,BaseDir} = ?PRIM_FILE_call(get_cwd, Handle, []),
     ?line Dir1 = BaseDir ++ "", %% Check that it's a string
-    ?line ok = ?PRIM_FILE_call(set_cwd, Handle, [Dir1]),
+    ?line ok = ?PRIM_FILE_call(set_cwd, Handle, [Dir1, "utag"]),
     ?line DirName = atom_to_list(?MODULE) ++
 	case Handle of
 	    [] ->
@@ -325,40 +325,40 @@ cur_dir_0(Config, Handle) ->
     %% Make a new dir, and cd to that
     ?line RootDir = ?config(priv_dir,Config),
     ?line NewDir = filename:join(RootDir, DirName),
-    ?line ok = ?PRIM_FILE_call(make_dir, Handle, [NewDir]),
+    ?line ok = ?PRIM_FILE_call(make_dir, Handle, [NewDir, "utag"]),
     ?line io:format("cd to ~s",[NewDir]),
-    ?line ok = ?PRIM_FILE_call(set_cwd, Handle, [NewDir]),
+    ?line ok = ?PRIM_FILE_call(set_cwd, Handle, [NewDir, "utag"]),
 
     %% Create a file in the new current directory, and check that it
     %% really is created there
     ?line UncommonName = "uncommon.fil",
     ?line {ok,Fd} = ?PRIM_FILE:open(UncommonName, [read, write]),
     ?line ok = ?PRIM_FILE:close(Fd),
-    ?line {ok,NewDirFiles} = ?PRIM_FILE_call(list_dir, Handle, ["."]),
+    ?line {ok,NewDirFiles} = ?PRIM_FILE_call(list_dir, Handle, [".", "utag"]),
     ?line true = lists:member(UncommonName,NewDirFiles),
 
     %% Delete the directory and return to the old current directory
     %% and check that the created file isn't there (too!)
     ?line expect({error, einval}, {error, eacces}, {error, eexist}, 
-		 ?PRIM_FILE_call(del_dir, Handle, [NewDir])),
-    ?line ?PRIM_FILE_call(delete, Handle, [UncommonName]),
-    ?line {ok,[]} = ?PRIM_FILE_call(list_dir, Handle, ["."]),
-    ?line ok = ?PRIM_FILE_call(set_cwd, Handle, [Dir1]),
+		 ?PRIM_FILE_call(del_dir, Handle, [NewDir, "utag"])),
+    ?line ?PRIM_FILE_call(delete, Handle, [UncommonName, "utag"]),
+    ?line {ok,[]} = ?PRIM_FILE_call(list_dir, Handle, [".", "utag"]),
+    ?line ok = ?PRIM_FILE_call(set_cwd, Handle, [Dir1, "utag"]),
     ?line io:format("cd back to ~s",[Dir1]),
-    ?line ok = ?PRIM_FILE_call(del_dir, Handle, [NewDir]),
-    ?line {error, enoent} = ?PRIM_FILE_call(set_cwd, Handle, [NewDir]),
-    ?line ok = ?PRIM_FILE_call(set_cwd, Handle, [Dir1]),
+    ?line ok = ?PRIM_FILE_call(del_dir, Handle, [NewDir, "utag"]),
+    ?line {error, enoent} = ?PRIM_FILE_call(set_cwd, Handle, [NewDir, "utag"]),
+    ?line ok = ?PRIM_FILE_call(set_cwd, Handle, [Dir1, "utag"]),
     ?line io:format("cd back to ~s",[Dir1]),
-    ?line {ok,OldDirFiles} = ?PRIM_FILE_call(list_dir, Handle, ["."]),
+    ?line {ok,OldDirFiles} = ?PRIM_FILE_call(list_dir, Handle, [".", "utag"]),
     ?line false = lists:member(UncommonName,OldDirFiles),
 
     %% Try doing some bad things
     ?line {error, badarg} = 
-	?PRIM_FILE_call(set_cwd, Handle, [{foo,bar}]),
+	?PRIM_FILE_call(set_cwd, Handle, [{foo,bar}, "utag"]),
     ?line {error, enoent} = 
-	?PRIM_FILE_call(set_cwd, Handle, [""]),
+	?PRIM_FILE_call(set_cwd, Handle, ["", "utag"]),
     ?line {error, enoent} = 
-	?PRIM_FILE_call(set_cwd, Handle, [".......a......"]),
+	?PRIM_FILE_call(set_cwd, Handle, [".......a......", "utag"]),
     ?line {ok,BaseDir} = 
 	?PRIM_FILE_call(get_cwd, Handle, []), %% Still there?
 
@@ -394,10 +394,10 @@ cur_dir_1(Config, Handle) ->
     ?line case os:type() of
 	      {unix, _} ->
 		  ?line {error, enotsup} = 
-		      ?PRIM_FILE_call(get_cwd, Handle, ["d:"]);
+		      ?PRIM_FILE_call(get_cwd, Handle, ["d:", "utag"]);
 	      vxworks ->
 		  ?line {error, enotsup} = 
-		      ?PRIM_FILE_call(get_cwd, Handle,  ["d:"]);
+		      ?PRIM_FILE_call(get_cwd, Handle,  ["d:", "utag"]);
 	      {win32, _} ->
 		  win_cur_dir_1(Config, Handle)
 	  end,
@@ -411,7 +411,7 @@ win_cur_dir_1(_Config, Handle) ->
     %% and try to get current directory for that drive.
 
     ?line [Drive, $:|_] = BaseDir,
-    ?line {ok, BaseDir} = ?PRIM_FILE_call(get_cwd, Handle, [[Drive, $:]]),
+    ?line {ok, BaseDir} = ?PRIM_FILE_call(get_cwd, Handle, [[Drive, $:], "utag"]),
     io:format("BaseDir = ~s\n", [BaseDir]),
 
     %% Unfortunately, there is no way to move away from the
@@ -1016,7 +1016,7 @@ file_write_file_info(Config, Handle, Suf
     ?line ok = ?PRIM_FILE:write_file(Name, "hello"),
     ?line Time = {{1997, 01, 02}, {12, 35, 42}},
     ?line Info = #file_info{mode=8#400, atime=Time, mtime=Time, ctime=Time},
-    ?line ok = ?PRIM_FILE_call(write_file_info, Handle, [Name, Info]),
+    ?line ok = ?PRIM_FILE_call(write_file_info, Handle, [Name, Info, "utag"]),
 
     %% Read back the times.
 
@@ -1039,12 +1039,12 @@ file_write_file_info(Config, Handle, Suf
     %% Make the file writable again.
 
     ?line ?PRIM_FILE_call(write_file_info, Handle, 
-			  [Name, #file_info{mode=8#600}]),
+			  [Name, #file_info{mode=8#600}, "utag"]),
     ?line ok = ?PRIM_FILE:write_file(Name, "hello again"),
 
     %% And unwritable.
     ?line ?PRIM_FILE_call(write_file_info, Handle, 
-			  [Name, #file_info{mode=8#400}]),
+			  [Name, #file_info{mode=8#400}, "utag"]),
     ?line {error, eacces} = ?PRIM_FILE:write_file(Name, "hello again"),
 
     %% Write the times again.
@@ -1052,7 +1052,7 @@ file_write_file_info(Config, Handle, Suf
 
     ?line NewTime = {{1997, 02, 15}, {13, 18, 20}},
     ?line NewInfo = #file_info{atime=NewTime, mtime=NewTime, ctime=NewTime},
-    ?line ok = ?PRIM_FILE_call(write_file_info, Handle, [Name, NewInfo]),
+    ?line ok = ?PRIM_FILE_call(write_file_info, Handle, [Name, NewInfo, "utag"]),
     ?line {ok, ActualInfo2} = 
 	?PRIM_FILE_call(read_file_info, Handle, [Name]),
     ?line #file_info{atime=NewActAtime, mtime=NewTime,
@@ -1070,7 +1070,7 @@ file_write_file_info(Config, Handle, Suf
     %% Make the file writeable again, so that we can remove the
     %% test suites ... :-)
     ?line ?PRIM_FILE_call(write_file_info, Handle, 
-			  [Name, #file_info{mode=8#600}]),
+			  [Name, #file_info{mode=8#600}, "utag"]),
     ?line test_server:timetrap_cancel(Dog),
     ok.
 
@@ -1246,11 +1246,11 @@ delete(Config, Handle, Suffix) ->
     %% Check that the file is readable
     ?line {ok, Fd2} = ?PRIM_FILE:open(Name, [read]),
     ?line ok = ?PRIM_FILE:close(Fd2),
-    ?line ok = ?PRIM_FILE_call(delete, Handle, [Name]),
+    ?line ok = ?PRIM_FILE_call(delete, Handle, [Name, "utag"]),
     %% Check that the file is not readable anymore
     ?line {error, _} = ?PRIM_FILE:open(Name, [read]),
     %% Try deleting a nonexistent file
-    ?line {error, enoent} = ?PRIM_FILE_call(delete, Handle, [Name]),
+    ?line {error, enoent} = ?PRIM_FILE_call(delete, Handle, [Name, "utag"]),
     ?line test_server:timetrap_cancel(Dog),
     ok.
 
@@ -1751,14 +1751,14 @@ make_link(Config, Handle, Suffix) ->
     ?line NewDir = filename:join(RootDir, 
 				 atom_to_list(?MODULE)
 				 ++"_make_link"++Suffix),
-    ?line ok = ?PRIM_FILE_call(make_dir, Handle, [NewDir]),
+    ?line ok = ?PRIM_FILE_call(make_dir, Handle, [NewDir, "utag"]),
     
     ?line Name = filename:join(NewDir, "a_file"),
     ?line ok = ?PRIM_FILE:write_file(Name, "some contents\n"),
     
     ?line Alias = filename:join(NewDir, "an_alias"),
     ?line Result = 
-	case ?PRIM_FILE_call(make_link, Handle, [Name, Alias]) of
+	case ?PRIM_FILE_call(make_link, Handle, [Name, Alias, "utag"]) of
 	    {error, enotsup} ->
 		{skipped, "Links not supported on this platform"};
 	    ok ->
@@ -1769,12 +1769,12 @@ make_link(Config, Handle, Suffix) ->
 		%% since they are not used on symbolic links.
 		
 		?line {ok, Info} = 
-		    ?PRIM_FILE_call(read_link_info, Handle, [Name]),
+		    ?PRIM_FILE_call(read_link_info, Handle, [Name, "utag"]),
 		?line {ok, Info} = 
-		    ?PRIM_FILE_call(read_link_info, Handle, [Alias]),
+		    ?PRIM_FILE_call(read_link_info, Handle, [Alias, "utag"]),
 		?line #file_info{links = 2, type = regular} = Info,
 		?line {error, eexist} = 
-		    ?PRIM_FILE_call(make_link, Handle, [Name, Alias]),
+		    ?PRIM_FILE_call(make_link, Handle, [Name, Alias, "utag"]),
 		ok
 	end,
     
@@ -1812,30 +1812,30 @@ symlinks(Config, Handle, Suffix) ->
     ?line NewDir = filename:join(RootDir, 
 				 atom_to_list(?MODULE)
 				 ++"_make_symlink"++Suffix),
-    ?line ok = ?PRIM_FILE_call(make_dir, Handle, [NewDir]),
+    ?line ok = ?PRIM_FILE_call(make_dir, Handle, [NewDir, "utag"]),
     
     ?line Name = filename:join(NewDir, "a_plain_file"),
     ?line ok = ?PRIM_FILE:write_file(Name, "some stupid content\n"),
     
     ?line Alias = filename:join(NewDir, "a_symlink_alias"),
     ?line Result = 
-	case ?PRIM_FILE_call(make_symlink, Handle, [Name, Alias]) of
+	case ?PRIM_FILE_call(make_symlink, Handle, [Name, Alias, "utag"]) of
 	    {error, enotsup} ->
 		{skipped, "Links not supported on this platform"};
 	    ok ->
 		?line {ok, Info1} = 
-		    ?PRIM_FILE_call(read_file_info, Handle, [Name]),
+		    ?PRIM_FILE_call(read_file_info, Handle, [Name, "utag"]),
 		?line {ok, Info1} = 
-		    ?PRIM_FILE_call(read_file_info, Handle, [Alias]),
+		    ?PRIM_FILE_call(read_file_info, Handle, [Alias, "utag"]),
 		?line {ok, Info1} = 
-		    ?PRIM_FILE_call(read_link_info, Handle, [Name]),
+		    ?PRIM_FILE_call(read_link_info, Handle, [Name, "utag"]),
 		?line #file_info{links = 1, type = regular} = Info1,
 		
 		?line {ok, Info2} = 
-		    ?PRIM_FILE_call(read_link_info, Handle, [Alias]),
+		    ?PRIM_FILE_call(read_link_info, Handle, [Alias, "utag"]),
 		?line #file_info{links=1, type=symlink} = Info2,
 		?line {ok, Name} = 
-		    ?PRIM_FILE_call(read_link, Handle, [Alias]),
+		    ?PRIM_FILE_call(read_link, Handle, [Alias, "utag"]),
 		ok
 	end,
     
@@ -1859,7 +1859,7 @@ list_dir_limit(Config) when is_list(Conf
     ?line NewDir = filename:join(RootDir,
 				 atom_to_list(?MODULE)++"_list_dir_limit"),
     ?line {ok, Handle1} = ?PRIM_FILE:start(),
-    ?line ok = ?PRIM_FILE_call(make_dir, Handle1, [NewDir]),
+    ?line ok = ?PRIM_FILE_call(make_dir, Handle1, [NewDir, "utag"]),
     Ref = erlang:start_timer(MaxTime*1000, self(), []),
     ?line Result = list_dir_limit_loop(NewDir, Handle1, Ref, MaxNumber, 0),
     ?line Time = case erlang:cancel_timer(Ref) of
@@ -1910,7 +1910,7 @@ list_dir_limit_loop(Dir, Handle, Ref, N,
     end.
 
 list_dir_check(Dir, Handle, Cnt) ->
-    case ?PRIM_FILE:list_dir(Handle, Dir) of
+    case ?PRIM_FILE:list_dir(Handle, Dir, "utag") of
 	{ok, ListDir} ->
 	    case length(ListDir) of
 		Cnt ->
