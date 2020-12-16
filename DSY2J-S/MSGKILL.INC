{ 16/6/88 }

overlay procedure MsgKill( KillMsgNum : integer);

var Msg : Msgheadertype;
    i   : integer;
    line:st128;
    kill : boolean;

{ D)eA)llocate B)lock }
procedure DAB( I : integer);
begin
   if (I < 0) OR (I > (sizeofblockmap*8+7) ) then
   begin
      str(I, UWtemp);
      UWriteln('no such block >'+UWtemp);
      halt
   end;
   Blockmap[ I DIV 8 ] := Blockmap[ I DIV 8 ] and (255 xor (1 shl (i mod 8)))
end;

begin {killmsg}
   str(KillMsgNum,uwtemp);
   kill := true;
   if user.singlekey then
      kill := (askyn('Kill message #'+uwtemp,'N') = 'Y');
   if kill then
   begin

      seek( MsgIndex, KillMsgNum);
      Read( MsgIndex, Msg);
      if msg.fromperson[1] <> 'd' then
         if (Msg.fromperson = user.name) or (Msg.toperson = user.name) or
            (user.priv >= area.killanypriv) then
         begin
            Msg.fromperson[1] := 'd';
            seek( MsgIndex, KillMsgNum);
            write( MsgIndex, Msg);

            if Msg.pointertobody <> -1 then
            begin
               i := Msg.pointertobody;
               seek(Msgtext, i);
               blockread(Msgtext, Line[1], 1);
               dab(i);
               while (ord(Line[128]) <> 0) do
               begin
                  inc(i);
                  blockread(Msgtext, Line[1], 1 );
                  dab(i)
               end;
            end;

            { update filedataBlock on disk }
            seek( MsgText, 0 );
            Blockwrite( MsgText, filedataBlock, sizeOfDataBlock div 128 );

            ugotoxy(0,user.consolehight-2);
            uclreol;
            str(KillMsgNum,UWtemp);
            UWrite('Message #'+UWtemp+' Killed. ');
            if user.consoletype = dumb then uwriteln('');
            replymsgheader := nulmsgheader;

            if killmsgnum = highestmsgnum then { scan highestmsgnum back }
            begin
               repeat
                  dec( highestmsgnum );
                  seek( MsgIndex, highestmsgnum);
                  read( MsgIndex, Msg)
               until (msg.fromperson[1] <> 'd') or { complete scan even if carrier lost }
                     (highestmsgnum < 2);
               if (highestmsgnum < 2) and (msg.fromperson[1]='d') then
                  dec(highestmsgnum)
            end;
            dec( nummsgs);
            if logactivity then writeln(TempLog,'  Kill message');
            updatemsgstats
         end
         else
            UWriteln('Not your message!');
   end
(*;for I := 0 to 32 do begin write(I:3,':',ba(I):5); if (I+1) mod 8 = 0 then writeLN end;*)
end; { msgkill }
