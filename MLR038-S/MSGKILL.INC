{ 4/6/88 - mailer version }

{$w1}
procedure msgKill( KillmsgNum : integer);

var msg : ^msgheadertype;
    i   : integer;
    line: ^st128;


{ D)eA)llocate B)lock }
procedure DAB( I : integer);
begin
(*   if (I < 0) OR (I > (sizeofblockmap*8+7) ) then
   begin
      str(I, UWtemp);
      Writeln('no such block >'+UWtemp);
      halt
   end;*)

   Blockmap[ I DIV 8 ] := Blockmap[ I DIV 8 ] and (255 xor (1 shl (i mod 8)))
end;

begin {killmsg}
   new(line); new(msg);

   seek( msgIndex, KillmsgNum);
   Read( msgIndex, Msg^);
   Msg^.fromperson[1] := 'd';
   seek( msgIndex, KillmsgNum);
   write( msgIndex, Msg^);

   WITH Msg^ DO
   begin
      if pointertobody <> -1 then
      begin
         Line^[0] := char( 128 );
         seek(msgtext, pointertobody);
         i := pointertobody;
         blockread(msgtext, Line^[1], 1);
         dab(i);
         while (ord(Line^[128]) <> 0) do
         begin
            inc(i);
            blockread(msgtext, Line^[1], 1 );
            dab(i)
         end;
      end
   end;

   { update filedataBlock on disk }
   SEEK( msgText, 0 );
   Blockwrite( msgText, filedataBlock, 4 );


   if killmsgnum = highestmsgnum then { scan highestmsgnum back }
   begin
      repeat
         dec( highestmsgnum );
         seek( msgIndex, highestmsgnum)(**01**);
         read( msgIndex, Msg^)
      until (Msg^.fromperson[1] <> 'd') or
            (highestmsgnum < 2);
      if (highestmsgnum = 1) and (Msg^.fromperson[1]='d') then
         dec(highestmsgnum)
   end;
   dec( nummsgs);
   updatemsgstats;
   dispose(line); dispose(msg);

end; { msgkill }
{$w0}
