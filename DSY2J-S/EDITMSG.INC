{ 23/9/88 }

OVERLAY procedure enterNewMsgHeader;
label ExitEnter;
type soc = set of char;
var tempcharSet : Soc;
    verr : integer;

type nodenametype = st20;
     phonetype = st20;
     nodetype = record
                   NetNumber,
                   nodeNumber,
                   routeToNet,
                   routeTonode,
                   baudrate:integer;
                   name:nodenametype;
                   phoneNumber:phonetype;
                   KEYword:ST6;
                   hold:boolean;
                   fill:array [1..10] of char;
                end;

var nodelist : file of nodetype;
    s : nodetype;

begin
   NewMsgHeader := nulmsgheader; { clear all fields }

   str( highestmsgnum + 1, UWtemp);
   UWriteln('This will be message #'+UWtemp);
   if area.readprivatepriv >= area.accesspriv then
      if askyn('Private','N') = 'Y' then
         NewMsgHeader.attribute := NewMsgHeader.attribute + [private];

   if (area.attribute in [1,2]) and {net or echo}
      (user.name = 'Sysop') then
   begin
      seperatorset:=seperatorset-[' '];
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
   else
   begin
      NewMsgHeader.fromPerson := User.name;
      UWriteln('From: '+NewMsgHeader.fromPerson)
   end;

   NewMsgHeader.orignet := thisnet;
   NewMsgHeader.orignode := thisnode;

   if not reply then
   begin
      SeperatorSet := SeperatorSet - [' '];
      repeat
         NewMsgHeader.ToPerson := namecase(Ask('To (? for list): ',36));
         if NewMsgHeader.ToPerson = '?' then viewuserlist;
         monitor
      until (NewMsgHeader.ToPerson <> '?') or eus;
      SeperatorSet := SeperatorSet + [' '];
      if area.attribute = 1 then
      begin
         repeat
            uwtemp := ask('Net/Node or ? : ',11);
            if uwtemp = '?' then displayfile(helpdrv+':HELP5.BBS');
         until (uwtemp <> '?') or (eus);
         if pos('/',uwtemp) = 0 then
         begin
            NewMsgHeader.destnet := thisnet;
            val(uwtemp,NewMsgHeader.destnode,verr);
            if (verr <> 0) or (uwtemp='') then NewMsgHeader.destnode := thisnode
         end
         else
         begin
            val(copy(uwtemp,pos('/',uwtemp)+1,maxint),NewMsgHeader.destnode,verr);
            delete(uwtemp,pos('/',uwtemp),maxint);
            val(uwtemp,NewMsgHeader.destnet,verr)
         end;
         str(NewMsgHeader.destnet,uwtemp);
         assign(nodelist,nodelistdrv+':NODELIST.BBS');
         {$i-} reset(nodelist); {$i-}
         if ioresult = 0 then
         begin
            s.netnumber := 0; s.nodenumber := 0;
            while (not eof(nodelist)) and
                  (not ((s.netnumber=NewMsgHeader.destnet)and(s.nodenumber=NewMsgHeader.destnode))) do
               read(nodelist,s);
            if not ((s.netnumber=NewMsgHeader.destnet)and(s.nodenumber=NewMsgHeader.destnode)) then
            begin
               uwrite('No node '+uwtemp);
               str(NewMsgHeader.destnode,uwtemp);
               uwriteln('/'+uwtemp);
               msgAbort := true;
               goto ExitEnter
            end
            else
               uwriteln('Send to: '+s.name);
         end
         else
         begin
            uwrite('Send to: '+uwtemp);
            str(NewMsgHeader.destnode,uwtemp);
            uwriteln('/'+uwtemp);
         end;
         if ((NewMsgHeader.destnet = thisnet) and
            (NewMsgHeader.destnode = thisnode)) or
            ((NewMsgHeader.destnet = altnet1) and
            (NewMsgHeader.destnode = altnode1)) then
            NewMsgHeader.attribute := NewMsgHeader.attribute + [localmsg];

      end
   end
   else
   begin
      if replymsgheader.fromperson = NewMsgHeader.fromperson then
      begin
         NewMsgHeader.toperson := replymsgheader.toperson;
         newmsgheader.destnet :=  replymsgheader.destnet;
         newmsgheader.destnode :=  replymsgheader.destnode
      end
      else
      begin
         NewMsgHeader.toperson := replymsgheader.fromperson;
         newmsgheader.destnet :=  replymsgheader.orignet;
         newmsgheader.destnode :=  replymsgheader.orignode
      end;
      if area.attribute <> 1 then
      begin
         newmsgheader.destnet :=  0;
         newmsgheader.destnode :=  0
      end;
      UWriteln('REPLY To: '+NewMsgHeader.toPerson);
      NewMsgHeader.replyto := replymsgnum
   end;

   if NewMsgHeader.toPerson = '' then goto exitenter;

   if area.attribute = 1 then
   begin
      if user.priv >= area.killsentPriv then
         if askYN('Kill after sending','N') = 'Y' then
            NewmsgHeader.attribute := NewmsgHeader.attribute + [killsent];

     if user.priv >= area.fileattachpriv then
         if askYN('Attach file','N') = 'Y' then
            NewmsgHeader.attribute := NewmsgHeader.attribute + [fileattached];
   end;

   if (not reply) or (fileattached in newmsgheader.attribute) then
   begin
      tempcharSet := seperatorSet; seperatorSet := [];
      askstrip := false;
      if fileattached in newmsgheader.attribute then
            NewMsgHeader.subj := Ask('File: ',70)
      else
            NewMsgHeader.subj := Ask('Subj: ',70);
      seperatorSet := tempcharSet;
      askstrip := true
   end
   else
   begin
      if (upstring(copy(replymsgheader.subj,1,4)) <> 'RE: ') and (replymsgheader.subj>'') then
         NewMsgHeader.subj := copy('Re: '+replymsgheader.subj,1,69)
      else
         NewMsgHeader.subj := replymsgheader.subj;
      uwriteln('Subj: '+NewMsgHeader.subj)
   end;

