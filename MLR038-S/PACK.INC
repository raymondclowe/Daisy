{ 23/12/88 }

{ Raymond Lowe 1987 }

overlay procedure pack;

var i,scanner : integer;
    msgheader : msgheadertype;
    packet: packetheadertype;

var ch : char;
    BossNetNode : st20;
    code:integer;
    TI:INTEGER;
    PktMsgsNum : integer;

var appending : boolean;

    f2,f:text;

    originstr : st80;
    addressStr,FullAddressstr : st40;


var blockbuf : st128;
    blockbuflen : byte absolute blockbuf;
var BuffF:file;
var pktnum : integer;
procedure pktwrite(l:st255);
begin
   while l > '' do
   begin
      if blockbuflen=128 then
      begin
         blockwrite(BuffF,blockbuf[1],1);
         blockbuf := ''
      end;
      blockbuf := blockbuf + l[1];
      delete(l,1,1)
   end
end;

                 { Net/Node this packet should be addressed do}
procedure pktopen(toNet,toNode:integer);
var bl,i,match, fp : integer; stri :st20;
    FB : array [1..128] of byte;
    pkt: packetheadertype absolute FB;
    ext, ext2: st4;
    out, hut : boolean;
    shouldhold : boolean;

begin
   shouldhold := holdNode(toNet,toNode);
   if shouldhold then ext := '.HUT' else ext := '.OUT';

   appending := false;

   { look for a packet with this address to append to }
   i := 1;
   str(i,stri);
   pktnum := i;
   match := 0;

   out := fileexists(CFG.WorkDrv+':'+stri+'.OUT');
   Hut := fileexists(CFG.WorkDrv+':'+stri+'.HUT');

   while (out or hut) and (match=0) do
   begin

      if HUT then ext2 := '.HUT' else ext2 := '.OUT';

      assign(BuffF,CFG.WorkDrv+':'+stri+ext2);
      reset(BuffF);
      if filesize(BuffF) > 0 then
      begin
         blockread(BuffF,FB,1);
(*         writeln('packet ',i,' is for ',pkt.destNet,'/',pkt.destNode);*)
         if (pkt.destNet = toNet) and (pkt.destNode = toNode) then match := i;
      end;
      i:=i+1;
      str(i,stri);
      pktnum := i;
      out := fileexists(CFG.WorkDrv+':'+stri+'.OUT');
      Hut := fileexists(CFG.WorkDrv+':'+stri+'.HUT')
   end;

   if match = 0 then
   begin { generate new packet }
      pktnum := i; { no .out or .hut of this number }
      i := 1;
      str(i,stri);
      while fileexists(CFG.WorkDrv+':'+stri+'.OUT') or fileexists(CFG.WorkDrv+':'+stri+'.HUT') do
      begin
         i:=i+1;
         str(i,stri)
      end;
      pktnum := i;
      assign(BuffF,CFG.WorkDrv+':'+stri+ext);
      rewrite(BuffF);
      blockbuf := '';

      packet.origNode := thisNode;
      packet.destNode := toNode;
      packet.year := todayyear + 1900;
      packet.month := todaymonth;
      packet.day := todayday;
      packet.hour := NowHour;
      packet.minute := NowMin;
      packet.second := 0;
      packet.baud := 0;
      packet.pkttype := 2;
      packet.origNet := thisNet;
      packet.destNet := toNet;
      packet.productcode := mailerproductcode;
      fillchar(packet.fill,33,0);

      for i := 1 to 58 do
         pktwrite(chr(mem[addr(packet)+i-1]));

      PktMsgsNum := 0
   end
   else
   begin { set up to append }
      pktNum := match; { this one was the correct n/n }
      str(pktnum,stri);
      appending := true;
(*      if ext <> ext2 then
      begin
         report('Renaming Pkt to '+CFG.WorkDrv+':'+stri+ext,3);
         rename(BuffF,CFG.WorkDrv+':'+stri+ext); { make sure it is the correct
                                           kind of pkt }
      end;*)

{      report('Appending to existing packet');}
      fp := filesize(BuffF)-1;
      seek(BuffF,fp); { move to lastblock }
      blockread(BuffF,blockbuf[1],1); blockbuf[0]:=#128; { read in }
      while blockbuf[blockbuflen] = ^Z do
         blockbuflen := pred(blockbuflen); { strip away fill }
         { set up buffer with content of last block }

      { now get rid of 00 00 end of packet mark }
      case blockbuflen of
         2..255 : blockbuflen := blockbuflen - 2;
         0,1    : begin
                     fp := fp-1;
                     seek(BuffF,fp);
                     blockread(BuffF,blockbuf[1],1); { read in }
                  end;
      end;
      if blockbuflen = 0 then blockbuflen:=126;
      if blockbuflen = 1 then blockbuflen:=127;

      seek(BuffF,fp); { position pointer ready to write here }
      pktmsgsnum := 0; {not true!}
   end;
