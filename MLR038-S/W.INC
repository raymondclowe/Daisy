{ 27/9/88 }

procedure w;

var w1f,
    LastCallTime,
    intercalldelay,
    jinfo:integer;
    KbdChr : char;
    JustR : boolean;
    stri : st4;

procedure makeoutgoingcall; { starts at w3 }
begin
   { pick a ThisCall - any ThisCall }
   scan := ThisCall;              {w3}
   repeat
      inc(scan);
      if scan > NumOutboundPkts then scan := 1;
   until (scan = ThisCall) or
         ((calls[scan].trytosend) and (not calls[scan].hold));
   ThisCall := scan;

   { w3 }
   if (calls[scan].trytosend) and (not calls[scan].hold) then
   begin
      CallResult := unknown; Rdone:=false;Sdone:=false;
      s; { send_mail; w4; s0} {ThisCall}
      {w5}
   end
end;


begin
{ W0  | WindTop  | 1 end of window reached | reset modem to not answr| exit|
|     |          | 2 time remains in window| ensure modem can answer | W1  |
| W1  | WindIdle | 1 incoming ThisCall         |                         | W2  |
|     |          | 2 CFG.ReceiveOnly mode     |                         | W0  |
|     |          | 3 CFG.SendOnly mode        |                         | W3  |
|     |          | 4 60-180 secs & no ThisCall |                         | W3  |
| W2* | WindRecv |                         | (receive ThisCall R0)       | W3  |
| W3  | WindCall | 1 select outgoing ThisCall  | increment try count     | W4  |
|     |          | 2 no outgoing Calls     |                         | W0  |

| W5  | WindMark | 1 ThisCall successful       | remove node fr ThisCall list| W0  |
|     |          | 2 no Connect            | remove if try cnt > lim | W0  |
|     |          | 3 ThisCall failed           | incr conn cnt, remove   | W0  |
|     |          |                         |   if con cnt > lim      |     |
}

   modinit;
(*   writeln('WindowLength ',CFG.WindowLength);*)

   InterCallDelay := CFG.betweencallsDelay + (random(45)-15);
   LastCallTime := -30000;

   ThisCall := 0;
   JustR := false;

   outdir('ATS7=120|~'); {ensure modem will wait for long time }
   human:= false;
   didgotpkt := false;

   writeln('Ready');
   repeat
      delay(100); passtime(9);

      callresult := unknown;
      {w1}

      w1f:=0;

      { The order of these is vital - they are in reverse order of
        priority }
      if elapsedtime >
            (LastCallTime +InterCallDelay) then w1f := 4;
      if kbdchr = 'C' then                     w1f := 4;
      if (CFG.SendOnly) or (JustR) then        w1f := 3;
      if CFG.ReceiveOnly then                  w1f := 2;
      if modcarrier then                       w1f := 1;
      JustR := false;
      { make or receive a ThisCall }
      if w1f in [1,3,4] then
      begin
         inctime;
         LastCallTime := elapsedtime;

         CallResult := unknown; Rdone:=false; Sdone:=false;

         case w1f of
            1 : begin
                   r;
                   JustR := true;
                end;
          3,4 : if PktsYetToSend > 0 then MakeOutGoingCall;
         end;
         if (not human) and (didgotpkt) then
         begin
            unpack;
            didgotpkt := false;
            modonhook;
         end;

         {increment (bad) Connect count; remove if Connect count > limit}
         if CallResult = failed  then {W5.3}
         begin
            inc(calls[ThisCall].connects);
            if calls[ThisCall].connects > CFG.connectslimit then calls[ThisCall].trytosend := false
         end;

         InterCallDelay := CFG.betweencallsDelay + (random(30)-15);
      end;

      kbdchr := #00;
      if keypressed then read(kbd,kbdchr);
      kbdchr := upcase(kbdchr);
      if not (kbdchr in [nul,esc,'C','I','P','#','W']) then
         writeln(cr,lf,'<Esc>)exit, C)all, I)nformation, P)oll, #)crash!, W)indow');
      if kbdchr = 'P' then makepoll(false);
      if kbdchr = '#' then cfg.sendonly := true;
      if kbdchr = 'W' then cfg.sendonly := false;
      if kbdchr = 'I' then
      begin
         clrscr;
         inctime;
         writeln('Elapsed time : ',elapsedtime,' |  Time remaining this window ',
                          CFG.WindowLength-elapsedtime,' |         ',datestr);
         writeln;
         if cfg.sendonly then writeln('Send Only');
         if cfg.receiveonly then writeln('Receive Only');
         writeln;
         writeln('Packet #  | To');
         writeln('----------|---------');

         for jinfo := 1 to numoutboundPkts do
         begin
             write(jinfo:3,'       | ',calls[jinfo].destnet,'/',calls[jinfo].destnode);
             passtime(100);
             if calls[jinfo].sent then write(' (Sent)');
             if calls[jinfo].hold then write(' (Hold)');
             str(jinfo,stri);
             if fileexists(cfg.workdrv+':'+stri+'.FIL') then write(' (FileAttaches)');
             writeln;
         end;
         writeln(cr,lf,'Ready')
      end
   until (KbdChr = esc) or
         ((PktsYetToSend = 0)and(not CFG.DoFullwindow)) or
         (elapsedtime > CFG.WindowLength) or
         (human);

   if not human then outdir('ATS0=1 S7=30|~'); {reset modem to normal}

end;
