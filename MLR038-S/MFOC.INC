{ 7/1/89 }

{ MFOC.INC = Message File Open/Close routines }

procedure openMsgfiles;
var msgtextfn, msgindexfn : ^st14;
    dummymsgheader:^msgheadertype;
    mif,mtf : ^boolean;
begin
(*   writeln('opening areas #',areanum);*)
   new(msgtextfn); new(msgindexfn); new(mif); new(mtf);

   reset(areasf);
   seek(areasf,areanum);
   blockread(areasf,Area,1);
   str(areanum,uwtemp);
   msgtextfn^ := Area.textdrive+':MSGTXT'+uwtemp+'.BBS';
   msgindexfn^ := Area.indexdrive+':MSGINX'+uwtemp+'.BBS';

   ASSIGN( MsgText, MsgTextFN^ );
   {$I-}RESet( MsgText );{$I+}
   MTF^ := IOResult=0;
   ASSIGN( MsgIndex, msgindexFN^ );
   {$I-}RESet( MsgIndex );{$I+}
   MIF^ := IOResult=0;

   if not (MIF^ and MTF^) then
   begin
      if (not MIF^) and (not MTF^) then { both of them do not exist}
      begin

         assign(msgindex,MsgINDEXFN^);
         {$I-} REwrite( msgindex ); {$I+}
         if IOResult <> 0 then
         begin
            writeln(cerr+MsgindexFN^);
            HALT
         end;
         NEW(DUMMYMSGHEADER);
         DummyMsgHeader^ := nulmsgheader;
         DummyMsgHeader^.pointertoBody := 0; { highest Msg num}
         DummyMsgHeader^.timesread := 0; { num of messages }

         write ( msgindex, DummyMsgHeader^ );

         DISPOSE(DUMMYMSGHEADER);
         seek(msgindex,0);

         assign(msgtext,MsgTextFN^);
         {$I-} REwrite( msgtext ); {$I+}
         if IOResult <> 0 then
         begin
            writeln(cerr+MsgTextFN^);
            HALT
         end;
         fillchar( filedatablock, 512, 0 ); { set all to unused }
         blockmap[ 0 ] := 15; { first 4 bits set : first 4 blocks used }
         blockwrite( msgtext, filedatablock, 4);
         seek(msgtext,0)
      end
      else
      begin { just one is missing }
         writeln('');
         writeln('Can only find one of the message files, BOTH must be available.');
         HALT
      end
   end;

   close(areasf);

   new(dummymsgheader);
   read( msgindex, dummymsgheader^);
   HighestMsgNum := dummymsgheader^.pointertobody;
   Nummsgs := dummymsgheader^.timesread;
(*;WRITELN('HIGHESTMSGNUM>',HighestMsgNum);*)
(*;WRITELN('NUMMSGS>',Nummsgs);*)
   dispose(dummymsgheader);

   blockread( MsgText, filedatablock, 4 );

   dispose(msgtextfn); dispose(msgindexfn); dispose(mif); dispose(mtf)
end;

Procedure updatemsgstats;
var dummymsgheader : ^msgheadertype;
begin
   new(dummymsgheader);
   dummymsgheader^.pointertobody := HighestMsgNum;
   dummymsgheader^.timesread := Nummsgs;
   seek( msgindex, 0);
   write( msgindex, dummymsgheader^);

   reset(areasf);
   seek(areasf,areanum);
   blockread(areasf,Area,1);
   area.highmsg := highestmsgnum;
   seek(areasf,areanum);
   blockwrite(areasf,area,1);
   close(areasf);
   dispose(dummymsgheader)
end;

procedure closeMsgfiles;
begin
   close( MsgIndex ); { pay attention to closing files; don't forget any! }
   close( MsgText)
end;

