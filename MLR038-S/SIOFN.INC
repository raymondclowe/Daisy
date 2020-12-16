{ 7/1/89 }

{ Ken's routines }
const iov = 'SSC Version';

var
MIO : byte absolute $E0A8;
MIOchar : char absolute $E0A8;
MST : byte absolute $E0A9;
CMD : byte absolute $E0AA;
CTL : byte absolute $E0AB;

type ms=string[128];

var  baudrate :integer;   {  Only 300, 1200, 2400 are supported          }
(*Modhighspeed:boolean;   {  True = 1200 baud       False = 300 baud  }*)


function ModCarrier:boolean;
begin
  ModCarrier:=((MST and $20)=$0)
end;

procedure ModOutC(c:char);
begin
  repeat until (MST and $10)=$10;
  MIOchar:=c
end;

procedure ModOutB(B:byte);
begin
  repeat until (MST and $10)=$10;
  MIO:=b
end;


procedure baud(b:integer);
var t2:integer;
begin
  case b of
    300 : b:=6;
   1200 : b:=8;
   2400 : b:=10
  end;
  b:=(b or $10);
  t2:=(ctl and $E0);
  t2:=(t2 or b);
  ctl:=t2;
end;

procedure databit(d:integer);  { d=0   8 data bits, 1 stop bit }
var t:integer;                 {   1   7            1          }
begin                          {   2   6            1          }
  d:=(d shl 5);                {   3   5            1          }
  t:=(ctl and $1F);            {   4   8            2          }
  t:=(t or d);                 {   5   7            2          }
  ctl:=t;                      {   6   6            2          }
end;                           {   7   5            2          }

procedure parity(p:integer);   { p=0,2,4,6  no parity }
var t:integer;                 {   1        odd       }
begin                          {   3        even      }
  p:=(p shl 5);                {   5        mark      }
  t:=(cmd and $1F);       {   7        space     }
  t:=(t or p);
  cmd:=t;
end;

function ModKeypressed:Boolean;
begin
  ModKeypressed:=((MST and $28)=$08)
end;

procedure ModInC(var ch:char);
var ready:boolean;
begin
   repeat
   until (Modkeypressed) or (not Modcarrier);
   ch:=MIOchar;
end;


procedure Modsetbaud;
var x : byte;
begin
(*   if Modhighspeed then baudrate:=1200 else baudrate:=300;*)
if baudrate > sysmaxbaud then baudrate := sysmaxbaud;
(*qwriteln('setting baud rate to ',baudrate);*)
   databit(0);parity(0); baud(baudrate); (** rl **)
end;

procedure ModWrite(l:st128);
var i : byte;
    len: byte absolute l;
begin
   i := 1;
   while i<=len do
   begin
      ModOutC(l[i]);
      i:=succ(i);
   end
end;

Procedure OutDir(l:st128);
var i : byte;
begin
(*   writeln('OutDir(',l,')');*)
   for i := 1 to length(l) do
   begin
      case l[i] of
        '|' : begin modoutC(cr); delay(200); end;
        '~' : delay(500)
      else
         modOutC(l[i])
      end
   end;
end;



procedure ModHangUp;
var t:byte;
begin
(*   if modcarrier then
      writeln('hangup')
   else
      writeln('already hungup');
*)   delay(100);
   CMD:=CMD and $FE;
   delay(400);
   CMD:=(CMD or $01);
   delay(50);

   t:=1;
   while (Modcarrier) and (t <6) do
   begin   { for those with no DTR connection in their cable acts like a backup! }
      delay(t * 1100);
      ModOutC('+'); delay(t * 10);
      ModOutC('+'); delay(t * 10);
      ModOutC('+');
      delay(t * 1100);
      ModWrite('ATH0'+cr);
      delay(t * 100);
      t:=succ(t)
   end;
end;

procedure getbaudviaCR;
var ch:char; ok:byte; start:integer;
begin
  ok := 0; start:=elapsedtime;
  repeat
    if Modkeypressed then
    begin
       ModInC(ch);
       if not (ch in [esc,cr,' ',tsynch,yoohoo]) then
       begin
          case baudrate of
            1200 : baudrate := 300;
             300 : baudrate := 2400;
            2400 : baudrate := 1200;
          end;
          Modsetbaud;
          delay(200); ch := MIOchar; { clear garbage }
       end
       else
          ok := ok+1;
    end;
    delay(50);
  until (not ModCarrier) or (ok > 2) or (elapsedtime > start+30);
  if ok = 0 then Modhangup
end;

{  --------------------------------------------------------  }
{  High Level SSC I/O Routines for general interface usages  }
{  --------------------------------------------------------  }



procedure ModMatchBaudRate;
begin
  GetbaudviaCR;
  Modsetbaud;
end;

procedure Modreset;
begin
   OutDir('ATZ|~~');
end;

procedure Modoffhook;
begin
   OutDir('~'+CFG.BusyStr+'~~');
end;

procedure Modonhook;
var m: char;
begin
   if quiet then m:='0' else m:='1';
   OutDir('ATM'+m+'H0|');
end;

procedure ModInit;
var m: char;
begin
   baudrate := sysmaxbaud;
   Modsetbaud;
   if quiet then m:='0' else m:='1';
   OutDir('AT|~ATH0E0Q1S0=1S9=12M'+M+'|~~');
   OUTDIR(CFG.INITstr);
end;


(**********************************************************************)

var timeout : boolean;
{ My verison of Stanley's routine }
function modIn(t:integer):char;  { wait for t/100 sec. before timeout }
var maxtime : integer;
    receive : boolean;
begin
  timeout:= false;
  receive:= false;
(**)  maxtime:= t * 5;
  if not ModCarrier then maxtime:= 0
  else
  begin
    repeat
      if ModKeypressed then
      begin
        modIn:= MIOchar;
        receive:= true;
      end
      else
begin        maxtime:= maxtime - 1;
(**)      delay(2); end
    until (receive) or (maxtime=0);
  end;
  if maxtime = 0 then timeout:= true;
end;

