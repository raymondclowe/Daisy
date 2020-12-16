{ 8/7/88 }

OVERLAY procedure RAWsubmit(fn:st14);
type linetype = array[1..20] of st128;
var f: text;
    f2:file;
    subfn:ST14;
    line:linetype;
    callstring:st128;
    p,l,I,numlines:integer;
    ch:char;
    chainf,executef:boolean;
begin

   if (insession) and (not echoon) and (userlocation = remote) then { warn sysop }
   begin
      gotoxy(1,15);write(ESC,'Y');
      gotoxy(1,19);writeln('File ',fn,' being submited',lf)
   end;

   if cpam then subfn := '$@$.SUB' else subfn := 'A:$$$.SUB';

   { save the time }
   assign(f,generaldrv+':SYSDATA.BBS');
   {$I-}REwrite( f );{$I+}
   readdate;
   writeln(f,presentdate);
   writeln(f,totalcalls);
   close(f);

   { get the sub commands }
   assign(f,fn);
   reset(f);
   l := 0;
   fillchar(line,sizeof(line),0);
   callstring := '';
   while not eof(f) do
   begin
      inc(l);
      readln(f,line[l]);
      line[l] := upstring(line[l]);
      if line[l]='^' then {off hook while event}
      begin
         ModemOffHook;
         dec(l)
      end;
      if callstring = '' then
      begin
         chainf := copy(line[l],1,6)='CHAIN ';
         executef := copy(line[l],1,8)='EXECUTE ';
         if chainf or executef then
         begin
            callstring := copy(line[l],pos(' ',line[l])+1,80)+' ';
            dec(l)
         end
      end;
   end;
   close(f);
   numlines := l;

   { open the sub file }
   assign(f2,subFN);
   if fileexists(subFN) then
   begin
      reset(f2);
      seek(f2,filesize(f2))
   end
   else
      rewrite(f2);

   { write into the sub file }
   for l := numlines downto 1 do
   begin
      LINE[L] := LINE[L] + #0'$';
      blockwrite(f2,LINE[L],1)
   end;

   close(f2);

   if logactivity then writeln(TempLog,'  Submiting '+fn);

   if Logeverything or logcallers or logactivity then updatelog;

   if keypressed then read(kbd,ch); { clear console }
   if chainf or executef then
   begin
      p:= pos(' ',callstring);
      assign(f2,copy(callstring,1,p-1)); { file to be called }
      callstring := copy(callstring,p+1,80); { remove filename }
      fillchar(passing,80,32);
      for i := 1 to length(copy(callstring,1,80)) do { no more than 80 }
         passing[i] := callstring[I];
      if chainf then
         CHAIN(F2)
      else
         execute(f2)
   end
   else
      HALT
end;
