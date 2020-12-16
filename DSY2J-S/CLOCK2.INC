{ 6/3/88 }
{ modified for Time ][ clock and renamed to clock2.inc on 20/02/88 }
{ release version 25/12/87 }

{ CLOCK2.INC    Function to get date/time from Time ][ clock      }
{ Written by Simmons Kwan    exclusively for DAISY                }
{ based on framework of Ken Lo's clock.inc for Timemaster II H.O. }

{ Note :  location of clock card must be stated in SYSOPT.BBS     }
{         ie. CLOCK = 4   -- clock card in slot #4                }
{         The clock card must be running in 24 hours mode since   }
{         the AM/PM flag is being ignored.                        }

{ I wish to thank Mr. Eric Chiu for helping me to solve the       }
{ hardware problem I encountered with my clock card, Mr. Daniel   }
{ Pun for supplying the information I need to creat this function }
{                                                   Ken Lo        }

CONST clockV = 'Time ][ driver by Simmons Kwan';

function Clockdate : st15;

Const
  Months = 'JanFebMarAprMayJunJulAugSepOctNovDec';

type
  d2 = string[2];
  d3 = string[3];

var
  PA, count : INTEGER;
  reg : array [1..11] of integer;

function dg(d:byte):byte;
var t:byte;
begin
  mem[PA] := d;
  t := mem[PA];
  if d in [37,40] then
    t := t mod 4;
  dg := t + ord('0')
end;

function day:d2;
begin
  day:= chr(reg[7]) + chr(reg[6])
end;

function month:d3;
var m:integer;
begin
  m:= (reg[9]*10 + reg[8]) - (11 * ord('0'));
  month:=copy(months,m*3-2,3)
end;

function year:d2;
begin
  year:= chr(reg[11]) + chr(reg[10])
end;

function hour:d2;
begin
  hour:= chr(reg[4]) + chr(reg[3])
end;

function minute:d2;
begin
  minute:= chr(reg[2]) + chr(reg[1])
end;

begin
  PA   := $E082 + (ClockSlot * $10);

  for count:= 1 to 11 do
    reg[count] := dg(count+33);

  clockdate:= day +' ' +month +' ' +year +' ' +hour +':' +minute
end;

{ eof : clock2.inc }
