{ 30/6/88 }

procedure renum;
label exit;
var firstblank,
    firstunpacked,
    i,tv,err,
    numusers : integer;

begin

   Writeln('');
   Writeln('Renumber');

   write('Area #: ');
   buflen := 2;
   readln(uwtemp);
   val(uwtemp,areanum,err);

   assign(areasf, areasdrv+':AREAS.BBS');
   reset(areasf);
   if (areanum > filesize(areasf)-1) or (uwtemp='') or (err>0) then
   begin
      Writeln('No area #'+uwtemp);
      goto exit
   end;
   openmsgfiles;

   if highestmsgnum > highestrenumberable then
   begin
      Writeln('Too many messages to renumber');
      closemsgfiles;
      goto exit
   end;

   Write('Pass 1  ');


   for i := 0 to highestrenumberable do
   begin
      Table[ i ] := i;
      deltable[ i ] := false
   end;

   firstblank := 1;
   seek( msgindex, firstblank );
   if not (eof(msgindex)) then read( msgindex, msg );
   while ((msg.fromperson[1] <> 'd') and (not eof(msgindex)) and (firstblank<highestmsgnum)) do
   begin
      Write(bs+spin[(firstblank mod 4)+1]);
      firstblank := succ( firstblank );
      seek( msgindex, firstblank );
      read( msgindex, msg );
   end;
   tv := firstblank-1;
   if firstblank = highestmsgnum then { there are no deleted msgs }
      firstblank := 0;
   firstunpacked := firstblank;
   if firstblank > 0  then
   begin
      while firstunpacked <= highestmsgnum do
      begin
         Write(bs+spin[(firstunpacked mod 4)+1]);
         seek( msgindex, firstunpacked );
         read( msgindex, msg );
         { if deleted note that in table for user latter }
         if msg.fromperson[1] = 'd' then deltable[firstunpacked] := true;
         if (msg.fromperson[1] <> 'd') and (firstblank <> 0) then
         begin
            seek( msgindex, firstblank );
            write( msgindex, msg );
            tv := firstblank;
            if firstunpacked = highestmsgnum then
            begin
               highestmsgnum := firstblank;
            end;
            firstblank := succ ( firstblank )
         end;
         table[firstunpacked] := tv;
         firstunpacked := succ( firstunpacked )
      end;
      for i := firstunpacked to highestrenumberable do
         table[i] := highestmsgnum;
   end
   else
      for i := highestmsgnum+1 to highestrenumberable do { do this if no moves }
         table[i] := highestmsgnum;

   nummsgs := highestmsgnum; { num of msgs = highstmsgnum }
   updatemsgstats;

   Write(cr+'Pass 2  ');
   for i := 1 to highestmsgnum do
   begin
      Write(bs+spin[(i mod 4)+1]);
      seek(msgindex,i);
      read(msgindex,msg);
      { if this has a replyto }

      if msg.replyto > 0 then
      begin
         if deltable[msg.replyto] then { and original msg is deleted }
            msg.replyto := 0 {set it to not have a reply }
         else
            msg.replyto := Table[msg.replyto]; {adjust the replyto num}

         seek(msgindex,i);
         write(msgindex,msg)
      end
   end;

   closemsgfiles; {this flushes}

   pass3;

   Writeln('Done.');
   writeln;

exit:

end; {renumber}