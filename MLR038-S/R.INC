{ 7/1/89 }

{ Raymond Lowe 1987 }

var isopus : boolean;

{ NOTE: modemsetbaudrate MUST exit after ?? seconds if it fails to match }
var getbaudok : boolean;
procedure getbaud;
var stri:st4;
begin
(*   report('Determining Baud Rate',2);*)
   getbaudok := true;
   ModMatchbaudrate;
   if not modcarrier then
   begin
      modhangup;
      getbaudok := false;
      baudrate := sysmaxbaud; ModSetBaud
   end;
end;

procedure R2; { FORWARD }
label exitr2;
var  i,j,t,ioerr : integer;
     stri,stri2 : st5;
     f:file;
     FB : array [1..128] of byte;
     pkt: packetheadertype absolute FB;
     start : integer;

function havepickup:boolean;
var i : integer;
    result : boolean;
begin
   result := false;
   if NumOutboundPkts > 0 then
   begin
      i := 0;
      repeat inc(i)
      until (i=NumOutboundPkts) or
            ((calls[i].destnode = pkt.orignode) and
             (calls[i].destnet = pkt.orignet) and
             (not calls[i].sent));
      if (calls[i].destnode = pkt.orignode) and
         (calls[i].destnet = pkt.orignet) and
         (not calls[i].sent) then
      begin
         ThisCall := i;
         result := true;
      end;
   end;
(*   writeln('Havepickup returning = ',result);*)
   havepickup := result
end;

var firstsynch : boolean;
    ts,ys,es   : integer;
    ct         : integer;

