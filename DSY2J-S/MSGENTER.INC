{ 23/4/88 }

procedure MsgEnter;

begin { msgenter }

   new(line);      { draw space from the heap for message text }
   uclrscr;

   MsgAbort := false;
   enterNewMsgHeader;
   if not MsgAbort then
   begin
      editmsg;
      if not Msgabort then
      begin
         if not eus then savemsg
      end
      else
         UWriteln('Message aborted'); { because 'A' command, or nul to: }
   end;

   dispose(line)
                                               
(*;for I := 0 to 32 do begin write(I:3,':',ba(I):5); if (I+1) mod 8 = 0 then writeLN end;*)
end; { MsgEnter }