ExitEnter:

end;


OVERLAY procedure editmsg;

label exiteditmsg;

var Commandst: st4;

type soc = set of char;
var tempcharSet : ^Soc;
    i,verr : integer;
    startline,err : integer;
    ok : boolean;

var edLineNum,
    positionofoldstring,
    Numerr : integer;
    oldstring,
    newstring : ^st128;
    prompt : st40;

const wln : st20 = ' what line number : ';

{ begin module EnterText }

procedure GetText;
var I, hardmargin (*, softmargin*) : INTEGER;
    ch:char;
    thisline : st128;
    thislinelen : byte absolute thisline;

procedure splitline(var L1,l2 : st128 );
var i : integer;
    L1len : byte absolute L1;
    l2len : byte absolute l2;
begin
   i := L1len;
   while (not (L1[i] = ' ')) and (i>0) do
   begin
      i := pred(i);
      uservebuff
   end;
   if i>0 then
   begin
      l2:=copy(L1,succ(i),maxint);
      L1len:=i
   end;

   { allows for a separator+CR sequence right at the end of the line }
   { without this the L2 would consist of just the CR }
   if l2len=1 then
   begin
      if l2 = CR then
      begin
         L1 := L1 + cr;
         l2 := ''
      end
   end
end; { split line }

begin { gettext }

   ctrls := false; ctrlc := false;

   if LineNum = 1 then UWriteln('Enter your message, blank line to end.');

   Line^[LineNum] := '';
   dec( LineNum );
   hardmargin := user.consolewidth-6;

