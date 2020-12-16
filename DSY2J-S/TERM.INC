{ 4/11/88 }
overlay procedure term;
var
MIO : byte absolute $E0A8;
MST : byte absolute $E0A9;

MIOchr : char absolute MIO;

var ch:char;
    exit : boolean;

const nc = 'No carrier>';
      ff  = ^L;
var i : integer;
    oe:boolean;

begin
   gotoxy(1,15);write(ESC,'Y');
   mussed := true;
   writeln('Dumb terminal, use <ESC>''?'' for help');

   if keypressed then read(kbd,ch);
   if (MST and $28)=$08 then ch :=MIOchr;
   exit := false;
   repeat
      repeat until ((MST and $28)=$08) or keypressed;
      if not keypressed then
      begin
         ch := MIOchr;
         write(ch);
      end;
      if keypressed then
      begin
         read(kbd,ch);
         if ch = esc then
         begin
            read(kbd,ch);
            case upcase(ch) of
               'Z' : clrscr;
               'X' : begin
                        write('[Exit?]');
                        read(kbd,ch);
                        if ch in ['y','Y'] then
                        begin
                          { save the time }
                          assign(f,generaldrv+':SYSDATA.BBS');
                          {$I-}REwrite( f );{$I+}
                          readdate;
                          writeln(f,presentdate);
                          writeln(f,totalcalls);
                          close(f);
                          halt;
                        end;
                        writeln;
                     end;
               'E' : exit := true;
               '3' : begin write('[300b]'); baudrate := 300; modemsetbaud end;
               '1' : begin write('[1200b]'); baudrate := 1200; modemsetbaud end;
               '2' : begin write('[2400b]'); baudrate := 2400; modemsetbaud end;
               '?' : begin writeln('[E)nd terminal mode, eX)it daisy, Z)clear screen, 3)00,1)200,2)400 baud rate]'); end
            else
               MIOchr := ch;
            end;
         end
         else
         begin
            MIOchr:=ch;

            if (MST and $20)<>$0 then { local echo if no carrier }
            begin
                write(ch);
                if ch = cr then write(lf,nc)
            end
         end
      end
   until exit;
   ModemDirect(''); { just something to poke at the modem more than anything }
   delay(500);
   for i := 1 to 14 do begin gotoxy(1,i); clreol end;
   oe := echoon; modeminit; echoon := oe
end;
