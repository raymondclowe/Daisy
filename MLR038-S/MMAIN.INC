{ 7/1/89 }

begin { main }
   gotoxy(1,15); write(esc,'Y');
   gotoxy(1,14);
   write('>->-------------------------------------------------------------------------<-<');
   gotoxy((39-((length(version)+2) div 2)),14); write(' '+version+' ');
   gotoxy(1,16);
   writeln('By Raymond Lowe, Ken Lo and Stanley Yau 1987, 1988, 1989');

   currentdrive := chr(bdos(25)+65);

   diskreset;

   elapsedtime := 30; mscounter := 0; lastupdated := 0;
                  {}
   readperams;

   assign(nodelist,CFG.nodelstdrv+':NODELIST.BBS');
   fillchar(node,sizeof(node),0);

   datestr := '01 Jan 80 00:00';
   todayyear := 80; TodayMonth := 1; todayday := 1;
   NowHour := 0; NowMin := 0; nowseconds := 0;

   statsread := false;
   Interactive := false;
   logopenf := false;

   tagnum := 1;
   repeat
      sdone := false; rdone := false;
      CFG.ReceiveOnly := false; CFG.SendOnly := false;
      human := false;

      case tagnum of
         1 : eventcode := CFG.PrimaryEventCode;
         2 : eventcode := CFG.SecondaryEventCode;
      end;

      if (eventcode <> 'R') and (arctest) then
      begin
         readstats; { date/time - areas.bbs }
         statsread := true;
         if CFG.KeepMailLog then
         begin
            logopen;
            logwrite(cr+lf+datestr+' Begin '+version+cr+lf);
         end;
         goto jump2arc;
      end;

      if (eventcode <> 'R') and (not statsread) then
      begin

         readstats; { date/time - areas.bbs }
         statsread := true;
         if CFG.KeepMailLog then
         begin
            logopen;
            logwrite(cr+lf+datestr+' Begin '+version+cr+lf);
         end;
         write('Interactive session [y,N]? ');
         ti:=elapsedtime;
         repeat delay(100) until (Keypressed) or (elapsedtime > ti +5);
         ch := 'N';
         if keypressed then read(kbd,ch);
         writeln(ch);
         Interactive := ch in ['Y','y'];
         if interactive then report('Interactive session',2);
      end;

      case eventcode of
       'H': begin
               report('Hold mail',0);
               unpack;
               pack;
               initpackets(true);
               CFG.ReceiveOnly := TRUE;
               w;
               if not human then unpack;
               renamePackets
            end;
       'C': begin
               unpack;
               pack;
               initpackets(true);
               if Pktsyettosend > 0 then
               begin
                  CFG.SendOnly := TRUE;
                  report('Crash mail',0);
                  w;
                  if not human then unpack;
                  renamePackets
               end;
            end;
       'W': begin
               report('Normal window',0);
               unpack;
               pack;
               initpackets(true);
               if CFG.SendOnly then report('Send only',1);
               if CFG.ReceiveOnly then report('Receive only',1);
               w;
               if not human then unpack;
               renamePackets
            end;
       'P': begin
               report('Pack',0);
               unpack;
               pack;
            end;
       'G': begin
               makepoll(true);
               CFG.SendOnly := TRUE;
               report('Get mail',0);
               w;
               unpack;
               renamePackets
            end;
       'R': begin
               if CFG.KeepMailLog then
               begin
                  logopen;
                  logwrite(cr+lf);
               end;
               CFG.ReceiveOnly := TRUE;
               report('Receive mail',0);
               NumOutBoundPkts := 0;       { default, unless initpkt is called by r2 }
               baudrate := callersbaud;
               ModSetBaud;

               if modcarrier then r2;

               show('Now reading stat files',1);

               readstats;             { date/time - areas.bbs }
               statsread := true;
               report(datestr,2);
               if not human then unpack;
               renamePackets
            end;
      else
         begin
            show('Not valid Mailer code',0);
         end
      end;

      inc(tagnum);
   until (tagnum > 2) or { done both }
         (human) or
         (arctest) or
         (not (CFG.SecondaryEventCode in valideventCodes)); { secondary tag not  valid}
Jump2ARC:
   elapsedtime := elapsedtime + 30;
   inctime;

   if fileexists(generaldrv+':SYSDATA.BBS') then
   begin
      new(df);
      assign(df^,generaldrv+':SYSDATA.BBS');
      rewrite(df^);
      writeln(df^,datestr);
      writeln(df^,totalcalls);
      close(df^);
      dispose(df);
   end;

   new(df);
   assign(df^,'EVENT.$$1');
   rewrite(df^);
   if not human then
   begin
      modHangup;
      ModOffHook;
      if fileexists('unarc.com') then
      begin
{         rawdir(cfg.FileAttachDrv,'????????MO?');}
         if arctest {filecounter > 0} then
         begin
            write(df^,'A');
            new(subline);
            fillchar(subline^,sizeof(subline^),0);
            subline^[1] := 'UNARC '+cfg.FileAttachDrv+':'+arcpkt+' '+cfg.workdrv+': N';
            subline^[2] := 'ERA '+cfg.FileAttachDrv+':'+arcpkt;
            numsublines := 2;
            submit;
            dispose(subline);
            report('Unarcing '+cfg.FileAttachDrv+':'+arcpkt,1)
         end
         else
            writeln('No arcmail packets');
      end
   end
   else
      write(df^,upcase(eventtag));
   close(df^);
   if elapsedtime >= cfg.windowlength then erase(df^);
   dispose(df);

   if CFG.KeepMailLog then logclose;

   if keypressed then read(kbd,ch); {clear console}
end.
