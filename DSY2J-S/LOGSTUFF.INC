{ 25/6/88 }

{ Copy temporary log onto the end of Main Log }
procedure UpdateLog;
type pfile = file;
var f: ^pfile;
    mainlog : ^pfile;
    l:^st128;
begin
   { write out remaining to temp }
   close(templog);  { TP will pad with SUBs }
   new(l);

   { open main file }
   new(mainlog);
   assign(mainlog^,systemLogFN);
   reset(Mainlog^);
   if filesize(mainlog^) > 0 then
      seek(MainLog^,filesize(MainLog^)-1); {position to last block in MainLog}

   { begining of temp }
   new(f);
   assign(f^,templogfn);
   reset(f^);

   { copy temp onto end of main (overwrite last block of main because
     that is duplicated as the first block of temp) }
   while not eof(f^) do
   begin
      blockread(f^,l^[1],1);
      blockwrite(MainLog^,l^[1],1);
   end;
   close(MainLog^);
   dispose(mainlog);

   { clear temp }
   close(f^);       { <---------\ }
   dispose(f);                 { same file name, musn't be open at same time! }
   rewrite(TempLog);{ <---------/ }

   { start temp with last block of main }
   l^[0] := chr(128);
   while l^[ord(l^[0])] = sub do
      l^[0] := pred(l^[0]);
   write(templog,l^);
   dispose(l)

end;