(*   if userlocation = remote then
   softmargin := hardmargin;
   else
      softmargin := hardmargin-8;*)

   repeat

      if LineNum >= 1 then
      begin
         line^[linenum+1] := '';
         if (length( Line^[LineNum] ) > hardmargin ) and
            ( not (Line^[LineNum][hardmargin+1] = ' ' )) then
         begin
            splitline(Line^[LineNum],Line^[LineNum+1]);
            for I := 1 to length(Line^[LineNum+1] ) do UWrite(BS);
            if user.consoletype = dumb then
               for I := 1 to length(Line^[LineNum+1] ) do UWrite(' ')
            else
               uclreol
         end
      end;

      LineNum := succ(linenum);
      thisline := line^[linenum]; { move to static var for fast access }

      UWriteln('');
      str(LineNum:2, UWtemp);
      uwrite(UWtemp+': '+thisline);
      linecount := 0; { prevent "more?" during entry }
{----}repeat
         UGetChar(ch);
         if ch in [' '..'~'] then
         begin
            thisline := thisline + ch;
{this}      UWrite(ch)
{is}     end
{the}    else
{most}      if (ch in [BS,DEL]) and (thislineLen > 0) then
{time}      begin
{critical}     thislinelen := pred(thislinelen);
{section}      UWrite(BS+' '+BS)
            end
            else
               if ch in [CAN,NAK] then
               begin
                  UWriteln('\');
                  str(LineNum:2, UWtemp);
                  thisline := '';
                  uwrite(UWtemp+': '+thisline);
               end;
         monitor;
      until (*( thislinelen > softmargin ) and (ch = ' ')) or*) { softmargin }
            ( thislinelen > hardmargin ) or
{----}      ( ch = CR ) or (EUS);
      if ch = CR then thisline := thisline + CR;

      line^[linenum] := thisline;      { move back to array }
      monitor
   until (Line^[LineNum] = CR) or (Linenum = maxLines) or (EUS);
   if Line^[LineNum] = CR then Line^[LineNum] := ''; { don't actually want the CR in the last Line }
   UWriteln('');
   ctrls := true; ctrlc := true

end; {get Msg Text }

procedure displayMsgText(start:integer);
var I : integer;
    uwlen : byte absolute uwtemp;
begin
   uinterupted := false;
   i := start;
   while (i <=linenum) and (not eus) and (not UInterupted) do
   begin
      str(I:2, UWtemp);
      UWtemp := UWtemp +': '+ Line^[I];
      while uwtemp[uwlen] = cr do uwlen := pred(uwlen);
      UWriteLN(UWtemp);
      inc( i );
      uctrlsc;
      monitor
   end
end;

{ end module EnterText }


begin { editmsg }

   if NewMsgHeader.toPerson = '' then
   begin
      msgabort := true; {because nul to:}
      goto exiteditmsg
   end;

   case user.help of
    1 : prompt := 'EDIT: ';
    2 : prompt := 'L C A T J S D I E P # or ?: ';
    3 : prompt := 'L)ist C)ont A)bort S)ave or ? for help: ';
   end;


      {EnterText}
      LineNum := 1;
      getText;

      { begin module Msgedit }
      repeat { outer loop; until User finished with editor; save or abort }
         repeat { Getting a valid Command }
            repeat
               UWriteln('');
               Commandst := Upstring(Ask(prompt,2));
(**)IF (COMMANDST = 'M') and (user.priv=100) THEN
BEGIN STR(MEMAVAIL,UWTEMP); UWRITELN('MEMAVAIL = '+UWTEMP) END;(**)
               monitor
            until (Commandst > '') or (EUS);
            val(commandST,startline,err);
            if err >0 then startline := 0;
            ok := CommandST[1] in ['L','S','A','C','T','J','D','I','E','P','?'];
            if (not ok) and
               (not (startline in [1..Linenum])) then notacommand;
            monitor
         until (ok) or
               (startline in [1..Linenum]) or (EUS);

         if startline in [1..Linenum] then DisplayMsgText(startline);
         {none of the others are numbers so they won't be activated}

         Case Commandst[1] of
          '?' : displayfile(helpdrv+':HELP6.BBS');
          'P' : if area.readprivatepriv >= area.accesspriv then
                   if private in NewMsgHeader.attribute then
                   begin
                      NewMsgHeader.attribute := NewMsgHeader.attribute - [private];
                      UWriteln('Public')
                   end
                   else
                   begin
                      NewMsgHeader.attribute := NewMsgHeader.attribute + [private];
                      UWriteln('Private')
                   end;
(*                else
                   uwriteln('Private not allowed here');*)
          'A' : if Askyn('Abort', 'N') = 'N' then CommandST[1] := ^@; {dummy; invalid Command}
          'L' : begin
                   uclrscr;
                   str( highestmsgnum + 1, UWtemp);
                   UWrite('#'+UWtemp);
                   if private in NewMsgHeader.attribute then uwrite('   Prvt');
                   uwriteln('');
                   UWriteln('From: '+NewMsgHeader.fromPerson);
                   UWriteln('To:   '+NewMsgHeader.toPerson);
                   uwriteln('Subj: '+NewMsgHeader.subj);
                   displayMsgText(1);
                end;
          'T' :
begin
   uwriteln('To: '+NewMsgHeader.ToPerson);
   SeperatorSet := SeperatorSet - [' '];
   repeat
      NewMsgHeader.ToPerson := namecase( Ask('To (? for list): ',36));
      if NewMsgHeader.ToPerson = '?' then viewuserlist;
      monitor
   until (NewMsgHeader.ToPerson <> '?') or eus;
   SeperatorSet := SeperatorSet + [' '];
end;
          'J' : begin
                   uwriteln('Subj: '+NewMsgHeader.subj);
                   new(tempcharset);
                   tempcharSet^ := seperatorSet; seperatorSet := [];
                   askstrip := false;
                   NewMsgHeader.subj := Ask('Subj: ',73);
                   seperatorSet := tempcharSet^;
                   dispose(tempcharset);
                   askstrip := true
                end;
          'C' : getText;
          'D' : begin
                   edlinenum := 0;
                   val( Ask('Delete'+wln,2), edLineNum, Numerr);
                   if (Numerr = 0) and (edLineNum in [1..LineNum-1]) then
                   begin
                      for i := edLineNum to LineNum - 1 do
                         Line^[i] := Line^[I+1];
                      if (linenum = maxlines) and (Line^[linenum]>'') then
                         Line^[linenum] := ''
                      else
                         dec( LineNum)
                   end
                end;
          'I' : if (Linenum < maxLines) or ((Linenum = maxLines) and (Line^[Linenum] = '')) then
                begin
                   edlinenum := 0;
                   val( Ask('Insert'+wln,2), edLineNum, Numerr);
                   if (Numerr = 0) and (edLineNum in [1..LineNum-1]) then
                   begin
                      for i := Linenum downto edLineNum+1 do
                         Line^[i] := Line^[I-1];
                      if Linenum < maxLines then
                      begin
                         inc( LineNum);
                         Line^[Linenum] := ''
                      end;
                      Line^[edLinenum] := CR
                   end
                end;
          'E' : begin
                   edlinenum := 0;
                   val( Ask('Edit'+wln,2), edLineNum, Numerr);
                   if (Numerr = 0) and (edLineNum in [1..LineNum-1]) then
                   begin

str(edLinenum:2, UWtemp); UWriteLN(UWtemp+': '+ Line^[edLinenum]);

new(oldstring); new(newstring); new(tempcharset);
tempcharSet^ := seperatorSet; seperatorSet := []; askstrip := false;
oldString^ := ask('Old string: ',length(Line^[edLinenum]));
newString^ := ask('New string: ',127-length(Line^[edLinenum])+length(oldString^));
seperatorSet := tempcharSet^; askstrip := true;
(*if length(Line^[edLineNum]+newString^)-length(oldString^) > 128 then
   uwriteln('Too long, no room: tell sysop')
else*)
   if oldString^ = '' then
      Line^[edLineNum] := newString^ + Line^[EdLineNum]
   else
   begin
      positionofoldstring := pos(upString(oldString^),upstring(Line^[edLineNum]));
      if positionofoldstring = 0 then
         uwriteln('Can''t find ['+oldString^+']')
      else
      begin
         delete(Line^[edLineNum],positionofoldstring,length(oldString^));
         if newString^ > '' then insert(newString^,Line^[edLineNum],positionofoldstring)
      end
   end;

dispose(oldstring); dispose(newstring); dispose(tempcharset);
str(edLinenum:2, UWtemp); UWriteLN(UWtemp+': '+ Line^[edLinenum]);

                   end
                end
         end;
         monitor
      until (Commandst[1] in ['S','A']) or (EUS);
      if CommandST[1] = 'A' then
         Msgabort := true
      else
         Msgabort := false;
      { end module editMsg }

exiteditmsg:
end; { edit msg }
