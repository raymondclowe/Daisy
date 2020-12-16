{ 27/9/88 }
procedure sysopmenu;
var Command : st1;
    ok : boolean;
begin
   uclrscr;
   if logactivity then writeln(TempLog,'! Sysop section');
   uwriteln('Sysop section');
   repeat { outer loop; until user goodbye }
      repeat { Getting a valid Command }
         UWriteln('');
         case user.help of
         2,3: Command := upstring( Ask('Sysop: R A B S T Q or ? for help: ',1));
         1: Command := upstring( Ask('Sysop: ',1));
         end;
         if command = '' then command := 'Q';
         ok := Command[1] in ['R','A','B','Q','S','T','?'];
         if not ok then NotACommand;
         Monitor
      until (ok) or (EUS);
      Monitor;
      if not EUS then
         Case Command[1] of
          '?' : displayfile(helpdrv+':HELP4.BBS');
          'R' : Renumbermsgs;
          'A' : if user.priv >= areasmpriv then ChangeAreas;
          'B' : if bulslistfn >'' then
                   changebulletins
                else
                   uwriteln('Multiple bulletin function not activated');
          'S' : SystemStats;
          'T' : typefile(upstring(ask('Filename : ',14)));

      end;
      Monitor
   until (Command[1] in ['Q']) or (EUS)
end; {sysopmenu}


procedure mainmenu;
var Command : st1;
    p : integer;
begin
   repeat { outer loop; until user goodbye }
      if userchanged then updateuser; { do it here incase the command }
                                      { will exit the system }
      repeat { Getting a valid Command }
         repeat
            UWriteln('');
            case user.help of
            2,3: Command := upstring( Ask(mainprompt,1));
            1: Command := upstring( Ask('MAIN: ',1));
            end;
            Monitor
         until (Command > '') or (EUS);
         Monitor;
         if not (Command[1] in maincommandset) then NotACommand;
         Monitor
      until (Command[1] in maincommandset) or (EUS);
      Monitor;
      if not EUS then
         Case Command[1] of
          '?' : Displayfile(Helpdrv+':HELP3.BBS');
          'D' : setsystemdate;
          'C' : Change;
          'M' : begin
                   areanum := user.lastarea;
                   openMsgFiles;
                   MsgRead;
                   closeMsgFiles
                end;
          'U' : ViewUserList;
          'Y' : if (allowyell) then
                begin
                   if logactivity then writeln(TempLog,'! Yelled');
                   uwrite('Yelling for Sysop');
                   yelldone := false;
                   yellbeep := 0;
                   uinterupted := false;
                   repeat
                      UWrite('.'+^G); if not echoon then write(^G);
                      inc(yellbeep);
                      monitor;delay(250);monitor;delay(250);
                      monitor;delay(250);monitor;delay(250);
                      UCtrlSC
                   until (yellbeep > 30) or (yelldone) or (uinterupted) or (eus);
                   if (yellbeep > 30) and (not yelldone) and (not eus) then
                   begin uwriteln('');
                      uwriteln('Sysop is not available at the moment')
                   end
                end
                else
                   uwriteln('Yelling at the Sysop not allowed now');
          'V' : begin
                   Uwriteln('Daisy '+version);
                   UWriteln('By Ken Lo and Raymond Lowe');
                   uwriteln('Copyright (C) 1987,1988');
                   uwriteln('All Rights Reserved');
                   uwriteln('');
                   uwriteln(iov);
                   uwriteln('');
                   uwriteln(clockv);
                   if not clock then uwriteln('(No clock installed)')
                end;
          'B' : displaybuls;
          'E' : displayfile(editorialfn);
          'O' : begin
if (fileexists(outsidefn)) and (askyn('Go outside Daisy','N')='Y') and
   (not eus) then
begin
   uWriteln('Wait ...');
   if not eus then submit(outsidefn)
end
                end; {outside}
          'F' : begin
if (askyn('Go to the external File system','N')='Y') and
   (not eus) then
begin
   uWriteln('Wait ...');
   if not eus then submit(eventdrv+':EVENT.C');
end
                end; {filer}
          'A' : begin
if fileexists(questionFN) then
begin
   p:=pos('.',questionfn);
   if p > 0 then
   begin
      if upstring(copy(questionfn,p,4)) = '.SUB' then
      begin
         if (askyn('Answer the Questionnair','N')='Y') then
         begin
            uWriteln('Wait ...');
            if not eus then submit(questionFN);
         end
      end
      else
         displayfile(questionfn);
   end
   else
      displayfile(questionfN);
end
                end; {answer}
          '*' : sysopmenu;
          'G' : if Askyn('Disconnect','Y') = 'N' then command := '*g'
      end;
      Monitor
   until (Command[1] in ['G']) or (EUS)
end; { main menu }
