{ 27/9/88 }

OVERLAY procedure Change;
label skipmenu;
var temppass : st20;
    tempwidth : St3;
    num, error : integer;
    command : st1;

begin

   repeat
      if inputbuffer > '' then goto skipmenu;
      Uclrscr;
      uinterupted := false;
      UWriteln('----- Change User -----');
      UWriteln('User name  : '+User.Name);
      if uinterupted then goto skipmenu;
      UWriteln('P)assword  : '+User.password);
      str( User.help, uwTemp );
      Uwriteln('Help L)evel: '+uwtemp);
      if uinterupted then goto skipmenu;
      UWriteln('----- Your Screen -----');
      str( User.nuls, uwTemp );
      UWriteln('N)ulls     : '+uwTemp);
      if uinterupted then goto skipmenu;
      str( User.consolewidth, uwTemp );
      UWriteln('W)idth     : '+uwTemp+' columns');
      str( User.consolehight, uwTemp );
      UWriteln('H)eight    : '+uwTemp+' rows');
      if uinterupted then goto skipmenu;
      UWrite  ('T)erminal  : '); case user.consoletype of dumb : uwriteln('Dumb');
                                                          soroc : uwriteln('Soroc');
                                                          ansi : uwriteln('ANSI');
                                                          datamedia : uwriteln('DataMedia') end;
      UWrite('"M)ore?"   : O'); if user.more then uwriteln('n') else uwriteln('ff');
      Uwrite('S)ingle key: O'); if user.singlekey then uwriteln('n') else uwriteln('ff');
skipmenu:
      repeat
         case user.help of
          3,2 : command := upstring( Ask('CHANGE: P L N W H T M S Q or ? for help : ',1));
          1   : command := upstring( Ask('CHANGE: ',1));
         end;
         monitor;
      until (command[1] in ['P','L','N','W','H','T','M','S','Q','?']) or ( command = '' ) or (eus);
      if command > '' then
      begin
         Case Command[1] of
          'P' : begin
temppass := NameCase(Ask('New password: ',20));
monitor;
if not eus then
begin
   if length(temppass) > 3 then
      User.password := temppass
   else
      UWriteLn('Too short')
end;
UWriteln('Your password is "'+User.password+'"')
                end;
          'L' : begin
repeat
   TempWidth := Ask('Help level (1=Expert,2=Normal,3=Novice): ',1);
   val( TempWidth, num, error );
   if (not num in [1..3]) or (error <> 0) then UWriteln('Must be 1 to 3');
   monitor;
until (num in [1..3]) or (eus);
if not eus then User.help := num;
UWriteln('Help '+TempWidth)
                end;
          'N' : begin
repeat
   TempWidth := Ask('Nulls (0 - 255): ',3);
   val( TempWidth, num, error );
   if (not num in [0..255]) or (error <> 0) then UWriteln('Must be 0 to 255');
   monitor
until (num in [0..255]) or (eus);
if not eus then User.nuls := num;
UWriteln('Nulls '+TempWidth);
                end;
          'W' : begin
repeat
   TempWidth := Ask('Screen Width (20 - 127): ',3);
   val( TempWidth, num, error );
   if (not num in [20..127]) or (error <> 0) then UWriteln('Must be 20 to 127');
   monitor;
until (num in [20..127]) or (eus);
if not eus then User.consolewidth := num;
UWriteln('Width '+TempWidth);
                end;
          'H' : begin
repeat
   TempWidth := Ask('Screen Height (10 - 80): ',2);
   val( TempWidth, num, error );
   if (not num in [10..80]) or (error <> 0) then UWriteln('Must be 10 to 80');
   monitor;
until (num in [10..80]) or (eus);
if not eus then User.consolehight := num;
UWriteln('Height '+TempWidth);
                end;
          'T' : begin
uwriteln('');
repeat
   if inputbuffer = '' then
   begin
      UWriteln('Terminal type');
      UWriteln(' 0 Dumb');
      UWriteln(' 1 Soroc');
      UWriteln(' 2 ANSI');
      UWriteln(' 3 DataMedia');
      uwriteln('');
   end;
   TempWidth := Ask('Type 0 - 3: ',1);
   val( TempWidth, num, error );
   if (not num in [0..3]) or (error <> 0) then uwriteln('Must be 0 to 3');
   monitor
until (num in [0..3]) or (eus);
if not eus then User.consoletype := num;
case user.consoletype of
   dumb : uwriteln('Dumb');
   soroc : uwriteln('Soroc');
   ansi : uwriteln('ANSI');
   datamedia : uwriteln('DataMedia')
end;
                end;
          'M' : begin
user.more := not user.more;
UWrite('"More?" O'); if user.more then uwriteln('n') else uwriteln('ff');
paginate := user.more
                end;
          'S' : begin
user.singlekey := not user.singlekey;
Uwrite('Single key O'); if user.singlekey then uwriteln('n') else uwriteln('ff');
flushinputbuffer
                end;
          '?' : begin
displayfile(helpdrv+':HELP2.BBS');
upressenter;
                end
         end
      end
   until (Command = '') or (Command = 'Q');

   userchanged := true;

end;
