{ 7/1/89 } { Raymond Lowe 1987,88,89 }
overlay procedure unpack;

label CantOpenPkt, badpkt, nulpkt, nopkts;

var fs, i,err : integer;
    ch: char;
    packet: packetheadertype;
    pktnum:integer;
    pktnumstr:st5;
    stri,stri2 : st5;
    ErrWhileUnpacking : boolean;

    msg : messageheadertype;

    TestF : file;
    mailertype : st20;

    nummsgs : integer;

var PktF:file;
    blockbuf : st128;

procedure pktread(var ch:char);
begin
   if blockbuf = '' then
   begin
      {$I-} blockread(PktF,blockbuf[1],1); {$I+}
      if ioresult <> 0 then
      begin
         report('Error reading pkt',3);
         fillchar(blockbuf,sizeof(blockbuf),0)
      end;
      blockbuf[0] := chr(128)
   end;
   ch := blockbuf[1];
   delete(blockbuf,1,1)
end;



procedure message;
var i:integer;
    datetime : st40;
    origNet,origNode,
    destNet, destNode : integer;
    ch:char;
    tossto,shouldarea : integer;
    tag:st14;
    okaychars : set of char;
    thisline: st128;
    thislinelen : byte absolute thisline;

procedure nulread(var l:st73);
begin
   l := '';
   pktread(ch);
   while Ch <> ^@ do
   begin
      l := l+ch;
      pktread(ch)
   end;
end;

