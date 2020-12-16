
var scripting : (none,readin,writeout);
    ScriptF : text;
    tempch : st1;

procedure rawFreadln(var l:st80);
begin readln(ScriptF,l)
end;

procedure rawKreadln(var l:st80);
begin readln(l)
end;

procedure readln(var l:st80);
begin

   if scripting = readin then
   begin
      rawFreadln(l);
      writeln(l);
   end
   else
   begin
      rawkreadln(l);
      if scripting = writeout then writeln(ScriptF,l)
   end;

end;

procedure preparescript;
var cmdline : st80;
    i : byte;
begin
   fillchar(cmdline,sizeof(cmdline),0);
   for i := 1 to 80 do
   begin
      if upcase(passing[i]) in ['A'..'Z','0'..'9',':','.'] then
         cmdline := cmdline + upcase(passing[i])
   end;

   scripting := none;
   if cmdline > '' then
   begin
      assign(scriptf,cmdline);
      {$I-} reset(scriptf); {$I+}
      if ioresult = 0 then
      begin
         scripting := readin;
         writeln('Reading script ',cmdline);
      end
      else
      begin
         scripting := writeout;
         rewrite(scriptf);
         writeln('Creating script ',cmdline);
      end
   end;

end;