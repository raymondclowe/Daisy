
{ Xmodem, Modem7, Telink drivers                                    }
{ Version 1.0                19 May 88                              }
{ Version 1.1                25 May 88                              }
{ Copyright 1988 by Stanley Yau.                                    }
{ All rights reserved.                                              }
{ This soruce code is a cross complied version of XMODEM.C used     }
{ in Puppy.                                                         }
{ Puppy is a BBS software by Tom Jennings.                          }
{ Thanks for Tom Jennings for his source code of Puppy.             }
{ Thanks for Raymond Lowe for his help in debugging the program     }


{ Parm. passing : dir     - true for send;                          }
{                           false for receive;                      }
{                 trans   - 0 for XMODEM;                           }
{                           1 for MODEM7;                           }
{                           2 for TELINK;                           }
{                 transfn - filename for transfer, invalid for      }
{                           MODEM7 & TELINK in receiving mode.      }
{                           EOT if transfn = ''.                    }
{                                                                   }
{ In return     : true for success.                                 }

OVERLAY function transfer(dir:boolean; trans:byte; transfn:st14):boolean;


type bufftype = array [1..128] of byte;


var  return      : integer;
     crc         : integer;
     chksum      : byte;
     timeout     : boolean;
     crcmode     : boolean;
     mode        : byte;     { 0 - XMODEM, 1 - MODEM7, 2 - TELINK }
     totl_files  : integer;
     totl_blocks : integer;
     totl_errors : integer;
     filename    : st14;
(**){ch          : char;}
     f           : file;
     telink      : record
                     fLengthl     : integer;
                     fLengthh     : integer;
                     CreationTime : integer;
                     CreationDate : integer;
                     fname        : array[1..16] of char;
                     version      : byte;
                     sendingProg  : array[1..16] of char;
                     telcrc       : byte;
                     fill         : array[1..86] of byte;
                   end;
     telbuf       : bufftype absolute telink;

const
     SOH = $01;
     EOT = $04;
     ACK = $06;
     NAK = $15;
     SYN = $16;
     SUB = $1A;

function carrier:boolean;
begin
  if (mem[$e0a9] and $20) = $0 then carrier:= true
  else carrier:= false;
end;


function modin(t:integer):byte;  { wait for t/100 sec. before timeout }
var maxtime : integer;
    receive : boolean;
begin
  timeout:= false;
  receive:= false;
(**)  maxtime:= t * 5;
  if not carrier then maxtime:= 0
  else
  begin
    repeat
      if (mem[$e0a9] and $28) = $08 then
      begin
        modin:= (mem[$e0a8]);
        receive:= true;
      end
      else
(**)  begin
         maxtime:= maxtime - 1;
(**)     delay(2);
(**)  end
    until (receive) or (maxtime=0);
  end;
  if maxtime = 0 then timeout:= true;
end;

procedure modout(t:byte);
begin
  if carrier then
  begin
    repeat until (mem[$e0a9] and $10) = $10;
    mem[$e0a8]:= t;
  end;
end;

(**)(*function allupcase(t:st14):st14;
var i:byte;
begin
  for i:= 1 to length(t) do t[i]:= upcase(t[i]);
  allupcase:= t;
end;*)

{ Convert a CP/M like filename to a normal ASCIZ name. }

procedure cvtfromfcb(inname:st14; var outname:st14);
var name : st14;
    i    : byte;

begin
  name:= '';
  for i:= 1 to 8 do
  begin
(**)    if UPCASE(inname[i]) IN (['!'..'~']) then name:= name + inname[i];      { ignore space }
  end;
  name:= name + '.';         { add the dot }
  for i:= 9 to 11 do
  begin
(**)    if UPCASE(inname[i]) IN (['!'..'~']) then name:= name + inname[i]
    else i:= 11;
  end;
  outname:= name;
end;

{ Convert a normal asciz string to MSDOS/CPM FCB format. Make the filename
  portion 8 characters, extention 3 maximum. skips drive specs. }

procedure cvttofcb(inname: st14; var outname: st14);
var first : string[14];
    last  : string[14];
    i     : byte;

  { Do part of a name }

  procedure cvt2(inname: st14; var outname: st14; n: byte);
  var i : byte;
  begin
    for i:= 1 to (n - length(inname)) do inname:= inname + ' ';
    outname:= inname;
  end;

begin
  if (length(inname) > 2) and (inname[2] = ':') then
    inname:= copy(inname,3,length(inname));
  i:= pos('.',inname);
  if i = 0 then cvt2(inname,inname,11)
  else
  begin
    first:= copy(inname,1,i-1);
    last:=  copy(inname,i+1,length(inname));
    cvt2(first,first,8);               { do name portion }
    cvt2(last,last,3);                 { do extention }
    inname:= first + last;
  end;
  outname:= inname;
end;

function checkf(fn:st14):st14;
var i,IOR : byte;

begin
  i:= 255;
  repeat
    assign(f,fn);
    {$I-} reset(f); {$I+}
    IOR:= IOResult;
    close(f);
    i:= i + 1;
    if (IOR = 0) then fn[length(fn)]:= chr(48+i);
  until (IOR <> 0) or (i = 10);
  if (IOR = 0) and (i = 10) then fn[length(fn)]:= '$';
  checkf:= fn;
end;

