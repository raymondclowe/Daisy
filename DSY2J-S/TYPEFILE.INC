{ 15/3/88 }

overlay procedure typefile( fn : st14 );
var f : file;
    f2:text;
    tblock: array[1..128] of char;
    line:st255;
    exit :boolean;
    j:integer;
begin
   if (fn > '') and
      (((fn[2]=':')and(fn[1] in ['A'..Highestdrv])) or (fn[2]<>':')) and
      (not EUS) then
   begin
      assign( f, fn);
      {$I-} reset( f ); {$I+}
      if IOResult <> 0 then
         UWriteLn('There is no file '+fn)
      else
      begin
         if logactivity then writeln(TempLog,'. Type file '+fn);
         {$i-} blockread(f,tblock,1); {$i+}
         exit := (ioresult <> 0);
         j := 1;
         while (j < 129) and (tblock[j] <> ^Z) and (not exit) do
         begin
            exit := not (tblock[j] in [^I,esc,bs,^L,cr,lf,' '..'~']);
            inc (j)
         end;

         close(f);
         if not exit then
         begin
            assign( f2, fn);
            reset(f2);
            UInterupted := false;
            uclrscr;
            while (not eof(f2)) and (not eus) and (not UInterupted) do
            begin
               readln(f2,line);
               uwriteln(Line);
               UCtrlSC;
               Monitor
            end;
            close(f2)
         end
         else
            uwriteln('File cannot be typed');
         if uinterupted then flushinputbuffer
      end
   end;
end;

