{Copyright (C) 10/8/1988 by Raymond Lowe}
{ version 2.5 - 02 Jan 1989 by Simmons Kwan }

procedure PACKMSGS;

label error, exit;

var TempIndex : file of MsgHeaderType;
    TempText  : file;


var fileDataBlock2 : array[ 1..512 ] of byte; (* 4 blocks *)
    Blockmap2      : array[ 0..sizeofblockmap ] of byte absolute fileDataBlock2;

var HighestMsgNum2 : integer; { the Number of the highest Msg, 0 when there is none }
    strHighestMsgNum2 : st4; { what happens if there are more than 9999 Msgs? }

    nummsgs2 : integer; { total number of messages excluding deleted ones }
    strnummsgs2 : st4;


const maxLines = 46;

type linetype = array [1..maxLines] of st128;

var Line : linetype;
    LineNum : integer; { this is also the Number of Lines }
    Msgabort : boolean;
    savemsgerr : byte;

var ioerr, err: integer;
    areaNumStr : st4;
    StrMax2keep : st4;
    Max2keep : integer;
    MsgNum : integer;
    j, temp, bits, first2keep : integer;

    ch,yn:char;
    f:file;

var workdrv     : char;
    abort       : boolean;
    highestArea : integer;

    renumber    : boolean;

var dummymsgheader : msgheadertype;


procedure ReadWriteMsg(Msg:msgheadertype);
label exithurl;
var toarea : integer;
    verr : integer;
    careasnum:integer; {temp}

var SB, { scanBlock }
    LS, { largestspace }
    NFB { NumfreeBlocks } : integer;

var BlockNum : integer;
    i : integer;

var ioerr : integer;

{ Block allocated ? }
function BA(POINT:INTEGER): boolean;
begin
   BA := odd( Blockmap2[ point DIV 8 ] shr (point mod 8) )
end;

{ allocate Block }
procedure AB( I : integer);
begin
   Blockmap2[ I DIV 8 ] := Blockmap2[ I DIV 8 ] OR (1 SHL (i MOD 8))
end; {ab}


