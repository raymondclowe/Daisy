{ 8/7/88 }

{ mail search and inquire/find in overlay with other stuff }
{ also stuff for msgread }
OVERLAY procedure mailsearch;
type listtype = array[0..40] of integer;
var unread, oldfrom, oldto : listtype;
    msg : msgheadertype;
    i : integer;
    stri: st4;

procedure DispListOfNums(var L:listtype);
var i : integer;
    stri:st4;
begin
   if L[0] < 1 then
      Uwrite('NONE')
   else
   begin
      for i := 1 to L[0] do
      begin
         str(L[i],stri);
         uwrite(stri+', ');
         if i mod 9 = 0 then uwriteln('')
      end;
      if L[0] = 40 then uwrite('and more! Kill some!')
   end;
   uwriteln('')
end; { DispListOfNums}

begin {mail search}
   Uclrscr;
   str(HighestMsgNum, strHighestMsgNum );
   str(nummsgs, strnummsgs);
   str(User.highestRead[areanum], uwtemp);
   uwriteln(strnummsgs+' messages, highest is #'+strHighestMsgNum+', highest you read was #'+uwtemp);
   if askyn('Check mail in '+area.selector, 'N') = 'Y' then
   begin
      unread[0] := 0; oldfrom[0] := 0; oldto[0] := 0;
      uinterupted := false;
      i := highestmsgnum;
      uwrite('Looking at msg #');
      while (i > 0) and (not uinterupted) do
      begin
         str(i,stri);
         while length(stri) < 4 do
            stri := stri+' ';
         Uwrite(stri+bs+bs+bs+bs);
         seek(msgindex, i);
         read(msgindex, msg);
         if Msg.fromperson[1] <> 'd' then
         begin
            if (msg.fromperson = user.name) and (oldfrom[0] < 40) then
            begin
               inc(oldfrom[0]);
               oldfrom[oldfrom[0]] := I
            end;
            if msg.toperson = user.name then
               if recd in msg.attribute then
               begin
                  if oldto[0] < 40 then
                  begin
                     inc(oldto[0]);
                     oldto[oldto[0]] := I
                  end
               end
               else
                  if Unread[0] < 40 then
                  begin
                     inc(Unread[0]);
                     unread[Unread[0]] := I
                  end
         end;
         monitor;
         UctrlSC;
         dec(i)
      end;
      uwriteln('');
      uwriteln('New messages to you:'); DispListOfNums(unread);
      uwriteln('Old messages to you:'); DispListOfNums(oldto);
      uwriteln('Messages you have entered:'); DispListOfNums(oldfrom);
   end
end; { mail search }


OVERLAY procedure Inquire;
type listtype = array[0..40] of integer;
var searchstring : st40;
    found : listtype;
    msg : msgheadertype;
    i : integer;
    stri: st4;
    tempcharSet:set of char;
begin
   uclrscr;
   tempcharSet := seperatorSet; seperatorSet := []; askstrip := false;
   SearchString := ask('Search for: ',40);
   seperatorSet := tempcharSet; askstrip := true;
   SearchString := upstring(SearchString);
   if SearchString > '' then
   begin
      found[0] := 0;
      i := HighestMsgNum;
      UWriteln('');
      uwrite('Looking at msg #');
      uinterupted := false;
      while (i > 0) and (not uinterupted) do
      begin
         str(i,stri);
         while length(stri) < 4 do
            stri := stri+' ';
         Uwrite(stri+bs+bs+bs+bs);
         seek(msgindex, i);
         read(msgindex, msg);

         if not (
          { deleted }
         (Msg.fromperson[1] = 'd') or

          { not match }
         (pos(searchstring,upstring(msg.fromperson+msg.toperson+msg.subj)) = 0) or

          { private and not read private priv and not right person }
         ((private in Msg.attribute) and (user.priv < area.readprivatepriv) and
          (user.name <> Msg.fromperson) and (user.name <> Msg.toperson)) or

          { in transit and not read private priv }
         (intransit in Msg.attribute) and (user.priv < area.readprivatepriv)

         ) then
         begin
            uwriteln('');
            str(i, UWtemp);UWrite( '#'+ UWtemp);
            str(msg.timesread:4, UWtemp);UWrite( UWtemp);
            if recd in msg.attribute then uwrite('   Rcv''d');
            if private in msg.attribute then uwrite('   Prv''t');
            uwriteln('');
            UWriteln('From: '+msg.fromPerson);
            UWriteln('To:   '+msg.toPerson);
            UWriteln('Subj: '+msg.subj);
            uwrite('Looking at msg #');
            if found[0] < 40 then
            begin
               inc(found[0]);
               found[found[0]] := i
            end
         end;
         monitor;
         UctrlSC;
         dec(i)
      end;
      uwriteln('');
      Uwrite('Messages: ');
      if found[0] > 0 then
         for i := 1 to found[0] do
         begin
            str(found[I],uwtemp); uwrite(uwtemp+', ')
         end
      else
         uwriteln('NONE');
      UWriteln('')
   end;
   uwriteln('');uwriteln('');
   if user.consoletype <> dumb then begin uwriteln('');uwriteln('') end
end; {inquire}

