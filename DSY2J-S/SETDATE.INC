{ 16/6/88 }

OVERLAY procedure setsystemdate;
label exit;
type soc = set of char;
var tempdate : ^st15;
    tempcharset : ^soc;
    i : byte;
    f : text;
begin
   new(tempdate);
   uclrscr;
   readdate;
   Uwriteln('Present date is '+presentdate);
   if clock or (user.priv < setdatepriv) then goto exit;

   new(tempcharset);
   tempcharSet^ := seperatorSet; seperatorSet := [];
   askstrip := false;
   tempdate^ := namecase(ask('New date      : ',15));
   seperatorSet := tempcharSet^;
   dispose(tempcharset);
   askstrip := true;

   if (tempdate^ <> '') and (length(tempdate^)=15) then
      if askyn('Set date to   : '+tempdate^,'Y')='Y' then
      begin
         for i := 1 to 15 do
            presentdate[I] := tempdate^[I];
         assign(f,generaldrv+':SYSDATA.BBS');
         {$I-}REwrite( f );{$I+}
         writeln(f,presentdate);
         writeln(f,totalcalls);
         close(f);
         if logactivity then writeln(TempLog,'. Set date');
      end;
   Uwriteln('Present date is '+presentdate);
   exit:;
   dispose(tempdate);
end;