begin  { message }

   NewMsgHeader := nulmsgheader;

   for i := 3 to 14 do
   begin
      pktread(ch);
      mem[addr(msg)+i-1] := ord(ch)
   end;

   {$v-}
   nulread(datetime);
   nulread(NewMsgHeader.toperson);
   nulread(NewMsgHeader.fromperson);
   nulread(NewMsgHeader.subj);
   {$v+}
   { convert to Daisy-allowed empty to/subj fields from ' ' }
   if NewMsgHeader.toperson = ' ' then NewMsgHeader.toperson := '';
   if NewMsgHeader.subj = ' ' then NewMsgHeader.subj := '';

   if datetime[1] = ' ' then datetime[1] := '0';
   if datetime[2] = ' ' then datetime := '0'+datetime;
   datetime := copy(datetime,1,pos(' ',copy(datetime,2,7))+7) +
               copy(datetime,pos(':',datetime)-3,6);

   move(datetime[1],NewMsgHeader.dateentered[1],15);

   newmsgheader.attribute := msg.attribute;

   NewMsgHeader.orignet := msg.orignet;
   NewMsgHeader.destnet := msg.destnet;
   NewMsgHeader.orignode := msg.orignode;
   NewMsgHeader.destnode := msg.destnode;

   shouldarea := mailarea;           { basic default }

   if not isthisnode(newmsgheader.destnet,newmsgheader.destnode) then
   begin
      { I don't want to forward any fileattaches thank you! }
      NewMsgHeader.attribute := NewMsgHeader.attribute - [FileAttached];
      NewMsgHeader.attribute := NewMsgHeader.attribute + [InTransit];
      if mailwork <> 0 then shouldarea := mailwork
   end;

   { intransit messages may have any characters in the string except nul}
   if InTransit in NewMsgHeader.attribute then
      okaychars := [#01..#255]
   else
      okaychars := [^A,CR,' '..'~'];

(*   new(line);*)
   fillchar(line,sizeof(line),0);
   fillchar(thisline,sizeof(thisline),0);

   Linenum := 1;
   repeat
      pktread(ch);
      if ch in okaychars then thisline := thisline + ch;
      if (thislinelen = 127) or (ch = #00) then
      begin
         if linenum < 46 then
         begin
            line[linenum] := thisline;
            thislinelen := 0;
            linenum := linenum+1
         end
         else
         begin
            report('Message is too long, shortened',3);
            line[linenum] := ^M+^M+' * MAILER ERROR: Message was too long';
            ch := #00
         end
      end;
   until ch=#00;
   passtime(1000);

   tossto := 0;
   if (CFG.DoTossMail) and { can toss }
      (isthisnode(newmsgheader.destnet,newmsgheader.destnode)) then { addressed to here }
   begin
      if copy(line[1],1,5) = 'AREA:' then
      begin
         tag := upstring(copy(line[1],6,pos(cr,line[1])-6));
         for i := 1 to numareas do
            if copy(tag,1,14) = areatag[i] then tossto := i
      end
   end;

   if tossto > 0 then   { if tossable set switch to the correct area }
   begin
      line[1] := copy(line[1],pos(cr,line[1])+1,maxint); { remove tag }
      shouldarea := tossto;
   end;

   if shouldarea <> areanum then
   begin
      closemsgfiles;
{      writeln('Switching to area #',shouldarea);}
      areanum := shouldarea;
      openmsgfiles;
      passtime(1000);
   end;

   writeln('Saving to area #',areanum);

   if (line[linenum] <> '') and (linenum < 46) then
      linenum := linenum + 1;
   passtime(1000);
   savemsg;
   if savemsgerr > 0 then
   begin
      report('Error saving message, this mail packet preserved!',3);
      ErrWhileUnpacking := true;
   end;

(*   dispose(line)*)

end; { message }

begin
   ModOffHook;
   diskreset;

   rawdir(cfg.workdrv,'????????PKT','');
   if filecounter = 0 then goto nopkts;

   areanum := mailarea;
   openmsgfiles;

   for pktnum := 1 to filecounter do
   begin
      writeln('Opening '+filename[pktnum]);
      nummsgs := 0;

      assign(TestF,CFG.WorkDrv+':'+filename[pktnum]);
      {$i-} reset(TestF); {$i+}
      ioerr := ioresult;
      if ioerr = 0 then
      begin
         fs := filesize(TestF);
         close(TestF);
         if fs = 0 then
         begin
            show('Null packet',1);
            goto NulPkt
         end;
      end
      else
         goto CantOpenPkt;

      assign(PktF,CFG.WorkDrv+':'+filename[pktnum]);
      reset(PktF);
      blockbuf := '';

      for i := 1 to 58 do
      begin
         pktread(ch);
         mem[addr(packet)+i-1] := ord(ch)
      end;

(*      writeln('Packet:');
      writeln('from ',packet.orignet,'/',packet.orignode);
      writeln('to   ',packet.destnet,'/',packet.destNode);
      writeln('date ',packet.day,' ',packet.month,' ',packet.year);
      writeln('time ',packet.hour:2,':',packet.minute:2,':',packet.second:2);
      writeln('baud ',packet.baud);
      writeln('type ',packet.pkttype);
      writeln('productcode ', packet.productcode);
  *)
      if packet.pkttype <> 2 then
      begin
         str(msg.messagetype,stri);
         report('Unknown packet type '+stri,3);
         ErrWhileUnpacking := true;
         goto badpkt;
      end;

      case packet.productcode of
         0 : mailertype := 'Fido';
         1 : mailertype := 'spark';
         5 : mailertype := 'oMMM';
         6 : mailertype := 'Dutchie';
         27 : mailertype := 'BinkleyTerm';
         $1e: mailertype := 'Daisy MAILER'
       else
          str(packet.productcode,mailertype)
      end;
      str(packet.orignet,stri); str(packet.orignode,stri2);

      for i := 1 to 2 do
      begin
         pktread(ch);
         mem[addr(msg)+i-1] := ord(ch)
      end;

      { Don't report unless there is at least a single real msgs in it }
      if msg.messagetype = 2 then
         report('Un-packing '+mailertype+' packet from '+stri+'/'+stri2,2);

      if not (msg.messagetype in [2,0]) then
      begin
         str(msg.messagetype,stri);
         report('Unknown message type '+stri,3)
      end;

      ErrWhileUnpacking := false;
      while (msg.messagetype = 2) and (not ErrWhileUnpacking) do
      begin
         inc(nummsgs);
         message;

         for i := 1 to 2 do
         begin
            pktread(ch);
            mem[addr(msg)+i-1] := ord(ch)
         end
      end;

badpkt:
      {pktclose;  erases file!!! if no errorsaving }
      close(PktF);
CantOpenPkt:
nulpkt:
      if (not CFG.preservepackets) and
         (not ErrWhileUnpacking) then
      begin
         {$I-} erase(PktF); {$I+}
         ioerr := ioresult;           { ignore this - don't really caure }
      end;

      if nummsgs > 0 then
      begin
         str(nummsgs,stri);
         Report(stri+' messages',2)
      end;

   end;
   closemsgfiles;

nopkts:

end; { unpack }
