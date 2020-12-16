{ 29/6/88 }

OVERLAY procedure savemsg;
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
   if (point < 0) OR (point > (sizeofblockmap*8+7) ) then
   begin
      str(point, UWtemp);
      UWriteln('no such block >'+UWtemp);
      halt
   end;
   BA := odd( Blockmap[ point DIV 8 ] shr (point mod 8) )
end;

{ allocate Block }
procedure AB( I : integer);
begin
   if (I < 0) OR (I > (sizeofblockmap*8+7) ) then
   begin
      str(I, UWtemp);UWriteln('no such block >'+UWtemp);
      halt
   end;
   Blockmap[ I DIV 8 ] := Blockmap[ I DIV 8 ] or (1 shl (i mod 8));
end; {ab}

begin
   savemsgerr := 0;
   { save process goes to completion even if Carrier loss during save }
   { so no testing of Carrier within save process }
   uclrscr;
   UWriteln('Saving your message ... ');
   readdate;
   NewMsgHeader.dateentered := presentdate;

   { begin module scrunch text }
{ scr = scrunched, f = First, L = last, Un = un (as in "not yet") }
{ will not work if any of the Lines is above the desired filled length ?}
{ needs these var : lscr,funscr,MB,L,LASTL }
{ when this exits the LineNum will be the Number of Lines of scrunched text
  not Including any blank Lines, blanks Lines we can do without after this }
if Line^[Linenum] = '' then dec(Linenum); { so that Linenum is always the last Line with stuff we want }
if LineNum > 1 then
begin
   lscr := 1;
   funscr := 2;
   new(l); new(mb);
   L^ := Line^[funscr];
   Line^[funscr] := '';
   if funscr = linenum then lastl := true else lastl := false;
   repeat
(*;writeln('b>',L^,'<');*)
      if (length(Line^[lscr])+length(L^) < 128) and (not lastl) then
      begin
(*;writeln('fire1');*)
         funscr := succ(funscr);
         L^ := L^ + Line^[funscr];
         Line^[funscr] := '';
         if funscr = linenum then lastl := true
      end
      else
         if length(Line^[lscr])+length(L^) < 128 then
         begin
(*;writeln('fire2');*)
            Line^[lscr] := Line^[lscr] + L^;
            L^ := ''
         end
         else
         begin
(*;writeln('fire3');*)
            mb^ := copy(L^,1,128-length(Line^[lscr]));
            delete(L^,1,length(mb^));
            Line^[lscr] := Line^[lscr] + mb^;
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

      if length( Line^[LineNum] ) < 128 then
         for I := length( Line^[LineNum] ) + 1 to 128 do
            Line^[LineNum][ I ] := char(0)
      else
      begin
         Inc( LineNum ); { one totally wasted block! }
         Line^[LineNum][0] := char(128);
         fillchar( Line^[LineNum][1], 128, 0 )
      end;

      SEEK( MsgText, 0 );
      Blockread( MsgText, filedataBlock, sizeofdatablock DIV 128 );

      {begin module find free space}
(*writeln('looking for room to put ',LineNum,' Lines');*)
LS := 0;
nfb := 0;
SB := 0;              { bits Numbered 0-7; so blocks are 0-xxx }
repeat
(*str(sb, UWtemp); UWriteln('sb='+UWtemp);*)
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
if (NFB < LineNum) or (ls+linenum > sizeofblockmap*80+7 - 1 ) then
begin  { can't find enough room in text directory }
   uwriteln('Error: No room in block map!');
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
{$i-} seek( MsgIndex, highestmsgnum + 1 ); {$i+}
ioerr := ioresult;
if ioerr = 0 then
begin
   {$i-} write( MsgIndex, NewMsgHeader ); {$i+}
   ioerr := ioresult
end;
if ioerr = 0 then
begin
   {$i-} flush( MsgIndex ); {$i+}
   ioerr := ioresult;
end;
if ioerr > 0 then
begin
   seek(msgIndex,0); { move away from edge?? }
   uwriteln('Error: Can''t save message');
   savemsgerr := 1;
   if ioerr = $f0 then
   begin
      uwriteln('Disk full');
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
(*str(blockNum, UWtemp);UWriteln('seek to >'+ UWtemp);*)
   {$i-} seek( MsgText, BlockNum); {$i+}
   ioerr := ioresult;
   if ioerr = 0 then
   begin
      {$i-} Blockwrite( MsgText, Line^[I][1], 1 ); {$i+}
      ioerr := ioresult
   end;
   if ioerr > 0 then
   begin
      uwriteln('Error: Can''t save all of message');
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

 { update filedataBlock }
 SEEK( MsgText, 0 );
 if ioerr = 0 then
   Blockwrite( MsgText, filedataBlock, sizeofdatablock DIV 128 )
 else
   Blockread( MsgText, filedataBlock, sizeofdatablock DIV 128 );

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
   reset(msgtext);

   if area.attribute in [1,2] then
      newmailmsg := true;

end;
   { finished saving }

   exitsavemsg:;
   {$i-} flush( MsgIndex ); {$i+}
   ioerr := ioresult; { so what? }

   str(savemsgerr,uwtemp);
   if logactivity then
      if savemsgerr <> 0 then
         writeln(TempLog,'! Error '+uwtemp+' saving msg')
      else
         writeln(TempLog,'  Enter message');

end; {savemsg }
