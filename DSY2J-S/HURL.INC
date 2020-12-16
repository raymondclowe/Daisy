{ 16/6/88 }

OVERLAY procedure Hurl(hurlmsgnum:integer);
label exithurl, nomaproomexit;
var toarea : integer;
    verr : integer;
    careasnum:integer; {temp}

var SB, { scanBlock }
    LS, { largestspace }
    NFB { NumfreeBlocks } : integer;

var BlockNum : integer;
    i : integer;
    fsize,nummatches,lastmatch:integer;
    temparea:areatype;

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
   Blockmap[ I DIV 8 ] := Blockmap[ I DIV 8 ] OR (1 SHL (i MOD 8))
end; {ab}


begin
   new(Line);

   ugotoxy(0,user.consolehight-2);
   uwtemp := namecase( ask('Destination area : ',14));
   if uwtemp = '' then goto exitHurl;

   reset(areasf);
   fsize := filesize(areasf)-1;

   i := 0; nummatches:=0; lastmatch :=0;
   repeat
      inc(i);
      seek(areasf,i);
      blockread(areasf,temparea,1);

      if (copy(temparea.selector,1,length(uwtemp)) = uwtemp ) and (user.priv >= temparea.accesspriv ) then
      begin
         inc(nummatches); lastmatch :=i
      end;
      monitor
   until (i = fsize) or (lastmatch>0) or (eus);
   if (lastmatch=0) or (nummatches > 1) then
   begin
      uwriteln('No such area '''+uwtemp+'''');
      goto exitHurl;
   end;

   toarea := i;

   seek( MsgIndex, hurlMsgNum);
   Read( MsgIndex, NewMsgHeader);
   if NewMsgHeader.fromperson[1] = 'd' then
      goto exitHurl
   else
   begin
      linenum := 0;
      if NewMsgHeader.pointertobody > 0 then
      begin
         linenum := 1;
         seek(msgtext,NewMsgHeader.pointertobody);
         blockread(msgtext, line^[linenum][1], 1); line^[linenum][0] := char(128);
         while line^[linenum][128] <> #00 do
         begin
            inc(linenum);
            blockread(msgtext, line^[linenum][1], 1); line^[linenum][0] := char(128);
            monitor
         end
      end;

      closemsgfiles;
      careasnum := areanum;
      areanum := toarea;
      openmsgfiles;

      if area.attribute = 1 then { hurling to net area }
      begin
         repeat
            ugotoxy(0,user.consolehight-2); uclreol;
            uwtemp := ask('Net/Node or ? : ',11);
            if uwtemp = '?' then displayfile(helpdrv+':HELP5.BBS');
         until uwtemp <> '?';
         if pos('/',uwtemp) = 0 then
         begin
            newmsgheader.destnet := thisnet;
            val(uwtemp,newmsgheader.destnode,verr);
            if (verr <> 0) or (uwtemp='') then newmsgheader.destnode := thisnode
         end
         else
         begin
            val(copy(uwtemp,pos('/',uwtemp)+1,maxint),newmsgheader.destnode,verr);
            delete(uwtemp,pos('/',uwtemp),maxint);
            val(uwtemp,newmsgheader.destnet,verr)
         end;

         ugotoxy(0,user.consolehight-2); uclreol;
         if askYN('Set SENT flag','Y') = 'Y' then
            newmsgheader.attribute := newmsgheader.attribute + [sent]
         else
            newmsgheader.attribute := newmsgheader.attribute - [sent];

         if NewMsgHeader.fromperson = 'Sysop' then
         begin
            seperatorset:=seperatorset-[' '];
            ugotoxy(0,user.consolehight-2); uclreol;
            NewMsgHeader.fromperson := namecase(ask('From: ',36));
            seperatorset:=seperatorset+[' '];
            if NewMsgHeader.fromperson = '' then
            begin
               str(thisnet,uwtemp);
               NewMsgHeader.fromperson := 'Sysop of '+uwtemp;
               str(thisnode,uwtemp);
               NewMsgHeader.fromperson := NewMsgHeader.fromperson+'/'+uwtemp
            end
         end

      end; { area.attribute = 1 }

      newmsgheader.replyto := 0;

      savemsgerr := 0;

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

         {begin module find free space}
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
            UWriteln('No map room');  { can't find enough room in text block directory }
            savemsgerr := 5;
            goto nomaproomexit;
         end;
         NewMsgHeader.pointertoBody := LS
         {end module find free space}

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
         SEEK(MSGINDEX,0); { move away from edge?? }
         uwriteln('Error: Can''t save message.');
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
            Blockwrite( MsgText, filedataBlock, sizeofdatablock Div 128 )
         else
            Blockread( MsgText, filedataBlock, sizeofdatablock Div 128 );
      end;

      if ioerr = 0 then
      begin
         { Inc and save highestmsgnum and number of messages }
         Inc( highestmsgnum );
         inc( nummsgs);
         updatemsgstats;
      end;

      ugotoxy(0,user.consolehight-2); uclreol;
      if savemsgerr = 0 then
      begin
         uwriteln('Message hurled');
         if logactivity then writeln(TempLog,'  Hurl message')
      end
      else
         uwriteln('Not hurled');
      closemsgfiles;
      areanum := careasnum;
      openmsgfiles
   end;

exithurl:
nomaproomexit:

   dispose(line)
end; { hurl }
