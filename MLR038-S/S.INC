{ 27/11/88 }

{ "callresult" means success of send packet }
procedure s3; { declared FORWARD }
label exits3;
type st3 = string [3];
var start,waituntil : integer;
    XMStart : boolean;
    i : integer;
    stri:st5;
    f:file;
    block : array [1..128] of byte;
    outfn:st14;
    WakeStr :st8; wakelen : byte absolute wakestr;
    mili5s, count5s : integer;


{>S3   WaitClear 1 no input for 0.5 secs  send TSYNCH = AEH        S4
                 2 over 60 seconds        hang up, report garbage  exit
                    and line not clear}
begin {s3}
   xmstart := false;
   WakeStr := tsynch+cr+' ';
   i := 1;
   mili5s := 0;
   count5s := 0;
(*   writeln('>Wait clear<');*)
   Start := elapsedtime;
   repeat
      ch := ModIn(10);
      if not timeout then write(ch)
   until (timeout) or { clear for 10 x 10ths }
         (elapsedtime > (start+60)) or (not modcarrier);
   if (not timeout) then
   begin
      {hangup report garbage}
      modhangup;
      report('Garbage',1);
      CallResult := garbage;
      goto exits3
   end;

(*   writeln(CR,LF,'>Clear<');*)

   { Wake up the other side with Tsynch }
   {s3f = 1}
