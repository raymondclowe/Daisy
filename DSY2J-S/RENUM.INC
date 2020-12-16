{ 9/7/88 }

OVERLAY procedure renumbermsgs; {called by callrenumbermsgs}
label exit;
const spin:st4 = '|/-\';

const highestrenumberable = 2000;
type Tabletype = array [ 0..highestrenumberable ] of integer;
     deltabletype = array [ 0..highestrenumberable ] of boolean;

var firstblank,
    firstunpacked,
    i,tv,err,
    numusers : integer;
    Table: ^Tabletype;
    deltable : ^deltabletype;
    msg  : msgheadertype;
    ruser : person;
    fsize, NumMatches, lastmatch : integer;
    temparea : areatype;

(*    c : char;*)

begin

   uwriteln('');
   UWriteln('Renumber');

   uwtemp := namecase( ask('Area : ',14));
   if uwtemp = '' then goto exit;

   reset(areasf);
   fsize := filesize(areasf)-1;

   i := 0; nummatches:=0; lastmatch :=0;
   repeat
      inc(i);
      seek(areasf,i);
      blockread(areasf,temparea,1);

      if (copy(temparea.selector,1,length(uwtemp)) = uwtemp ) and (user.priv >= temparea.accesspriv ) then
      begin
         inc(nummatches); lastmatch :=i
      end;
      monitor
   until (i = fsize) or (lastmatch>0) or (eus);
   if (lastmatch=0) or (nummatches > 1) then
   begin
      uwriteln('No such area '''+uwtemp+'''');
      goto exit;
   end;

   areaNum := i;

   openmsgfiles;

   if highestmsgnum > highestrenumberable then
   begin
      UWriteln('Too many messages to renumber');
      closemsgfiles;
      goto exit
   end;

   if logactivity then writeln(templog,'. Renumber area #'+uwtemp);

   UWrite('Renumbering '+area.selector+': Pass 1  ');

   new(Table);
   new(deltable);

   for i := 0 to highestrenumberable do
   begin
      Table^[ i ] := i;
      deltable^[ i ] := false
   end;

   firstblank := 1;
   seek( msgindex, firstblank );
   if not (eof(msgindex)) then read( msgindex, msg );
(*;WriteLN('read ',firstblank);*)
   while ((msg.fromperson[1] <> 'd') and (not eof(msgindex)) and (firstblank<highestmsgnum)) do
   begin
      uwrite(bs+spin[(firstblank mod 4)+1]);
      firstblank := succ( firstblank );
      seek( msgindex, firstblank );
      read( msgindex, msg );
(*;WriteLN('read ',firstblank)*)
   end;
   tv := firstblank-1;
   if firstblank = highestmsgnum then { there are no deleted msgs }
      firstblank := 0;
   firstunpacked := firstblank;
(*;Writeln('highesetmsgnum ',highestmsgnum);Writeln('firstblank ',firstblank);*)
   if firstblank > 0  then
   begin
      while firstunpacked <= highestmsgnum do
      begin
         uwrite(bs+spin[(firstunpacked mod 4)+1]);
         seek( msgindex, firstunpacked );
         read( msgindex, msg );
(*;WriteLN('read ',firstunpacked);*)
         { if deleted note that in table for user latter }
         if msg.fromperson[1] = 'd' then deltable^[firstunpacked] := true;
         if (msg.fromperson[1] <> 'd') and (firstblank <> 0) then
         begin
            seek( msgindex, firstblank );
            write( msgindex, msg );
(*;WriteLN('MOVING ',FIRSTUNPACKED,' TO ',FIRSTBLANK);*)
            tv := firstblank;
            if firstunpacked = highestmsgnum then
            begin
               highestmsgnum := firstblank;
(*;Writeln('setting highestmsgnum down to ',highestmsgnum)*)
            end;
            firstblank := succ ( firstblank )
         end;
         table^[firstunpacked] := tv;
         firstunpacked := succ( firstunpacked )
(*; REPEAT UNTIL KEYPRESSED ; read(kbd,c);*)
      end;
      for i := firstunpacked to highestrenumberable do
         table^[i] := highestmsgnum;
   end
   else
      for i := highestmsgnum+1 to highestrenumberable do { do this if no moves }
         table^[i] := highestmsgnum;

   nummsgs := highestmsgnum; { num of msgs = highstmsgnum }
   updatemsgstats;

   uwrite(bs+bs+bs+'2  ');
   for i := 1 to highestmsgnum do
   begin
      uwrite(bs+spin[(i mod 4)+1]);
      seek(msgindex,i);
      read(msgindex,msg);
      { if this has a replyto }

(*;writeln(I,': Replyto ',MSG.REPLYTO,': DELTABLE ',deltable^[msg.replyto]);*)

      if msg.replyto > 0 then
      begin
         if deltable^[msg.replyto] then { and original msg is deleted }
            msg.replyto := 0 {set it to not have a reply }
         else
            msg.replyto := Table^[msg.replyto]; {adjust the replyto num}

         seek(msgindex,i);
         write(msgindex,msg)
      end
   end;

   closemsgfiles; {this flushes}

   uwrite(bs+bs+bs+'3  ');

   {$I-}reset( UserFile );{$I+}
   if IOResult <> 0 then
   begin
      UWriteLn(oerr+UserListFN);
      goto exit
   end;

   numusers := filesize( userfile );
   i := 0;
   while i < numusers do
   begin
      uwrite(bs+spin[(i mod 4)+1]);
      seek( userfile, i );
      read( userfile, ruser );

      if ruser.name[1] <> 'd' then { not deleted }
      begin
         ruser.HighestRead[areanum] := Table^[ruser.highestread[areanum]];
         seek( userfile, i );
         write( userfile, ruser );
         if i = usernumber then user.highestread[areanum] := ruser.highestread[areanum];
      end;

      i := succ( i )

   end;

   dispose(Table);
   dispose(deltable);


   close( userfile );
   uwriteln(bs+bs+'. ');
   UWriteln('Done.');

exit:

end; {renumber}