begin
{| R2  | WaitTsync| 1 TSYNCH received       | ignore input not TSYNCH | R3  |
 |     |          | 2 60 seconds timeout    | hang up, report not Fido| exit|}
   modwrite('You have reached Daisy '+version+cr+lf); { a spot of advertising }
   show('WaitTsync',1);
   ch := nul;
   isopus := false;
   firstsynch:=true;
   ts := 0; ys := 0; es := 0;
   ct := elapsedtime; {Clear Time starting now}
   start := elapsedtime;
   human := false;
   repeat
      ch := modin(50);
      if not timeout then
      begin
         dispCh(ch);
         case ch of
          tsynch : inc (ts);
          yoohoo : inc (ys);
          Req_CRC,NAK : ModOutC(EOT);
          enq : inc(es);
         end;
         if not Sdone then
         begin
            case ch of
              cr :
              begin
                if CFG.AllowHuman then
                 modwrite('Press <Esc> to enter the BBS'+cr)
                else
                 modwrite(version+cr)
              end;
              esc :
              begin
                if CFG.allowhuman then
                begin
                 modwrite(lf+'Daisy now loading, keep pressing <Enter> to start it.'+cr+lf);
                 writeln;
                 report('Human',1);
                 human := true;
                end
              end
            end
         end;
         ct := elapsedtime;
      end;
   until
         { one tsynchs }
         (ts > 0) or

         { one yoohoo }
         (ys > 0) or

         { two ENQ (bark) }
         (ES > 1) or

         { one (bark), and 2 second clear }
         ((ES > 0)and(elapsedtime > ct + 2)) or

         { 1 tsynch and this is turnaround }
         ((ts > 0) and (Sdone)) or

         { called in receive mode, after 2 sec }
         ((eventcode = 'R') and (elapsedtime > start + 2)) or

         { turnaround, clear for 15sec (missed sync?)}
         (( elapsedtime > ct + 15) and (Sdone)) or

         { timeout }
         (elapsedtime > start + 30) or {60}

         { human! }
         (human) or

         (not modcarrier);

   writeln;
   if eventcode = 'R' then inc(ts); {presumed at least one synch received by daisy}
   if sdone and (ts+ys=0) then inc(ts); {might have missed tsych of turnaround }
   isOpus := (ys>0);

   if human then goto exitR2;

   { tell barker we're not interested }
   if ES > 0 then
   begin
(*      show('Barked');*)
      modoutC(CAN); { no bark }
      modoutC(ETB); { no bark }
      ModoutC(EOT); { Opus does this before hanging up }
      modoutC(CR); { Both Fido and Brinkley do this }
      delay(1000);
      ModHangUp;
   end;

   { r2 }
   if (ts+ys <= 0) or (not ModCarrier) then
   begin
      delay(1000);
      modhangup;{hangup report not fido}
(*      if sdone then
         writeln('Nothing to pickup')
      else
         report('Not fido',3);*)
      if not sdone then report('Not fido',3);
      goto exitr2
   end;

   j := 0;
   repeat
      j := j + 1;
      str(j,stri)
   until not fileexists(CFG.WorkDrv+':'+stri+'.PKT');

   { wait for it to be quiet, or 10 seconds }
(*   show('>WaitClear<',1);*)
   start := elapsedtime;
   repeat
      ch := ModIn(10); if not timeout then dispch(ch);
   until timeout or (elapsedtime > start + 10) or (Not ModCarrier);

(*   writeln('Waiting for Mail Packet');*)

(*   { Check out that this file will be okay, disk reset of problems }
   {$i-} assign(f,CFG.WorkDrv+':'+stri+'.PKT'); {$i+}
   ioerr := ioresult;
   if ioerr = 0 then
   begin
      {$i-} rewrite(f); {$i+}
      ioerr := ioresult;
      if ioerr = 0 then
      begin
         {$i+} close(f); {$i+}
         ioerr := ioresult
      end
   end;
   if ioerr <> 0 then diskreset;*)

   ch := modin(20); { eat stray char tsynch? }
(*   { just before xr }
   if isopus then
   begin
      show('Opus doing a YooHoo',1);
      show('Send an extra Req_CRC',2);
      ModOutC(Req_CRC); { tell it we 'might be a fido' }
   end;*)

   {r3;}
   xr(CFG.WorkDrv+':'+stri+'.PKT');      { receive mail packet }

   {r4}
   if not xrOK then
   begin
      delay(1000);
      modhangup;{hangup report mail fail}
      report('Mail fail',3);
      goto exitr2
   end;

   assign(f,CFG.WorkDrv+':'+stri+'.PKT');
   reset(f);
   {$I-} blockread(f,FB,1); {$I+}
   if ioresult = 0 then
   begin
      str(pkt.orignet,stri); str(pkt.orignode,stri2);
      writeln('Received packet from '+stri+'/'+stri2);
   end
   else
      report('Possible error with pkt',2);
   Close(f);
   didgotpkt := true;

   { give line a chance to calm down, this should really defeat the
     multiple EOT problem }
(*   writeln('pause 1');*)
   delay(1000);{delay 1 second; flush input (?!)}
   ch := ModIn(1); { eat char }

   {r5;} br;

   if not brOK then {r6}
   begin
      delay(1000);
      modhangup;{hangup, report bad file}
      report('Bad file',3);
      goto exitr2
   end;

(*  writeln('pause 2');*)
   delay(2000);{delay 2 secs}

   if eventcode = 'R' then initpackets(false);

   { r7 }
   if (not havepickup) or (sdone) then
   begin
      str(pkt.orignet,stri); str(pkt.orignode,stri2);
      writeln('No mail for '+stri+'/'+stri2);

      delay(1000);
      modoutC(CAN); { no bark }
      modoutC(ETB); { no bark }
      ModoutC(EOT); { Opus does this before hanging up }
      modoutC(CR); { Both Fido and Brinkley do this }

      modhangup;
(*      writeln('Receive okay'); {hangup, report receive OKay}*)
      goto exitr2
   end;

   {receiver becomes sender; s3*}
   str(pkt.orignet,stri); str(pkt.orignode,stri2);
   report('Have mail for '+stri+'/'+stri2,2);
(*   writeln('receiver becomes sender');*)
   { send packet calls[ThisCall] - ThisCall set by HavePickUp }

   rdone:=true;
   s3; { callresult set by s }

   exitr2:
end;      {R2;}


procedure r;
label exitr;
var r0f,
    r1f,
    t : integer;
    stri:st5;

{ r needs to "report" but does NOT need to have a "callresult" }

{ R0  | WaitCxD  | 1 carrier detected      |                         | R1  |
|     |          | 2 external timer expires| report no calls         | exit|
| R1  | WaitBaud | 1 baud rate detected    | send signon with <cr>s  | R2  |
|     |          | 2 no detect in ?? secs  | hang up, report no baud | exit|
| R2  | WaitTsync| 1 TSYNCH received       | ignore input not TSYNCH | R3  |
|     |          | 2 60 seconds timeout    | hang up, report not Fido| exit|
| R3* | RecMail  |                         | (XMODEM rec packet XR0) | R4  |
| R4  | XRecEnd  | 1 XMODEM successful     | delay 1 second          | R5  |
|     |          |                         |   flush input           |     |
|     |          | 2 XMODEM failed         | hang up, rept mail fail | exit|
| R5* | RecFiles |                         | (BATCH rec files BR0)   | R6  |
| R6  | ChkFiles | 1 BATCH recv successful | delay 2 secs            | R7  |
|     |          | 2 BATCH recv failed     | hang up, report bad file| exit|
| R7  | AllowPkup| 1 have pickup for sender| receiver becomes sender | S3* |
|     |          | 2 nothing to pickup     | hang up, rept recv Okay | exit|}
begin {r}
   rdone:=true;
   report('Incoming call',1);
   r0f := 0;

   if modcarrier then r0f := 1;
  {if external timer expires then r0f := 2;}

   if r0f = 0 then
   begin
     report('No call',2){report no calls};
     goto exitr
   end;

   {r1, sets r1f}
   getbaud;

   if not getbaudOK then
   begin
      {ModHangUp - already done by GetbaudviaCr }
      report('No baud',2);{hangup report no baud}
      goto exitr
   end;

   str(baudrate,stri);
   report('Connected at '+stri+' baud',1);

   Human := false;

   R2;

ExitR:

   if not human then
   begin
      if ModCarrier then ModHangup;
      delay(1000); { nice health pause to let things settle }
   end;
   writeln('Ready')

end;
