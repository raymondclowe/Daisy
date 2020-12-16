{ 8/7/88 }

{ MFOC.INC = Message File Open/Close routines }

overlay procedure openMsgfiles;
var msgtextfn, msgindexfn : ^st14;
    dummymsgheader:^msgheadertype;
    mif,mtf : boolean;
begin
(*   writeln('opening areas #',areanum);*)
   new(msgtextfn); new(msgindexfn);

   reset(areasf); { don't see why i have to do this, it is already open! }
   UServeBuff;
   seek(areasf,areanum);
   blockread(areasf,Area,1); { area data is kept in memory while area is open }
   UServeBuff;
   str(areanum,uwtemp);
   monitor;
   msgtextfn^ := Area.textdrive+':MSGTXT'+uwtemp+'.BBS';
   msgindexfn^ := Area.indexdrive+':MSGINX'+uwtemp+'.BBS';

   ASSIGN( MsgText, MsgTextFN^ );
   {$I-}RESet( MsgText );{$I+}
   MTF := IOResult=0;
   UServeBuff;
   ASSIGN( MsgIndex, msgindexFN^ );
   {$I-}RESet( MsgIndex );{$I+}
   MIF := IOResult=0;
   UServeBuff;
   monitor;

   if not (MIF and MTF) then
   begin
      if (not MIF) and (not MTF) then { both of them do not exist}
      begin

         assign(msgindex,MsgINDEXFN^);
         {$I-} REwrite( msgindex ); {$I+}
         if IOResult <> 0 then
         begin
            uwriteln(cerr+MsgindexFN^);
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
         blockwrite( msgtext, filedatablock, sizeofdatablock DIV 128);
         seek(msgtext,0)

      end
      else
      begin { just one is missing }
         uwriteln('');
         uwriteln('Can only find one of the message files, BOTH must be available.');
         HALT
      end
   end;
   { at this point either message files exist or have been created }

   new(dummymsgheader);
   read( msgindex, dummymsgheader^);
   UServeBuff;
   HighestMsgNum := dummymsgheader^.pointertobody;
   Nummsgs := dummymsgheader^.timesread;
   area.highmsg := highestmsgnum; { make sure this is correct }
(*;WRITELN('HIGHESTMSGNUM>',HighestMsgNum);*)
(*;WRITELN('NUMMSGS>',Nummsgs);*)
   dispose(dummymsgheader);

   monitor;
   blockread( MsgText, filedatablock, sizeofdatablock div 128 );
   UServeBuff;

   dispose(msgtextfn); dispose(msgindexfn);
end;

overlay Procedure updatemsgstats;
var dummymsgheader : ^msgheadertype;
begin
   new(dummymsgheader);
   dummymsgheader^.pointertobody := HighestMsgNum;
   dummymsgheader^.timesread := Nummsgs;
   seek( msgindex, 0);
   monitor;
   write( msgindex, dummymsgheader^);
   dispose(dummymsgheader)
end;

overlay procedure closeMsgfiles;
var dummymsgheader : ^msgheadertype;
begin
   UServeBuff;
   close( MsgIndex ); { pay attention to closing files; don't forget any! }
   UServeBuff;
   close( MsgText);
   UServeBuff;
   monitor;

   new(dummymsgheader);
   reset(areasf);
   UServeBuff;
   area.highmsg := highestmsgnum;
   seek(areasf,areanum);
   UServeBuff;
   blockwrite(areasf,area,1);
   monitor;
   dispose(dummymsgheader)

end;