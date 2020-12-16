{ 11/10/88 }
{$v-} {so that specialreadln can be used}
{ outputs User and LogOnOk }
OVERLAY procedure LogOn;
label jumplogon;
const trytimes = 5;
var times : integer; ufn, usn : st20; divd : st1; nameOK : Boolean;
    j,ioerr : integer;
    et : integer;
    ch : char;
    f : text;
    l : st14;

procedure specialReadLn(var Line : st128; flag:byte);
var ch : char;
    linelen : byte absolute line;
begin
   ctrls := false; ctrlc := false;
   line := '';
   repeat
      ugetchar(ch);
      { if tsynch and flag right and mailer and there is a event A then do mail }
      if mailer and (flag=1) and (ch in [tsynch,yoohoo]) then SUBMIT(eventdrv+':EVENT.A');
      if ch in [' '..'~'] then
      begin
         line := line + ch;
         if flag = 0 then
            uwrite('.')
         else
            uwrite(ch);
      end
      else
         if ch in [bs,del] then
         begin
            if lineLen > 0 then
            begin
               lineLen :=  pred(LineLen);
               uwrite(bs+' '+bs)
            end
         end
         else
            if ch in [nak, can] then
            begin
               uwriteln('\');
               line := '';
            end;
      monitor
   until (ch=cr) or (linelen>127) or (Eus);
   uwriteln('');
   ctrls := true; ctrlc := true;
   linecount := 0;
end;

procedure CheckPassword;
var Password : st20;
begin
   times := 0;
   flushinputbuffer;
   repeat
      Inc (times ); if times > TryTimes then LogOnOk := false;
      if LogOnOk then
      begin
         uwrite('Password: ');
         specialreadln(password,0);
         password := namecase(copy(password,1,20))
      end;
      if Password <> User.Password then
      begin
         UWriteLn('Wrong!');
         if logactivity then writeln(templog,'# Bad password '''+password+''' from '''+user.name+'''');
         if times >trytimes then logonok := false
      end;
      monitor
   until (password = user.password) or (EUS) or (not LogOnOk);
end; { Check Password }

procedure CheckUser;
VAR TryUser : Person;
type keytype = array [1..2] of char;
var tempuser: file of keytype;
    temp,
    searchkey : keytype;
    numusers,lastrec : integer;
begin
   UWriteln('Wait ...');
   RESET( UserFile );
(*   if IOResult <> 0 then
   begin
      UWriteLn(oerr+UserListFN);
      HALT
   end;*)
   Usernumber := -1;
   numusers := filesize (Userfile);
   lastrec := numusers -1;
   if numusers > 0 then
   begin

      if ramdrv = ' ' then
      begin
         repeat
            read( UserFile, TryUser );
            inc( Usernumber );
            UserveBuff;
            monitor
         until (TryUser.Name = User.Name ) or ( usernumber = lastrec ) or (EUS)
      end
      else
      begin
         assign(tempuser,ramdrv+':tempuser.$$$');
         {$i-} reset(tempuser); {$i+}
         if ioresult <> 0 then rewrite(tempuser);
         tryuser := nulperson;
         searchkey[1] := user.name[1];
         searchkey[2] := user.name[length(user.name)];
         repeat
            inc( Usernumber );
            seek( tempuser, usernumber);
            UserveBuff;
            if usernumber = filesize(tempuser) then
            begin
               seek( userfile, usernumber);
               {$i-} read( UserFile, TryUser ); {$i+}
               if ioresult <> 0 then
               begin
                  uwriteln('Internal userlist error!');
                  if logactivity then writeln(templog,'# Internal userlist error!');
                  close(tempuser); erase(tempuser);
                  if userlocation = remote then
                  begin
                     repeat modemhangup; monitor until eus
                  end
                  else
                     eus := true;
               end
               else
               begin
                  seek( tempuser, usernumber );
                  temp[1] := tryuser.name[1];
                  temp[2] := tryuser.name[length(tryuser.name)];
                  write(tempuser, temp)
               end
            end
            else
               read( tempuser, temp);
            UserveBuff;
            if temp = searchkey then
            begin
               seek( userfile, usernumber); UserveBuff;
               read( UserFile, TryUser );
            end;
            UserveBuff; monitor
         until (TryUser.Name = User.Name ) or ( usernumber = lastrec ) or (EUS);
         UserveBuff;
         close(tempuser);
      end
   end
   else
      tryUser.name := ''; { dummy to make sure there is no match }
   if ( usernumber = lastrec ) and ( TryUser.Name <> User.Name ) and (not eus) then
   begin
      if allownewUsers then
      begin
         inc( Usernumber );
         times := 0;
         repeat
            repeat
               User.password := Namecase(ask('Please pick a password: ',20));
               flushinputbuffer;
               if length(user.password) < 4 then Uwriteln('Too short. Try again.');
               inc(times);
               monitor
            until (Length(User.password) >= 4) or (times >trytimes) or (EUS);
            monitor
         until (askYN( User.password, 'Y' ) = 'Y') or (times >trytimes) or (EUS);
         if (times > trytimes) or (eus) then logonOk := false;
         if logOnOk then
         begin
            user.priv := newuserpriv;              { defaults from nul }

            seek(userfile, Usernumber);    { move to EOF }
            {$I-} Write( Userfile, User ); {$I+}   { write new user }
            ioerr := ioresult;
            if ioresult = 0 then
            begin
               {$I-} flush( Userfile ); {$I+}
               ioerr := ioresult;
            end;
            if IOerr <> 0 then
            begin
               UWriteLn('');
               UWriteLn('Sorry '+first(User.Name)+'  can''t add new user.');
               if logactivity then writeln(templog,'! Unable to add new user');
               LogOnOk := false
            end
         end
      end
      else
      begin
         UWriteLn('');
         UWriteLn('Sorry '+first(User.Name)+' you are not a user.');
         LogOnOk := false
      end
   end
   else
      User := TryUser;
   close(Userfile)
end; {CheckUser}

procedure subtract(pmt:char);
begin
   delete(mainprompt,pos(pmt,mainprompt),2);
   maincommandset := maincommandset - [pmt]
end;

begin { LogOn }
   user := nulperson;
   user.nuls := 5;
   usernumber := 0;
   userchanged := false;

   seperatorset := [' ', ';']; { init ask }
   flushinputbuffer;
   askstrip := true;
   inyell := false;

   Linecount := 0;
   paginate := false;
   LogOnOk := true; { set true by default, check routines may set it false }

   if fileexists(generaldrv+':LASTUSER.BBS') and returning then
   begin
      assign(f,generaldrv+':LASTUSER.BBS');
      reset(f);
      readln(f,user.name); user.name[0] := chr(pos(',',user.name)-1);
      readln(f,l);
      if l[1] = 'R' then val(copy(l,9,4),baudrate,ioerr);
(*      modemhighspeed := (l <> 'Remote, 300');*)
      modemsetbaud;
      close(f);

      goto jumplogon
   end;

   modemMatchBaudrate;
   ctrls := false; ctrlc := false;
   if userlocation = remote then uwriteln('');
   UWriteln('Daisy '+version);

   if (mailer) and (userlocation = remote) then
   begin
      et := 0; {elapsed time}
      repeat
         delay(1); inc(et);
         if (buffhead <> bufftail) or (ukeypressed) then
         begin
            ugetchar(ch);
            if ch in [tsynch,yoohoo] then
               SUBMIT(eventdrv+':EVENT.A')
            else
               uwrite(cr);
         end;
         monitor
      until (et>fidowait) or (eus)
   end;
   ctrls := true; ctrlc := true;
   monitor;
   if not eus then
   begin
     displayfile(systemlogoFN);
     UWriteln('')
   end;
(*   flushinputbuffer;*)

if not eus then
begin

   { begin module get User Name }
   { var times : integer; ufn, usn, st20; divd : st1; nameOK : Boolean; }
   { by not considering space to be a seperator for the second Name }
   { it is possible to have Names of any length; from One to any number }
   { of Names }
   times := 0;
   repeat
      repeat
         repeat
             inc(times); if times > trytimes then LogOnOk := false;
             if LogOnOk then
             begin
                uwrite('Your FIRST name: ');
                if inputbuffer = '' then specialreadln(inputbuffer,1); { first line is read here to allow for tsynch }
                while (inputbuffer[1] = ' ') and (inputbufferlen>0) do  { strip leading }
                 delete(inputbuffer,1,1);
                while inputbuffer[inputbufferlen] = ' '  do { strip trailing }
                 inputbufferlen := pred(inputbufferlen);
                while pos('  ',inputbuffer) > 0 do { strip imbedded doubles }
                 delete(inputbuffer,pos('  ',inputbuffer),1);

                UFN := NameCase( Ask('Your FIRST name: ',20) );
(*                if ufn = '' then UWriteln('Must be 1 to 2 words')*)
             end;
             monitor
         until (ufn > '') or (EUS) or (not LogOnOk);
         if logonok and (not eus) then
         begin
            SeperatorSet :=  SeperatorSet - [' '];
            USN := NameCase( Ask('Your  LAST name: ',20) );
            SeperatorSet :=  SeperatorSet + [' '];
            if (UFN = '') OR (USN = '') then DIVD := '' ELSE DIVD := ' ';
            User.Name := UFN + DIVD + USN;;
            if user.name = 'All' then
               begin uwriteln('That is a reserved name'); user.name := '' end;
         end;
         monitor
      until (length(User.Name) > 1) or (EUS) or (not LogOnOk);
      if LogOnOk and (not eus) then nameOK := (AskYN( User.Name, 'Y' ) = 'Y');
      monitor
   until (nameOK) or (EUS) or (not LogOnOk);
   { end module get User Name }

   if LogOnOk then
   begin
jumplogon:

      CheckUser;

      { This is the LOCK OFF feature }
      if user.password = 'L' then LogOnOk := false;

      monitor;                       { NO password required }
      if (LogOnOk) and (not EUS) and (user.password <> 'N' ) then CheckPassword
   end
   else
      LogOnOk := false;
   monitor;
   if eus then
   begin
      LogOnOk := false;
      user := nulperson
   end;
   if not LogOnOk then
   begin
      if logcallers then writeln(templog,'* Failed log-on attempt : "'+user.name+'"');
      eus := true
   end
   else
   begin
      readdate;
      if logcallers then
      begin
         writeln(templog,'* Log-on okay by : '''+user.name+'''');
         writeln(templog,'  On at : ',presentdate);

         if userlocation = remote then
            writeln(templog,'  Baud rate : ',baudrate)
         else
            writeln(templog,'  Local call');
      end;
      inc(user.numcalls);
      user.lastcalled := presentdate;
      userchanged := true; updateuser;
      paginate := user.more;

      maincommandset := ['A','D','C','M','F','G','V','U','Y','B','E','O','*','?'];
      mainprompt := 'Main: M F G C U Y B E O D A * V or ? for help: ';
      if (Bulletinfn = '') and (bulslistfn = '') then subtract('B');
      if editorialfn = '' then subtract('E');
      if (user.priv < outsidepriv) or (outsidefn = '') then subtract('O');
      if (not filer) or (user.priv < filerpriv) then subtract('F');
      if user.priv < sysopmenupriv then subtract('*');
      if user.priv < yellpriv then subtract ('Y');
      if questionFN = '' then subtract('A');
   end;
   if (not echoOn) and (not eus) and (userlocation = remote) then
   begin
      GOTOXY(1,15);WRITE(ESC,'Y');
      gotoxy(1,17);
      writeln('Users name is ',user.name);
      write('Privilage level : ',user.priv)
   end
end
end; { LogOn }
