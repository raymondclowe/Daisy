{ 4/6/88 - PP4 version }

procedure savemsg;
label exitsavemsg;

var lscr,funscr : integer;
    mb : ^st128;
    L  : ^st255;
    lastl : boolean;


var SB, { scanBlock }
    LS, { largestspace }
    NFB { NumfreeBlocks } : integer;

var BlockNum : integer;
    i : integer;


{ Block allocated ? }
function BA(POINT:INTEGER): boolean;
begin
(*   if (point < 0) OR (point > (sizeofblockmap*8+7) ) then
   begin
      str(point, UWtemp);
      Writeln('no such block >'+UWtemp);
      halt
   end;*)
   BA := odd( Blockmap[ point DIV 8 ] shr (point mod 8) )
end;

{ allocate Block }
procedure AB( I : integer);
begin
(*   if (I < 0) OR (I > (sizeofblockmap*8+7) ) then
   begin
      str(I, UWtemp);Writeln('no such block >'+UWtemp);
      halt
   end;*)

(*WRITELN('bitvalue =',BITVALUE);WRITELN('before >', Blockmap[ I DIV 8 ] );*)

   Blockmap[ I DIV 8 ] := Blockmap[ I DIV 8 ] or (1 shl (i mod 8));

(*;wRITELN('after  >', Blockmap[ I DIV 8 ] );*)
(*;writeln('allocated >',i,'<>',ba(i));*)

end; {ab}

begin
   savemsgerr := 0;
   { save process goes to completion even if Carrier loss during save }
   { so no testing of Carrier within save process }

   { begin module scrunch text }
{ scr = scrunched, f = First, L = last, Un = un (as in "not yet") }
{ will not work if any of the Lines is above the desired filled length ?}
{ needs these var : lscr,funscr,MB,L,LASTL }
{ when this exits the LineNum will be the Number of Lines of scrunched text
  not Including any blank Lines, blanks Lines we can do without after this }
if line[Linenum] = '' then dec(Linenum); { so that Linenum is always the last Line with stuff we want }
if LineNum > 1 then
begin
   lscr := 1;
   funscr := 2;
   new(l); new(mb);
   L^ := line[funscr];
   line[funscr] := '';
   if funscr = linenum then lastl := true else lastl := false;
   repeat
(*;writeln('b>',L^,'<');*)
      if (length(line[lscr])+length(L^) < 128) and (not lastl) then
      begin
(*;writeln('fire1');*)
         funscr := succ(funscr);
         L^ := L^ + line[funscr];
         line[funscr] := '';
         if funscr = linenum then lastl := true
      end
      else
         if length(line[lscr])+length(L^) < 128 then
         begin
(*;writeln('fire2');*)
            line[lscr] := line[lscr] + L^;
            L^ := ''
         end
         else
         begin
(*;writeln('fire3');*)
            mb^ := copy(L^,1,128-length(line[lscr]));
            delete(L^,1,length(mb^));
            line[lscr] := line[lscr] + mb^;
            lscr := succ(lscr)
         end
(*;writeln('a>',L^,'<');*)
   until (L^ = '') and lastl;
   dispose(l); dispose(mb);
   Linenum := lscr
end;
   { end module scrunch text }

   if LineNum > 0 then
   begin

      if length( line[LineNum] ) < 128 then
         for I := length( line[LineNum] ) + 1 to 128 do
            line[LineNum][ I ] := char(0)
      else
      begin
         Inc( LineNum ); { one totally wasted block! }
         line[LineNum][0] := char(128);
         fillchar( line[LineNum][1], 128, 0 )
      end;

      {begin module find free space}
(*writeln('looking for room to put ',LineNum,' Lines');*)
LS := 0;
nfb := 0;
SB := 0;              { bits Numbered 0-7; so blocks are 0-xxx }
repeat
   while (BA( SB ) and ( NFB < LineNum )) and (SB < (sizeofblockmap*8+7)) do
      SB := succ(SB );

   if SB < (sizeofblockmap*8+7) then
   begin
      LS := SB;

      NFB := 0;
      while (( not BA(SB) ) and ( NFB < LineNum )) or (SB > (sizeofblockmap*8+7)) do
      begin
         NFB := succ( NFB );
         SB := succ( SB)
      end
   end

until (SB > (sizeofblockmap*8+7)) or (NFB >= LineNum);
if (NFB < LineNum) or (ls+linenum > sizeofblockmap*8+7 - 1 ) then
begin  { can't find enough room in text directory }
   writeln('Error: No room in block map!');
   savemsgerr := 4;
   goto exitsavemsg
end;
NewMsgHeader.pointertoBody := LS
      {end module find free space}
(* ;writeln('Numfreeblocks =',NFB); writeln('scanbit       =',sb );writeln('largestspace  =',ls );*)

      end
      else
         NewMsgHeader.pointertoBody := -1; { null message text }

   { begin module write Header }
seek( MsgIndex, highestmsgnum + 1 );
{$i-} write( MsgIndex, NewMsgHeader ); {$i+}
ioerr := ioresult;
if ioerr = 0 then
begin
   {$i-} flush( MsgIndex ); {$i+}
   ioerr := ioresult;
end;
if ioerr > 0 then
begin
   SEEK(MSGINDEX,0); { move away from edge?? }
   writeln('Error: Can''t save message.');
   savemsgerr := 1;
   if ioerr = $f0 then
   begin
      writeln('Disk full');
      savemsgerr := 2
   end;
end;
   { end module write Header }
if (ioerr = 0) and (linenum>0) then
begin    { begin module write Text }
         { var I : integer; }
 BlockNum := NewMsgHeader.pointertoBody;
 i := 1;
 repeat
   seek( MsgText, BlockNum);
   {$i-} Blockwrite( MsgText, line[I][1], 1 ); {$i+}
   ioerr := ioresult;
   if ioerr > 0 then
   begin
      writeln('Error: Can''t save all of message');
      savemsgerr := 3
   end
   else
   begin
      AB( BlockNum );
      Inc( BlockNum )
   end;
   inc(i);
 until (I > linenum) or (ioerr>0);
   { end module write Text }
 if ioerr = 0 then
 begin
 { update filedataBlock }
   SEEK( MsgText, 0 );
   Blockwrite( MsgText, filedataBlock, 1 )
 end
end;

if ioerr = 0 then
begin
{ Inc and save highestmsgnum and number of messages }
   Inc( highestmsgnum );
   inc( nummsgs);
   updatemsgstats;{ note that the highestmsgnum value on disk is updated after every }
    { message is Entered this avoids messages becoming un-available after crash }

   close(msgindex);  { open and close to update directory }
   close(msgtext);   { don't bother with full areaopen/close }
   reset(msgindex);  { because that also references area.bbs }
   reset(msgtext)

end
else
begin {error}
   seek(MsgText,0);
   BlockRead(MsgText,FileDataBlock,4)
end;
   { finished saving }

   exitsavemsg:;
   flush( MsgIndex )

end; {savemsg }