end;

procedure pktclose(okay:boolean);
begin
   pktwrite(nul);pktwrite(nul);     { nul message to end packet }

   while blockbuflen<128 do
      blockbuf := blockbuf + ^Z;
   blockwrite(BuffF,blockbuf[1],1);

   close(BuffF);
   if not okay then erase(BuffF)
end;

var viaNet, viaNode : integer;
    routed,defaultroute : boolean;

procedure packmessage(msgnum:integer);

var datetime : st20;
    code :integer;
    ch:char;
    msg: messageheadertype;  { packed }
    msgheader:msgheadertype; { daisy }
    i : integer;
    exitf:boolean;
    buff:st128;
    textblock : integer;
    stri,stri2,stri3,stri4,stri5:st5;
    secondsstr : st2;
begin

   seek(msgindex,msgnum);
   read(msgindex,msgheader);
   passtime(500);

   str(random(60),secondsstr);
   if length(secondsstr) = 1 then secondsstr := '0' + secondsstr;

   datetime := copy(msgheader.dateentered, 1, 10)+' '+copy(msgheader.dateentered, 11, 5)+':'+secondsstr+#00;
   for i := 1 to 8 do
      if datetime[i] = '-' then datetime[i] := ' ';

                                          { zero these bits }
   msg.attribute := msgheader.attribute - [Sent, InTransit, Orphan, Local,
                                           HoldForPickup];
   if area.attribute = 2 then msg.attribute := msgheader.attribute + [killSent];
   msg.destNet := msgheader.destNet;
   msg.origNet := msgheader.origNet;
   msg.destNode := msgheader.destNode;
   msg.origNode := msgheader.origNode;

   { handle echo area }
   if area.attribute = 2 then
   begin
      msg.destNet := area.forwardNet;
      msg.destNode := area.forwardNode;
   end;

   str(msgnum,stri);
   str(msg.destNet,stri2);str(msg.destNode,stri3);
   if routed then
   begin
      str(viaNet,stri4);str(viaNode,stri5);
      report('Msg #'+stri+' to '+stri2+'/'+stri3+' via '+stri4+'/'+stri5,1);
   end;
   passtime(2000);

   msg.messagetype := 2;
   msg.cost := 0;

   for i := 1 to 14 do      { first 14 bytes taken directly from var. }
      pktwrite(chr(mem[addr(msg)+i-1]));

   pktwrite(datetime); { datetime - taken from msgheader, already nul terminated }
   if (msgheader.toperson = 'Sysop') and isthisnode(msgheader.orignet,msgheader.orignode) then
      msgheader.toperson := msgheader.toperson + ' of ' + addressstr;
   if (msgheader.fromperson = 'Sysop') and isthisnode(msgheader.orignet,msgheader.orignode) then
      msgheader.fromperson := msgheader.fromperson + ' of ' + addressstr;
   if msgheader.toperson = '' then pktwrite(' '); { make '' a single ' '}
   pktwrite(msgheader.toperson+nul);    { touserName }
   pktwrite(msgheader.fromperson+nul);    { origuserName }
   if msgheader.subj = '' then pktwrite(' '); { make '' a single ' '}
   pktwrite(msgheader.subj+nul);

   { handle echo area }
   if area.attribute = 2 then
      pktwrite('AREA:'+upstring(areatag[AreaNum])+CR);

   exitf := false;
   buff := '';
   if msgheader.pointertobody > 0 then
   begin
      seek(msgtext,msgheader.pointertobody);
      repeat
         blockread(msgtext,buff[1],1);
         passtime(100);
         buff[0] := #128;
         if pos(#00,buff)>0 then
         begin
            exitf := true;
            buff[0] := chr(pos(#00,buff)-1)
         end;
         pktwrite(buff);
      until exitf
   end;

   { handle echo area }
   if area.attribute = 2 then
   begin
      pktwrite(CR+'---- '+copy(version,1,11)+CR);
      pktwrite(' * Origin: '+originstr+' ('+FullAddressStr+')'+CR);
      pktwrite('SEEN-BY: '+AddressStr+CR);
      pktwrite(^A+'PATH: '+AddressStr+CR);
   end;

   pktwrite(nul);

   { mark message as sent } {unmark it if packet not sent?}
   seek(msgindex,msgnum);
   read(msgindex,msgheader);
   msgheader.attribute := msgheader.attribute + [sent];
   seek(msgindex,msgnum);
   write(msgindex,msgheader);
   passtime(1000);

   if (InTransit in msgheader.attribute) or
      (KillSent in msgheader.attribute) then msgkill(msgnum);

end;


procedure packanarea(areatopack:integer);
label skippack;
var currentNet, currentNode : integer;
    stri,stri2,stri3,stri4,stri5 : st5;
    originfn :st14;
    okdestbbs : boolean;
    p:integer;
    l:st70;
begin

   writeln('Packing area '+areatag[areatopack]+' (#',areatopack,')');

   if interactive then
   begin
      passtime(2000);
      write('Address packets to (default ',CFG.BossNet,'/',CFG.BossNode,')? ');
      buflen := 11;
      readln(BossNetNode);
      writeln;

      CFG.BossNet:=thisNet;
(*      CFG.BossNode:= -1;*)
      if bossnetnode > '' then
      begin
         p:=pos('/',BossNetNode);
         val(copy(BossNetNode,p+1,maxint),CFG.BossNode,code);
         if p >0 then
         begin
            delete(BossNetNode,p,maxint);
            val(BossNetNode,CFG.BossNet,code)
         end
      end;
   end;

   if (CFG.BossNet=0) and (CFG.BossNode=0) then
      routed := true
   else
      routed := false;
   if routed and fileexists(CFG.NodeLstDrv+':ROUTE.BBS') then
      defaultroute := false
   else
      defaultroute := true;


   if routed then
      report('Routed',0)
   else
   begin
      str(CFG.BossNet,stri); str(CFG.BossNode,stri2);
      report('Via '+stri+'/'+stri2,0);
   end;

   str(thisNet,uwtemp);
   AddressStr := uwtemp;
   str(thisNode,uwtemp);
   AddressStr := addressStr+'/'+uwtemp;
{   str(CFG.OurZone,uwtemp);
   FullAddressStr := uwtemp+':'+AddressStr;
   str(CFG.Ourpoint,uwtemp);
   FullAddressStr := FullAddressStr+'.'+uwtemp;}
   FullAddressStr := AddressStr;

   { read in a file called ORIGIN.xx or ORIGIN.BBS, where xx is the
     area number }
   str(areatopack,originfn);
   originfn := 'ORIGIN.'+originfn;
   if not fileexists(originfn) then originfn := 'ORIGIN.BBS';
   if fileexists(originfn) then
   begin
      assign(f,originfn);
      reset(f);
      readln(F,originstr);
      close(f);
   end
   else
      originstr := 'Daisy';

{   if routed and defaultroute then report('Default routing');}

   currentNet := 0;
   currentNode := 0;

   if not routed then pktopen(CFG.BossNet,CFG.BossNode);

   areanum := areatopack;
   openmsgfiles;

   i := 1;

   if area.attribute = 2 then
   begin
      i:=highestmsgnum+1;
      repeat
         dec(i);
         seek(msgindex,i);
         read(msgindex,msgheader);
         passtime(500);
      until (i<=1) or (sent in msgheader.attribute);
   end;
   show('',2);
   writeln('Starting at msg #',i);
   while (i >= 1) and (i <= highestmsgnum) do
   begin
      seek(msgindex,i);
      read(msgindex,msgheader);
      passtime(500);

      { handle echo area }
      if area.attribute = 2 then
      begin
         if not isthisnode(msgheader.destNet,msgheader.destNode) then
         begin { This IS one of ours (it's not been sent here) }
            msgheader.destNet := area.forwardNet; { direct it }
            msgheader.destNode := area.forwardNode; { to forward }
         end
         else
         begin { it's NOT one of ours }
            if not (sent in msgheader.attribute) then { not scanned before }
            begin { but we've looked at it now, so mark it }
               msgheader.attribute := msgheader.attribute + [sent];
               seek(msgindex,i);
               write(msgindex,msgheader);
            end
         end
      end;

      if (not (msgheader.fromperson[1] = 'd')) and               { not deleted }
         (okayNetNode(msgheader.destNet,msgheader.destNode)) and { listed Node}
         (not isthisNode(msgheader.destNet,msgheader.destNode)) and{ not this Node }
         (not (sent in msgheader.attribute)) then                { not already sent }
      begin
         scanto(msgheader.destNet,msgheader.destNode); { addressee }
         if routed then { what if not default route !!!!!!!!!!!!!! }
         begin
            viaNet := Node.routetoNet;     { where I would send the pkt to }
            viaNode := Node.routetoNode;

               { if it's trying to send direct }
            if ((viaNet=Node.Netnumber) and (viaNode=Node.Nodenumber)) and

               { to a private or hold hode }
               ((Node.keyword = 'PVT') or (Node.keyword = 'HOL')) and

               { and I'm not the host }
               (not isthisNode(Node.Netnumber,0)) then

            begin { forcibly send it via the host of it's Net}
               viaNet := Node.Netnumber;
               viaNode := 0
            end;

            { fileattached }
            if fileattached in msgheader.attribute then
            begin
               if (Node.keyword='PVT') or (Node.keyword='HOL') then
                  goto skippack;
               viaNet := msgheader.destNet; {send direct}
               viaNode := msgheader.destNode;
            end;

            pktopen(viaNet,viaNode);
{writeln('Pktnum = ',pktnum);}
            packmessage(i);
            pktclose(true); { keep packet }

            if fileattached in msgheader.attribute then
            begin
               msgheader.subj := upstring(msgheader.subj);
               str(pktnum,uwtemp);
               if fileExists(CFG.WorkDrv+':'+uwtemp+'.FIL') then
               begin
                  assign(f2,CFG.WorkDrv+':'+uwtemp+'.FIL');
                  rename(f2,CFG.WorkDrv+':'+uwtemp+'.$$$');
                  reset(f2);
                  assign(f,CFG.WorkDrv+':'+uwtemp+'.FIL');
                  rewrite(f);

                  while not eof(f2) do
                  begin
                     readln(f2,l);
                     if l <> msgheader.subj then { avoid duplicate }
                        writeln(f,l);
                  end;
                  close(f2);
                  erase(f2);
               end
               else
               begin
                  assign(f,CFG.WorkDrv+':'+uwtemp+'.FIL');
                  rewrite(f);
               end;

               writeln(f,msgheader.subj);
               close(f);
            end;
skippack:
         end
         else
         begin
            packmessage(i);
            inc(PktMsgsNum)
         end
      end;
      inc(i)
   end;
   closemsgfiles;

   if not routed then
   begin
      str(pktmsgsnum,uwtemp);
      if appending then
         report('There are '+uwtemp+' more messages in this packet',1)
      else
         report('There are '+uwtemp+' messages in this packet',1);

      if CFG.PollAble or (pktmsgsnum > 0) or (appending) then
         pktclose(true)
      else
         pktclose(false)
   end

end; { packanarea }

begin
   ModOffHook;
   for scanner := 1 to numareas do
   begin
      if (areaattribute[scanner] = 1) or
         ((areaattribute[scanner] = 2) and (CFG.DoScanMail))
         then PackAnArea(scanner);
   end;
end;

{ 23/11/87 }

overlay procedure makePoll(now:boolean);
var Pollblock : array [ 1..128 ] of byte;
    PollPacket : Packetheadertype absolute pollblock;
    f : file;
    stri,stri2 : st5;
    bossNetNode : st14;
    Code,
    p,pollnum:integer;
    pollnumstr:st5;
begin
   if now then
      pollnum := 32767
   else
      pollnum := NumOutboundpkts+1;
   str(pollnum,pollnumstr);

   if interactive or (not now) then
   begin
      passtime(5000);
      write('Poll from (default ',CFG.BossNet,'/',CFG.BossNode,')? ');
      buflen := 11;
      readln(BossNetNode);
      writeln;

      if bossnetnode > '' then
      begin
         CFG.BossNet:=thisNet;
         p:=pos('/',BossNetNode);
         val(copy(BossNetNode,p+1,maxint),CFG.BossNode,code);
         if p >0 then
         begin
            delete(BossNetNode,p,maxint);
            val(BossNetNode,CFG.BossNet,code)
         end
      end;
   end;

   if okaynetnode(CFG.BossNet,CFG.BossNode) then
   begin
    writeln('Packet addressed to ',CFG.BossNet,'/',CFG.BossNode);

    assign(f,CFG.workdrv+':'+pollnumstr+'.OUT');
    rewrite(f);

    fillchar(Pollblock,sizeof(pollblock),0);

    PollPacket.origNode := thisNode;
    PollPacket.destNode := CFG.BossNode;
    PollPacket.year := todayyear + 1900;
    PollPacket.month := todaymonth;
    PollPacket.day := todayday;
    PollPacket.hour := NowHour; PollPacket.minute := NowMin; PollPacket.second := 0;
    PollPacket.baud := 0;
    PollPacket.pkttype := 2;
    PollPacket.origNet := thisNet;
    PollPacket.destNet := CFG.BossNet;
    PollPacket.productCode := mailerproductcode;

    blockwrite(f,Pollblock,1);
    close(f);

    if now then
    begin
       NumOutboundpkts := 1;
       PktsYetToSend := 1
    end
    else
    begin
       inc(NumOutboundpkts);
       inc(PktsYetToSend)
    end;
    calls[NumOutboundpkts].Packetnum := pollnum;
    calls[NumOutboundpkts].destNet := PollPacket.destNet;
    calls[NumOutboundpkts].destNode := PollPacket.destNode;
    calls[NumOutboundpkts].sent := false;
    calls[NumOutboundpkts].hold := false;
    calls[NumOutboundpkts].connects := 0;
    calls[NumOutboundpkts].trytoSend := true;

    str(PollPacket.destNet,stri); str(PollPacket.destNode,stri2);
    report('Poll Packet for '+stri+'/'+stri2,0);
   end
   else
      report('No such node!',0)
end;
