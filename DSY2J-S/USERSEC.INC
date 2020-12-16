{ 27/9/88 }
   newmailmsg := false;
   LogOn;

   {$i-} reset(areasf); {$i+} { keep it open during entire logon }
   if ioresult <> 0 then
   begin
      uwriteln(oerr+areasfn);
      goto exitusersection
   end;

   if not eus then
   begin
      if returning then
      begin
         uwriteln('Wait...');
         goto JumpLogOnStuff
      end;

      uclrscr;

      if welcomefn > '' then displayfile(welcomefn);

      if bulletinfn > '' then
         displayfile(Bulletinfn)
      else
         if bulslistfn > '' then
            displaybuls;

      uwriteln('');
      str(user.numcalls,uwtemp);
      uwrite('Your call #'+uwtemp);

      if (recordnumcalls) and (userlocation = remote) then
      begin
         inc(totalcalls);
         str(totalcalls,uwtemp);
         uwriteln('. System call #'+uwtemp);
         uwriteln('Wait ...');

         assign(f,generaldrv+':SYSDATA.BBS');
         rewrite(f);
         writeln(f,presentdate);
         writeln(f,totalcalls);
         close(f);
      end
      else
         uwriteln('.');

jumplogonstuff:

      if recordlastuser then
      begin
         assign(f,generaldrv+':LASTUSER.BBS');
         rewrite(f); { erase it every time }
         writeln(f,user.name,', ',usernumber);
         if userlocation = remote then
            writeln(f,'Remote, ',baudrate)
         else
            writeln(f,'Local');
         close(f);
      end;

      fillchar(shouldsearch,sizeof(shouldsearch),0); { set all to false }
      reset(areasf);
      i := 0;
      repeat
         i := i + 1;
         seek(areasf,i);
         blockread(areasf,area,1);
         if area.highmsg > user.highestread[I] then shouldsearch[I] := true;
         monitor
      until (i = filesize(areasf)-1) or (eus);
      close(areasf);

      Monitor;
      if (LogOnOK) and (not EUS) then

         (**) MainMenu (**)

   end;

exitusersection:
   uwriteln('Logging '+User.name+' off. Hang up now.');
   if logcallers then writeln(templog,'* User off at ', presentdate);
   if logactivity and (userlocation = remote) and (not modemcarrier) then
      writeln(templog,'  by carrier loss');
   GoOnHook;

   close(areasf);
   clrscr; mussed := true;

  { do update User file , even if no carrier }
   if userchanged then updateuser;

   if logeverything or logcallers or logactivity then
      UpdateLog;  { write temporary log to main log }

   InSession := FALSE;

   IOinit;
