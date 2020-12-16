{ release version 25/12/87 }

{ CLOCK.INC    Function to get date/time from Timemaster H.O.     }
{ Written by Ken Lo    exclusively for DAISY                      }

{ Note :  location of clock card must be stated in SYSOPT.BBS     }
{         ie. CLOCK = 4   -- clock card in slot #4                }

{ I wish to thank Mr. Eric Chiu for helping me to solve the       }
{ hardware problem I encountered with my clock card, Mr. Daniel   }
{ Pun for supplying the information I need to creat this function }

const clockv = 'Timemaster clock';

function Clockdate : st15;

Const

Months = 'JanFebMarAprMayJunJulAugSepOctNovDec';

type
d3 = string[3];
var

PA,CA,PB,CB : INTEGER;

procedure Initclock;
begin
If mem[CB]=0 then
  begin
    mem[CA]:=0;mem[CB]:=0;mem[PA]:=0;mem[PB]:=255;
    mem[CA]:=4;mem[CB]:=4;
  end;
end;

function dg(d:byte):byte;
var t:byte;m:integer;
begin
  m:=16;
  if d in [53,56] then m:=4;
  mem[PB]:=16;
  mem[PB]:=d;
  t:=mem[PA];
  mem[PB]:=47;
  dg:=t-(t div m)*m+48;
end;

function day:d3;
begin
  day:=chr(dg(56))+chr(dg(55))+' ';
end;

function month:d3;
var m:integer;
begin
  m:=(dg(58)-48)*10;
  m:=m+(dg(57)-48);
  month:=copy(months,m*3-2,3);
end;

function year:d3;
begin
  year:=' '+chr(dg(60))+chr(dg(59));
end;

function hour:d3;
begin
  hour:=' '+chr(dg(53))+chr(dg(52));
end;

function minute:d3;
begin
  minute:=':'+chr(dg(51))+chr(dg(50));
end;


begin
  PA   := $E080+ClockSlot*$10;
  CA   := PA+1;
  PB   := PA+2;
  CB   := PA+3;

  initclock;

  clockdate:=day+month+year+hour+minute;
end;

