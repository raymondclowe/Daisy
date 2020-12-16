{ 30/6/88 }

OVERLAY procedure Userlisteditor;
label loop;
var UEnumber : integer;
    EUUser : person;

    UEField : char;

    UEChanged : boolean;

    commandchar :char;
    Jumpfactor,verr : integer;
    userfilesize : integer;


procedure sayname;
begin
   gotoxy(20,16);
   if EUUser.name[1] = 'd' then
      write('Deleted')
   else
      write('       ');
   gotoxy(21,17);
   if EUUser.name[1] = 'd' then
      write(copy(EUUser.Name,2,36))
   else
      write(EUUser.Name);
   clreol;
end;


procedure findUser;
var Search : st36;
    start : integer;
    ch : char;
begin

   gotoxy(4,17);
   write('N)ame or part  :|',copy(dashes,1,36),'|'); clreol;
   gotoxy(21,18); clreol;
   gotoxy(21,19); clreol;
   gotoxy(21,20); clreol;
   gotoxy(54,21); clreol;
   gotoxy(54,22); clreol;
   gotoxy(21,17);
   buflen := 36;
   readln(Search);
   Search := upstring(Search);
   gotoxy(20,17); write(' ''',search,''''); clreol;
   if Search > '' then
   begin
      start := UEnumber;
      repeat
         UEnumber := ((UEnumber+1) mod userfilesize);
         gotoxy(10,16);
         write(UEnumber,'  ');
         seek(UserFile,UEnumber); tick; tick;
         read(Userfile,EUuser);   tick; tick;
      until (UEnumber = start) or
            (pos(search,upstring(EUuser.name)) > 0) or
            (keypressed);
      if keypressed then read(kbd,ch)
   end

end;

procedure erasetemp;
var f : file;
begin
   if ramdrv <> ' ' then
      if fileexists(ramdrv+':TEMPUSER.$$$') then
      begin
         assign(f,ramdrv+':TEMPUSER.$$$');
         erase(f)
      end
end;

procedure changeUEField;
var input : st40;
    num : integer;
    error : integer;
    i : integer;
begin
   UEChanged := true;
   case UEField of
      'N' : begin
               gotoxy(20,17);
               write('|',EUUser.name);
               write(copy(dashes,1,36-length(EUUser.name)));
               write('|');
               gotoxy(21,17);
               buflen := 36;
               readln(input);
               gotoxy(20,17); write(' ');
               if input > '' then
               begin
                  EUUser.name := namecase(input);
                  eraseTemp
               end;
               Sayname;
            end;
      'P' : begin
               gotoxy(20,18);
               write('|',EUUser.password);
               write(copy(dashes,1,20-length(EUUser.password)));
               write('|');
               gotoxy(21,18);
               buflen := 20;
               readln(input);
               gotoxy(20,18); write(' ');
               if input > '' then EUUser.password := namecase(input);
               gotoxy(21,18); write(EUUser.password); clreol;
            end;
      'C' : begin
               gotoxy(20,19);
               write('|---|');
               gotoxy(21,19);
               buflen := 3;
               readln(input);
               gotoxy(20,19); write(' ');
               val(input,num,error);
               if (error = 0) and (num in [20..127]) and (input > '') then
                  EUUser.consolewidth := num;
               gotoxy(21,19); write(EUUser.consolewidth); clreol
            end;
      'L' : begin
               gotoxy(20,20);
               write('|---|');
               gotoxy(21,20);
               buflen := 3;
               readln(input);
               gotoxy(20,20); write(' ');
               val(input,num,error);
               if (error = 0) and (input > '') and (num in [1..100]) then
                  EUUser.priv := num;
               gotoxy(21,20); write(EUUser.PRIV); clreol;
            end
   end
end; { change }

begin { edit EUUser list }


   {$I-}REset( UserFile );{$I+}
   if IOResult <> 0 then
   begin
      WriteLn(oerr+UserListFN);
      HALT
   end;
   if eof(userfile) then
   begin
      gotoxy(1,18);writeln('No users')
   end
   else
   begin
      UEnumber := 0;
      JumpFactor := 1;
      userfilesize := filesize(userfile);
      UEField := 'N';

      gotoxy(1,15); writeln(esc,'Y>>'+copy(dashes,1,30)+' edit user list '+copy(dashes,1,29)+'<<');

loop: gotoxy(1,16);
      write('   User #',UEnumber);
      seek( Userfile,  UEnumber);
      read( Userfile, EUUser );
      gotoxy(1,17);
      write('   N)ame          : '); clreol;
      sayName; writeln;
      writeln('   P)assword      : ',EUUser.password);
      writeln('   C)onsoleWidth  : ',EUUser.consolewidth);
      writeln('   L)evel priv    : ',EUUser.PRIV);
      gotoxy(40,21); write('Last  called: ',euuser.lastcalled);
      gotoxy(40,22); write('Times called: ',euuser.numcalls);

      UEChanged := false;

      repeat { loop until exit }
         gotoxy(10,16);
         write(UEnumber,'  ');
         gotoxy(74,16); write('[',jumpFactor,'] ' );

         gotoxy(1,17);if UEField = 'N' then write('-> ') else write('   ');
         gotoxy(1,18);if UEField = 'P' then write('-> ') else write('   ');
         gotoxy(1,19);if UEField = 'C' then write('-> ') else write('   ');
         gotoxy(1,20);if UEField = 'L' then write('-> ') else write('   ');
         gotoxy(80,24);

         Commandchar := nul;
         repeat    { loop for command }
            if keypressed then
            begin
               read(kbd, Commandchar);
               Commandchar := upcase(Commandchar);
            end;
            tick; if cursormoved then gotoxy(80,24);
            tick; if cursormoved then gotoxy(80,24);
            cursormoved := false;
         until (Commandchar in ['1'..'9','N','P','C','L','X',ESC,'<','S',^H,
                                ^I,^U, ',', '.', '>','F','E','U','D',^M,'K','I']);

         case commandchar of
          '1'..'9' : jumpfactor:= (ord(commandchar)-48) * 10;

          'I' : begin
                   FindUser;
                   goto loop
                end;

          'N','P','C',
          'L' : UEField := commandchar;

          'E','D',
          'U' : case commandchar of
                 'E',
                 'U' : case UEField of
                        'P' : UEField := 'N';
                        'C' : UEField := 'P';
                        'L' : UEField := 'C'
                       end;
                 'D' : case UEField of
                        'N' : UEField := 'P';
                        'P' : UEField := 'C';
                        'C' : UEField := 'L';
                       end
                end;
          ^I,^U,^H,'F',
          'S' : begin
                   case commandchar of
                     ^H,'S' : UEnumber := UEnumber-jumpfactor;
                     ^I,^U,'F' : UEnumber := ((UEnumber+jumpfactor) mod userfilesize);
                   end;
                   while UEnumber < 0 do
                      UEnumber := UEnumber+userfilesize;
                   jumpFactor := 1;
                   seek( Userfile, UEnumber ); tick;
                   read( Userfile, EUUser );   tick;

                   SayName;
                   gotoxy(21,18); write(EUUser.password); clreol;
                   gotoxy(21,19); write(EUUser.consolewidth,' ');
                   gotoxy(21,20); write(EUUser.PRIV,'  ');
                   gotoxy(54,21); write(EUUser.lastcalled);
                   gotoxy(54,22); write(EUUser.numcalls,'   ');

                  UEChanged := false
                end;
          ^M : ChangeUEField;
          'K': begin
                  if EUUser.name[1] <> 'd' then
                     EUUser.name := 'd' + EUUser.name
                  else
                     delete(EUUser.name,1,1);
                  sayname;
                  eraseTemp;
                  uechanged := true
               end; { kill }
        end;
        if UEChanged then
        begin
           seek(Userfile, UEnumber);
           write(Userfile, EUUser);
           UEChanged := false
        end
      until commandchar in ['X',ESC];
      gotoxy(1,17);
      writeln('   ');
      writeln('   ');
      writeln('   ');
      write('   ');

      close(Userfile)
   end;

   mussed := true

end; { edit EUUser list }
