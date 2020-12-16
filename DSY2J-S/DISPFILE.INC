{ 26/9/88 }

overlay procedure displayfile( fn : st14 ); {with change }
var f : ^text;
    Line:^st80;
    exit, lf : boolean;
    i,err:integer;
    answer:^st80;
begin
   new(line);
   new(f);
   if (fn > '') and (not EUS) then
   begin
      assign( f^, fn);
      {$I-} reset( f^ ); {$I+}
      if IOResult <> 0 then
         UWriteLn('There is no file '+fn)
      else
      begin
         UInterupted := false;
         exit := false; lf := true;
{         uclrscr;}
         while (not eof(f^)) and (not eus) and (not UInterupted) and (not exit) do
         begin
            line^:=' '; { to prevent @ hangover }
            readln(f^,Line^);
            if line^[1] = '@' then
            begin
                if Line^='@' then
                   exit := true
                else
                begin
                 case upcase(Line^[2]) of
                  '-' : paginate := false;
                  '+' : paginate := user.more;
                  'L' : lf := false;
                  'M' : Uinterupted := (AskYN(copy(Line^,3,maxint),'Y') = 'N');
                  'C' : Uclrscr;
                  'P' : begin
val(copy(Line^,3,3),i,err);
if (err = 0) and (user.priv<i) then {not enough priv}
begin
   if (length(Line^)>5) and (copy(Line^,6,1) = 'S') then
      readln(f^,Line^) {skip a line}
   else
      exit := true; {exit}
end
                        end; {'P'}
                  'E' : if logeverything or logcallers or logactivity then
                           writeln(templog,'* ',copy(line^,3,maxint));
                  'A' : if logeverything or logcallers or logactivity then
                        begin
                           seperatorset := [];
                           askstrip := false;
                           new(answer);
                           answer^:= ask(copy(line^,3,maxint),80);
                           writeln(templog,answer^);
                           dispose(answer);
                           askstrip := true;
                           seperatorset := [' ',';'];
                        end;
                 end; {case}
                end {if line^='@'}
            end
            else
            begin
               uwrite(Line^);
               if lf then
                  uwriteln('')
               else
                  lf := true; { clear flag }
               UCtrlSC;
               Monitor
            end;
            monitor
         end;
         if uinterupted then flushinputbuffer
      end;
      close(f^)
   end;
   dispose(line);
   dispose(f);
end;