begin { ReadWriteMsg }
   write('Copying message ',MsgNum,' ',^M);

   ioerr := 0;

   { read in message }
   linenum := 0;
   if Msg.pointertobody > 0 then
   begin
      linenum := 1;
      {$i-} seek(msgtext,Msg.pointertobody); {$i+}
      ioerr := ioresult;
      if ioerr = 0 then
      begin
         {$i-} blockread(msgtext, line[linenum][1], 1); {$i+}
         ioerr := ioresult
      end;
      if ioerr <> 0 then
      begin
         fillchar(line[linenum][1],32,128);
         line[linenum] := cr+cr+cr+'  * Bad line error'+cr+cr;
      end;
      line[linenum][0] := chr(128);
      while line[linenum][128] <> #00 do
      begin
         inc(linenum);
         {$i-} blockread(msgtext, line[linenum][1], 1); {$i+}
         if ioresult <> 0 then
         begin
            fillchar(line[linenum][1],32,128);
            line[linenum] := cr+cr+cr+'  * Bad line error'+cr+cr;
         end;
         line[linenum][0] := chr(128)
      end
   end;

   { write out message }
   savemsgerr := 0;

   if LineNum > 0 then
   begin

      {begin module find free space in msg2}
      LS := 0;
      nfb := 0;
      SB := 0;              { bits Numbered 0-7; so blocks are 0-xxx }
      repeat
         while (BA( SB ) and ( NFB < LineNum )) and (SB < (sizeofblockmap*8+7)) do
         Inc(SB );

         if SB < (sizeofblockmap*8+7) then
         begin
            LS :=  SB;

            NFB := 0;
            while ((  not BA(SB) ) and ( NFB < LineNum )) or (SB > (sizeofblockmap*8+7)) do
            begin
               Inc( NFB );
               Inc( SB)
            end
         end

      until (SB > (sizeofblockmap*8+7)) or (NFB >= LineNum);
      if (NFB < LineNum) or (ls+linenum > sizeofblockmap*8+7 - 1 ) then
      begin
         Writeln('No map room');  { can't find enough room in text block directory }
         savemsgerr := 5;
         goto ExitHurl
      end;
      Msg.pointertoBody := LS
      {end module find free space}

   end
   else
      Msg.pointertoBody := -1; { null message text }

(********************************************************************)
   if renumber then { adjust reply-to pointers }
   begin
      if msg.replyto > 0 then
      begin
         if deltable[msg.replyto] then { and original msg is deleted }
            msg.replyto := 0 {set it to not have a reply }
         else
            msg.replyto := Table[msg.replyto]; {adjust the replyto num}
      end;
   end;
(********************************************************************)

   { begin module write Header }
   seek( TempIndex, highestmsgnum2 + 1 );
   {$i-} write( TempIndex, Msg ); {$i+}
   ioerr := ioresult;
   if ioerr = 0 then
   begin
      {$i-} flush( TempIndex ); {$i+}
      ioerr := ioresult;
   end;
   if ioerr > 0 then
   begin
      SEEK(TempIndex,0); { move away from edge?? }
      writeln('Error:');
      savemsgerr := 1;
      if ioerr = $f0 then
      begin
         writeln(' Disk full');
         savemsgerr := 2
      end
   End;
   { end module write Header }
   if (ioerr = 0) and (linenum>0) then
   begin    { begin module write Text }
            { var I : integer; }
      BlockNum := Msg.pointertoBody;
      i := 1;
      repeat
         seek( TempText, BlockNum);
         {$i-} Blockwrite( TempText, Line[I][1], 1 ); {$i+}
         ioerr := ioresult;
         if ioerr > 0 then
         begin
            writeln('Error: ');
            if ioerr = $f0 then writeln(' Disk full');
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

   end;

   if ioerr = 0 then
   begin
      { Inc and save highestmsgnum and number of messages }
      Inc( highestmsgnum2 );
      if Msg.fromperson[1] <> 'd' then inc( nummsgs2);

   end;

exithurl:

   if (ioerr > 0) and (savemsgerr=0) then savemsgerr := 255;

   if savemsgerr <> 0 then writeln('Error');

end; { hurl }

procedure CopyBack;
var tempBlock   : array [1..128] of byte;

begin
   writeln('Copying packed files back to original drive');

   reset(TempIndex);
   rewrite(msgindex);
   while not eof(TempIndex) do
   begin
      read(TempIndex,dummymsgheader);
      write(msgindex,dummymsgheader);
      write('i')
   end;
   close(TempIndex);
   close(msgindex);

   reset(TempText);
   rewrite(msgText);
   while not eof(TempText) do
   begin
      blockread(TempText,tempBlock,1);
      blockwrite(msgtext,tempBlock,1);
      write('t')
   end;
   close(TempText);
   close(msgtext);
   writeln;

end;

procedure OpenFiles;
begin
   writeln('Opening original files');

   OpenMsgFiles;

   if HighestMsgNum > (filesize(msgIndex)-1) then HighestMsgNum := (filesize(msgIndex)-1);

   writeln(' ',filesize(msgindex),' Filesize');
   writeln(' ',highestmsgnum,' Highestmsgnum');
   writeln(' ',numMsgs,' NumMsgs');

   writeln('Creating temporary files');

   assign(TempIndex,workdrv+':MSGINX.$$$');
   assign(TempText,workdrv+':MSGTXT.$$$');

   DummyMsgHeader := nulmsgheader;
   DummyMsgHeader.pointertoBody := 0; { highest Msg num}
   DummyMsgHeader.timesread := 0; { num of messages }
   REwrite( TempIndex );
   write ( TempIndex, DummyMsgHeader );

   fillchar( filedatablock2, 512, 0 ); { set all to unused }
   blockmap2[ 0 ] := 15; { first 4 bits set : first 4 blocks used }
   REwrite( TempText );
   blockwrite( TempText, filedatablock2, 4 );

   HighestMsgNum2 := 0;
   Nummsgs2 := 0;

end;


procedure PackFiles;
begin

   fillchar(deltable,2000,0);
   fillchar(table,2000,0);

   MsgNum := 1;
   saveMsgErr := 0;
   while (MsgNum <= highestmsgnum) and (savemsgerr=0) do
   begin

      {$I-} seek( MsgIndex, MsgNum); {$i+}
      ioerr := ioresult;
      if ioerr = 0 then
      begin
         {$i-} Read( MsgIndex, Msg); {$i+}
         ioerr := ioresult
      end;
      { added - first2keep }
      if ((ioerr <> 0) or (msgNum < first2keep)) then
      begin
         Msg := nulmsgheader;
         Msg.fromperson := 'd'; { deleted }
      end;

      if (Msg.fromperson[1] = 'd') then
      begin
         Msg.pointertobody := -1;
         deltable[MsgNum] := true
      end;

      if not ((Msg.fromperson[1] = 'd') and (renumber)) then
         ReadWriteMsg(Msg);

      table[MsgNum] := numMsgs2;

      MsgNum := MsgNum + 1
   end;
   if savemsgerr <> 0 then abort := true;

end;


procedure getfirst2keep;
begin
   writeln;
   write('First message to be kept is: ');
   if Nummsgs <= max2keep then
      first2keep := 1
   else
   begin
      Msgnum := HighestMsgNum;
      while (Max2keep > 0) and (MsgNum > 0) do
      begin
         {$I-} seek( MsgIndex, MsgNum); {$i+}
         ioerr := ioresult;
         if ioerr = 0 then
         begin
            {$i-} Read( MsgIndex, Msg); {$i+}
            ioerr := ioresult
         end;
         if ioerr <> 0 then
         begin
            Msg := nulmsgheader;
            Msg.fromperson := 'd'; { deleted }
         end;

         if (Msg.fromperson[1] <> 'd') then
            Max2Keep := Max2Keep - 1;

         MsgNum := MsgNum - 1;
      end;

      first2keep := MsgNum + 1;
   end;
   writeln(first2keep);
end;


begin { packmsgs }
   writeln;
   writeln('Pack Messages');
   writeln;
   workdrv := ' ';
   if ramdrv = ' ' then
   begin
      while not ( workdrv in ['A'..HighestDrv] ) do
      begin
         write('Use what drive for the temporary files? ');
         fillchar(tempch,sizeof(tempch),0);
         readln(tempch);
         workdrv := tempch[1];
         if workdrv in [#0,cr,esc] then goto exit
      end
   end
   else
      workdrv := ramdrv;

   reset(areasf);
   highestArea := sizeof(areasf);
   write('Pack which area #: ');
   repeat
      readln(areaNumStr);
      val(areaNumStr,areanum,err);
   until (areanum in [1..highestArea]) and (err = 0 );
   if (areaNumStr = '') or (err>0) then goto exit;
   writeln;

   repeat
     write('Renumber at the same time? ');
     buflen := 1;
     fillchar(tempch,sizeof(tempch),0);
     readln(tempch);
     yn := upcase(tempch[1]);
   until yn in ['Y','N'];
   renumber := ( yn = 'Y');

   OpenFiles;
   abort := false;

   { retain at most MAXNUM msgs - Simmons Kwan }
   writeln;
   repeat
      write('Remove old messages? ');
      buflen := 1;
      fillchar(tempch,sizeof(tempch),0);
      readln(tempch);
      yn := upcase(tempch[1]);
   until yn in ['Y','N'];

   first2keep := 1;

   if (yn = 'Y') then
   begin
      repeat
         write('Maximum no. of messages to keep: ');
         readln(StrMax2keep);
         val(StrMax2keep,Max2keep,err);
      until (Max2keep > 0) and (err = 0 );
      getfirst2keep;
   end;

   PackFiles;

   if abort then
   begin
error: writeln('Error, aborted!');
       closemsgfiles;

      close( TempIndex ); close( TempText);
      erase(TempIndex);   erase(TempText);
   end
   else
   begin
      { update new file }
      DummyMsgHeader := nulmsgheader;
      DummyMsgHeader.pointertoBody := highestmsgnum2; { highest Msg num}
      DummyMsgHeader.timesread := nummsgs2; { num of messages }
      seek( TempIndex, 0);
      write ( TempIndex, DummyMsgHeader );
      seek(TempText,0);
      blockwrite( TempText, filedatablock2, 4 );

      closemsgfiles; {main files}

      close( TempIndex ); { close temp files }
      close( TempText);

      CopyBack;

      writeln('Erasing temporary files');
      erase(TempIndex); erase(TempText);

      writeln;
      if renumber then
      begin
         writeln('Updating user list');
         pass3;
         writeln;
      end;
      writeln('Done.');
      writeln;
   end;

exit:
end;
