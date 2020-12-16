{ release version 23/9/88 }

{ SSC.INC     I/O routines for Super Serial Card used on Apple ][  }
{ These routines are for external 1200 baud modem only.            }
{ Note :  Program must be compiled with Directives C- & U-         }
{ Routines written by Ken Lo   TOPWB (164/9)    March  1987        }

{ three modifications by RL: MONITOR inserted in indata, speaker off and
  LF insert removed 27/4/87}
{ two more: LF generate after modemreadln exits because of CR, added kludgy
  backup to non functional hangup routine 28/4/87 }
{ one more: Delay put in modeminit, makes it work everytime, previously it
  missed about 1 in 8. 29/4/87 }
{ one more: added ken's modemkeypressed .  3/5/87 }
{ one more; this one is a kludge, put in a call to a procedure that tries
  to handle ctrl-c and ctrl-s, it doesn't work very will unfortunately }
{ one more: fixed backspace problem. No more backspace when margin reached }
{ Provide support to protocol 7E1 at 300 baud    22/5/87  KL  }
{ optemized +++hangup time, and other stuff to allow for return from
  outside 3/7/87}
{ moved delay to modemsetbaud }
{ M0 optional in modeminit. 31/7/87 RL }
{ basic line reduced to 128. 20/8/87 rl }
{ additions made to support buffering, and provide more freqent calls
  UCTRLSC. 25/8/87 rl }
{ longer drop tdr in hangup 31/10/rl }
{ destructive bs, 38/11/rl }
{ zero high bit of indata, 27/12/87, rcl }
{ add input buffering, 03/01/88 SK }
{ all LINEFEED references remove RL }
{ buffering shifted to uio.inc }
{ highspeed removed, 2400 added }

const iov = 'SSC Version';

const
MIO = $E0A8;
MST = $E0A9;
CMD = $E0AA;
CTL = $E0AB;

var MIOchar : char absolute MIO;

type ms=string[128];

var  (*baudrate :integer;*)   {  Only 300, 1200 and 2400 are supported          }
(*     modemhighspeed:boolean;   {  True = 1200 baud       False = 300 baud  }*)
     echoON   :boolean;   {  True = Echo to local screen              }


function TDREmpty:boolean;          { SK }
begin
  TDREmpty:=((mem[MST] and $10)=$10)
end;

procedure DataSendReady;
begin
  while not TDREmpty do         { empty loop }
end;

function ModemCarrier:boolean;
begin
  ModemCarrier:=((mem[MST] and $20)=$0)
end;

procedure modemwritechar(c:char);
begin
  repeat until TDREmpty;
  mem[MIO]:=ord(c)
end;


procedure modemdirect(s:ms); {  By pass Carrier check in order to speak to modem }
var l:integer;
begin
   s:=s+chr(13);
   for l:=1 to length(s) do
   begin
      DataSendReady;
      mem[MIO]:=ord(s[l]);
   end;
end;


procedure baud(b:integer);
var t2:integer;
begin
  case b of
    300 : b:=6;
   1200 : b:=8;
   2400 : b:=10;
  end;
  b:=(b or $10);
  t2:=(mem[ctl] and $E0);
  t2:=(t2 or b);
  mem[ctl]:=t2;
end;

procedure databit(d:integer);  { d=0   8 data bits, 1 stop bit }
var t:integer;                 {   1   7            1          }
begin                          {   2   6            1          }
  d:=(d shl 5);                {   3   5            1          }
  t:=(mem[ctl] and $1F);       {   4   8            2          }
  t:=(t or d);                 {   5   7            2          }
  mem[ctl]:=t;                 {   6   6            2          }
end;                           {   7   5            2          }

procedure parity(p:integer);   { p=0,2,4,6  no parity }
var t:integer;                 {   1        odd       }
begin                          {   3        even      }
  p:=(p shl 5);                {   5        mark      }
  t:=(mem[cmd] and $1F);       {   7        space     }
  t:=(t or p);
  mem[cmd]:=t;
end;

function ModemKeypressed:Boolean;
begin
  ModemKeypressed:=((mem[MST] and $28)=$08)
end;

procedure modemgetchar(var ch:char);
var ready:boolean;
begin
   repeat
      monitor
   until (modemkeypressed) or (not modemcarrier);
   ch:=chr(mem[MIO]);
end;


procedure modemsetbaud;
begin
(*   if modemhighspeed then baudrate:=1200 else baudrate:=300;*)
   databit(0);parity(0); baud(baudrate); (** rl **)
   delay(50);
end;

procedure ModemHangUp;
var t:byte;
begin
   delay(100);
   mem[CMD]:=(mem[CMD] and $FE);
   delay(400);
   mem[CMD]:=(mem[CMD] or $01);
   delay(50);

   t:=1;
   while (modemcarrier) and (t <6) do
   begin   { for those with no DTR connection in their cable acts like a backup! }
(*      writeln('Hangup ',t);*)
      delay(t * 1100);
      modemwritechar('+'); delay(t * 10);
      modemwritechar('+'); delay(t * 10);
      modemwritechar('+');
      delay(t * 1100);
      modemdirect('ATH0');
      delay(t * 100);
      t:=succ(t)
   end;
end;

procedure getbaudviaCR;
var ch:char; ok:byte; t:integer;
begin
  ok := 0; t:=0;
  repeat
    if modemkeypressed then
    begin
       modemgetchar(ch);
       if not (ch in [cr,' ',esc]) then
       begin
          OK := 0;
          case baudrate of
            1200 : baudrate := 2400;
            2400 : baudrate := 300;
             300 : baudrate := 1200;
          end;
          (*modemhighspeed:=not modemhighspeed;*);
          modemsetbaud
       end
       else
          ok := ok+1;
       delay(200); ch:=MIOchar; { wait and clear input }
    end;
    t:=succ(t); delay(5);
    monitor;
  until (not ModemCarrier) or (ok > 1) or (t>3000);
end;

{  --------------------------------------------------------  }
{  High Level SSC I/O Routines for general interface usages  }
{  --------------------------------------------------------  }



procedure ModemMatchBaudRate;
begin
  GetbaudviaCR;
  modemsetbaud;
end;


procedure modemcheck;
begin
   if not( (MEM[$E205] = 56) and (MEM[$E207] = 24) and (MEM[$E20B] =1 ) and ( (MEM[$E20C] and $f0) = 48 ) ) then
   begin
      writeln('No SSC in slot 2');
      HALT
   end
end;

procedure modemringhandler;
begin end;

procedure modemreset;
begin
   modemdirect('ATZ');
   delay(1000);
end;

procedure modemoffhook;
begin
   modemdirect('ATM0H1');
   delay(1000)
end;

procedure modemonhook;
begin
   modemdirect('ATM1H0');
   delay(100);
end;

procedure ModemInit;
var m: char;
begin
(*   modemhighspeed:=true;*)
   baudrate := sysmaxbaud; { default }
   modemsetbaud;
   modemdirect('');delay(300);
   if quiet then m:='0' else m:='1';
   modemdirect('ATH0E0Q1S0=1M'+M);delay(850);
end;
