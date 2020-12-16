{ 23/6/88 }
procedure callusersection; { declared forward }
begin usersection end;

procedure callUSERLISTEDITOR; { declared forward }
begin USERLISTEDITOR end;

procedure callterm; { declared forward }
begin term end;

procedure submit; {(fn:st14); FORWARD;}
begin rawsubmit(fn) end;


begin { bbs }

   setup;

   if not modemcarrier then returning := false; { clear flag }
   repeat
      displaystatusline;

      insession := false;
      CarrierLatch := FALSE;
      offhook := false;

      Userlocation := REMOTE;

      if mussed then setupsysopinterface;

      repeat
         tick;
         sysopinterface;
         modemringhandler;

         if schedular then
         begin
            readdate;
            { if this time is >= next event or day <> day then run schedular }
            if ((presentdate[11]+presentdate[12]+ { hour }
                 presentdate[14]+presentdate[15]) { minute }
                 >= NextEventTime) or
               ((presentdate[1]+presentdate[2])  { day }
                 <> NextEventDay) then
            begin
               sysopcommand := event;
               sysopPeram2 := 'D'
            end
         end;

         if newmailmsg and mailer then
         begin
            NewMailMsg := false; { clear flag }
            sysopcommand := event;
            sysopPeram2 := 'B'
         end;

         if modemcarrier then
         begin
            SysopCommand := startsession; SysopPeram1 := R
         end;

         if (externalevent <> nul) and (sysopcommand = none) then
         begin
            sysopcommand := event;
            sysopPeram2 := externalevent;
            externalevent := nul
         end;

      until SysopCommand <> none;

      ProcessSysopCommand;
      returning := false; { clear flag }

   until exitbbs;

   modemreset;

   if Logeverything or logcallers or logactivity then
      updatelog;

   { save the time }
   assign(f,generaldrv+':SYSDATA.BBS');
   rewrite(f);
   readdate;
   writeln(f,presentdate);
   writeln(f,totalcalls);
   close(f);

   gotoxy(1,21)

end.