(*   writeln('>Start Tsynch<');*)
   start := elapsedtime;
   modOutC(Tsynch);
   repeat
      if Modkeypressed then
      begin
         modInC(ch);
         if (ch in [Req_CRC,NAK]) then
            XmStart := true
         else
            write(ch)
      end;
      if not XmStart then
      begin
         delay(5);
         mili5s := succ(mili5s);
         count5s := succ(count5s);
         if count5s > 30 then    { 150ms } { time to send another wakechar }
         begin
            repeat ch := ModIn(10);        { wait for quiet }
             if not timeOut then
             begin
                if (ch in [Req_CRC,NAK]) then
                   XmStart := true
                else
                   write(ch) { echoing to screen all the while }
             end
            until (timeout) or (XmStart) or (not ModCarrier);
            if not XmStart then { wouldn't really matter? }
            begin
               modOutC(wakestr[ i ]);         { send it }
               i := succ(i); if i > wakelen then i := 1;
               count5s := 0;
            end
         end;
      end
   until (xmstart) or { xmodem start? }
         (elapsedtime > (start+60)) or {<<<< this is a note in FSCxxx, makes sense }
         (not modcarrier);

   if not modcarrier then
   begin
      report('Lost carrier while sending TSYNCH',1);
      callresult := failed;
      goto exits3
   end;

   if not xmstart then
   begin
      modhangup;
      report('No responce to TSYNCH!',1);
      CallResult := failed;
      goto exits3
   end;

   str(calls[ThisCall].packetnum,stri);
   if calls[ThisCall].hold then
      outfn := CFG.WorkDrv+':'+stri+'.HUT'
   else
      outfn := CFG.WorkDrv+':'+stri+'.OUT';


   xs(outfn); {s4 xs0}

   if not xsOK then
   begin
      {hangup , report mail bad}
      delay(1000);
      modhangup;
      report('Mail bad',1);
      CallResult := failed;
      goto exits3
   end;

{   writeln('shortpause,eatchar');
   delay(100); ch:=modin(1);}

   bs(CFG.WorkDrv+':'+stri+'.FIL'); {s6 bs0}
   if not bsOK then
   begin
      {hangup, reprt files fail}
      modhangup;
      delay(1000);
      report('Files bad',1);
      CallResult := failed;
      { Mail has gone okay so erase it, just leave null packet }
      assign(f,outfn);
      reset(f); blockread(f,block,1); close(f);
      fillchar(block[59],69,0);
      rewrite(f); blockwrite(f,block,1); close(f);
      goto exits3
   end;

   writeln('Send Okay');
   CallResult := Okay;
   sdone:=true;
   calls[ThisCall].sent := true; {remove node from ThisCall list}
   calls[ThisCall].trytosend := false;
   dec(PktsYetToSend);

   { sent okay so kill it }
   if not CFG.preservepackets then
   begin
      assign(f,outfn); {.hut or .out}
      erase(f);
      if fileexists(CFG.WorkDrv+':'+stri+'.FIL') then
      begin
         assign(f,CFG.WorkDrv+':'+stri+'.FIL');
         erase(f)
      end;
   end;

   {s8}
   if (not CFG.TryPickUp) or (Rdone) then
   begin
      delay(5000);
      modhangup;
      {hangup, rept send Okay}
      goto exits3;
   end;

   {note send Okay}
(*   writeln('Try to pickup mail');*)
   r2; {r2*}

exits3:
end;

{ sends packet detailed in calls[ThisCall] }
procedure s;
var i : integer;

{S0   SendInit                           dial modem               S1
 S1   WaitCxD   1 carrier detected       delay 1-5 seconds        S2
                2 busy, etc.             report no connection     exit
                3 voice                  report no carrier        exit
                4 carrier not detected   report no connection     exit
                   within 60 seconds
 S2   WhackCRs  1 over 30 seconds        report no response <cr>  exit
                2 ?? <cr>s received      delay 1 sec              S3
                3 <cr>s not received     send <cr> <sp> <cr> <sp> S2
                                          delay ??? secs
 S4*  SendMail                           (XMODEM send packet XS0) S5
 S5   CheckMail 1 XMODEM successful      (Fido registers success) S6
                2 XMODEM fail or timeout hang up, report mail bad exit
 S6*  SendFiles                          (BATCH send files BS0)   S7
 S7   CheckFile 1 BATCH send successful                           S8
                2 BATCH send failed      hang up, rept files fail exit
 S8   TryPickup 1 wish to pickup         note send Okay             R2*
                2 no desire to pickup    delay 5 secs             exit
                                          hang up, rept send Okay}

function s2:boolean; { WackCRs }
var i, start : integer;
    wakestr : st8;
    wakelen : byte absolute wakestr;
    GotCr : boolean;
    mili5s, count5s : integer;
begin
(*   writeln('>Pause<');*)
   GotCr := false;
   i := 1;
   wakestr := cr+cr+cr+' '+cr+' ';

   mili5s := 0;
   count5s := 0;
   start := elapsedtime;

   {delay 1-5 seconds}
   ch := modIn(200);
   if not timeout then write(ch);

(*   writeln('>Wait CRs<');*)
   repeat
      if Modkeypressed then
      begin
         modInC(ch);
         if (ch = cr) then
            GotCr := true
         else
            write(ch)
      end;
      if not GotCR then
      begin
         delay(5);
         mili5s := succ(mili5s);
         count5s := succ(count5s);
         if count5s > 100 then { time to send another wakechar }
         begin
            repeat ch := ModIn(10);        { wait for quiet }
             if not timeOut then
             begin
                if (ch = cr) then
                   GotCr := true
                else
                   write(ch) { echoing to screen all the while }
             end
            until (timeout) or (GotCR) or (not ModCarrier);
            if not gotCR then
            begin
               modOutC(wakestr[ i ]);         { send wakechar }
               i := succ(i);
               if i > wakelen then
               begin
                  i := 1;
                  ModWrite(wakestr); {WACK!!}
               end;
               count5s := 0;
            end
         end;
      end
   until (GotCR) or (elapsedtime > (start + 30)) or (not modcarrier);
   s2 := GotCR;

end;

label ExitS;
var stri, stri2, stri3, stri4 :st5; kch:char;
    callbaud,
    Rnd : integer;
    connectstr:st80;
    start,waituntil : integer;
begin {s}


   CallResult := unknown;
(*   writeln('Send FSM triggered');*)
   sdone:=true;

   str(calls[ThisCall].destnode,stri);
   str(calls[ThisCall].packetnum,stri2);
   str(calls[ThisCall].destnet,stri3);

   callbaud := nodebaud(calls[ThisCall].destnet,calls[ThisCall].destnode);

   str(callbaud,stri4);
   connectstr := nodename(calls[ThisCall].destnet,calls[ThisCall].destnode)+
          ' ('+stri3+'/'+stri+') at '+stri4+' baud to send packet #'+stri2;

   show('Calling '+connectstr,1);

   modinit;
   baudrate := callbaud;
   ModSetBaud;

   delay(500);
   outdir(CFG.DialStr+NodePhoneNumber(calls[ThisCall].destnet,calls[ThisCall].destnode)+
          CFG.DialSufix);

   {s1?}
   rnd := random(3)-1;
   scanto(calls[ThisCall].destnet,calls[ThisCall].destnode);
   { if calling a pep modem add 20 seconds to wait time }
   if PEP in node.flags then rnd := rnd + 20;
   start := elapsedtime;
   waituntil := start +(CFG.WaitCarrierDelay+RND);
   repeat
      delay(25);
      if keypressed then
      begin
         read(kbd,kch);
         case kch of
            nul : ;
            esc : begin
                   writeln('Abort');
                   outdir('|~|~|~|~|');
                   waituntil := start + 2;
                  end
         else
            writeln('Press <Esc> to abort call')
         end
      end
   until modcarrier or
         (elapsedtime > waituntil);

   if not modcarrier then
   begin
      {report no carrier/connect/  timed out probably}
      outdir('|~~');
      show('No carrier',2);
      modinit;
      CallResult := noconnect;
      goto exits
   end;

   report('Connected to '+connectstr,1);
   writeln; writeln; writeln; write(^k,^k,^k);

   {s2 wack crs }
   if s2 then
      s3 { everything okay up till now, proceed to s3 }
   else
   begin
      report('No responce',1);
      modhangup {this is not in fsc};
      CallResult := failed;
      goto exits
   end;

ExitS:
   delay(1000);  { pause }
   lowvideo; normvideo;
   writeln('Ready')
end;
