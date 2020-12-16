{ 27/9/88 }
{$w1}
overlay procedure DisplayMsg( DisplayMsgNum : integer);
var msgchanged : boolean;
begin
   monitor;
   UInterupted := false;

   { while displaying msgs any of these characters, which are valid commands
     at the msg prompt, may interupt the display }
(*   validset := ['A','B','E','F','H','I','K','M','N','P','Q','R','S','T',
                'a','b','e','f','h','i','k','m','n','p','q','r','s','r',
                '*','<','>','=','?',' ',^H,^I,^U,cr,'1'..'9','#'];*)
   UCtrlSC;
   if not (DisplayMsgNum in [1..HighestMsgNum]) then
   begin
      if continuous then
      begin
         continuous := false;
         uwrite('Finished dump');
         if userconsoletype <> dumb then begin upressenter; uwriteln('') end;
         uwriteln(''); uwriteln('')
      end
      else
      begin
         if DisplayMsgNum > HighestMsgNum then
         begin
            uwrite('Highest message. ');

            { jump over private msgs at top of area }
            user.highestread[areanum] := HighestMsgNum;
            LastReadMsgNum := HighestMsgNum;
(*            ReplyMsgHeader := NulMsgHeader;*)
            Userchanged := true
         end
         else
            uwrite('Lowest message. ');
         if userconsoletype = dumb then uwriteln('')
      end
   end
   else { is an ok Number }
   begin
      seek( MsgIndex, DisplayMsgNum);
      Read( MsgIndex, ReadMsg^);
      msgchanged := false;

      if not (
          { deleted }
         (ReadMsg^.fromperson[1] = 'd') or

          { is mask and not match }
         ((maskstring > '') and
          (pos(maskstring,upstring(readmsg^.fromperson+readmsg^.toperson+readmsg^.subj)) = 0)) or

          { private and not read private priv and not right person }
         ((private in readmsg^.attribute) and (user.priv < area.readprivatepriv) and
          (upstring(user.name) <> upstring(ReadMsg^.fromperson)) and (upstring(user.name) <> upstring(ReadMsg^.toperson))) or

          { in transit and not read private priv }
         (intransit in readmsg^.attribute) and (user.priv < area.readprivatepriv)

         ) then
     begin { is ok to display }
         replyMsgheader := readMsg^;         { just in case need to reply to this }
         replymsgnum := displaymsgnum;
         LastreadMsgNum := DisplayMsgNum;
         if DisplayMsgNum > user.highestread[areanum] then
         begin
             user.highestread[areanum] := DisplayMsgNum;
             Userchanged := true
         end;

         WITH ReadMsg^ DO
         begin
            if not continuous then uclrscr; (*uhome*)
(*            uclreol;*)
            str(DisplayMsgNum, UWtemp);UWrite( '#'+ UWtemp);
            if counttimesread then
            begin
(*               if timesread < maxint then
               begin*)
               msgchanged := true;
               inc(timesread);
(*               end;*)
               str(timesread:4, UWtemp);UWrite( UWtemp);
            end;
            uwrite(' '+dateentered+' ');
            if recd in attribute then uwrite('Rcv''d ');
            if private in attribute then uwrite('Prvt ');
            if area.attribute = 1 then
            begin
               if sent in attribute then uwrite('Sent ');
               if Localmsg in attribute then uwrite('Local ');
               if fileattached in attribute then uwrite('FileAttached ');
               if killsent in attribute then uwrite('Kill/Sent ');
               if intransit in attribute then uwrite('InTransit ');
            end;
            uwriteln(''); (*uclreol;*)
            if (toperson = user.name) (*and (not (recd in attribute))*) then
            begin
               msgchanged := true;
               attribute := attribute + [recd]
            end;
            if msgchanged then
            begin
               seek( MsgIndex, DisplayMsgNum);
               write( MsgIndex, ReadMsg^)
            end;

(*            uclreol;*)
            UWrite('From: '+fromPerson);
            if area.attribute = 1 then
            begin
               str(orignet,uwtemp); uwrite(' at '+uwtemp+'/');
               str(orignode,uwtemp); uwriteln(uwtemp);
            end
            else
               uwriteln('');

            topoftext := 3;

            if (toperson > '') and (not uinterupted) then
            begin
(*               uclreol;*)
               UWrite('To:   '+toPerson);
               if area.attribute = 1 then
               begin
                  str(destnet,uwtemp); uwrite(' at '+uwtemp+'/');
                  str(destnode,uwtemp); uwriteln(uwtemp);
               end
               else
                  uwriteln('');
               topoftext := topoftext + 1
            end;

            if (replyto <> 0) and (not uinterupted) then
            begin
(*               uclreol;*)
               str(replyto,uwtemp);
               Uwriteln('REPLY To #'+uwtemp);
               topoftext := topoftext + 1
            end;

            if (subj > '') and (not uinterupted) then
            begin
(*               uclreol;*)
               if fileattached in attribute then
                  uwrite('File(s): ')
               else
                  uwrite('Subj: ');
               UWriteln(subj);
               topoftext := topoftext + 1
            end;

            if (pointertobody <> -1) and (not brief) and (not uinterupted) and (not EUS) then
            begin
(*               uclreol;*)
               UWriteln('');
               displaytext( pointertobody);
               if uinterupted then
               begin
                  flushinputbuffer;
                  if continuous then
                  begin
                     if userconsoletype <> dumb then
                        begin upressenter; uwriteln('') end;
                     continuous := false
                  end
               end
            end
            else
            begin
               ugotoxy(0,topoftext-1);
               clearrestofpage
            end;
            UWriteln(''); if not brief then UWriteln('');
         end
      end
      else
      begin { not ok to display, try next one unless interupted }
         if (not uinterupted) and (not eus) then
         begin
            if olddirection = 'N' then
               CallDisplayMsg( DisplayMsgNum + 1 )
            else
               callDisplayMsg( DisplayMsgNum - 1 )
         end
      end;
      if (continuous) and (not uinterupted) and (not eus) then
         if olddirection = 'N' then
            callDisplayMsg( DisplayMsgNum + 1 )
         else
            callDisplayMsg( DisplayMsgNum - 1 )
   end;
(*   validset := []*)
end; { Display message }
{$w0}
