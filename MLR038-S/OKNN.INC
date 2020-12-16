{ 7/1/89 }

function isthisnode(qnet,qnode:integer):boolean;
begin
    isthisnode := (((qnet=thisnet)and(qnode=thisnode))or
                   ((qnet=altnet1)and(qnode=altnode1))or
                   ((qnet=altnet2)and(qnode=altnode2))or
                   ((qnet=altnet3)and(qnode=altnode3))or
                   ((qnet=altnet4)and(qnode=altnode4))
                  );
end;

procedure scanto(qnet,qnode:integer);
begin
   if not ((node.netnumber=qnet) and (node.nodenumber=qnode)) then
   begin { not in the right position, scan to it }
      passtime(500);
      reset(nodelist);
      repeat
         {$i-} read(nodelist,node); {$i+}
         if ioresult <>0 then
         begin
            writeln('Bad Nodelist'); HALT
         end;
      until (eof(nodelist)) or ((node.netnumber=qnet) and (node.nodenumber=qnode));
      close(nodelist);
   end;
   { else
       this is already in the right position }
end;

function okaynetnode(qnet,qnode:integer):boolean;
begin
   scanTo(Qnet,Qnode);
   if ((node.netnumber=qnet) and (node.nodenumber=qnode)) then
      okaynetnode := true
   else
      okaynetnode := false;
end;

function nodephonenumber(qnet,qnode:integer):phonetype;
begin
   scanTo(Qnet,Qnode);
   if ((node.netnumber=qnet) and (node.nodenumber=qnode)) then
      nodephonenumber := node.phonenumber
   else
      HALT;
end;

function nodename(qnet,qnode:integer):nodenametype;
begin
   scanTo(Qnet,Qnode);
   if ((node.netnumber=qnet) and (node.nodenumber=qnode)) then
      nodename := node.name
   else
      HALT;
end;

function nodebaud(qnet,qnode:integer):integer;
begin
   scanTo(Qnet,Qnode);
   if ((node.netnumber=qnet) and (node.nodenumber=qnode)) then
   begin
      if node.baudrate > CFG.maxbaud then
         nodebaud := CFG.maxbaud
      else
         nodebaud := node.baudrate
   end
   else
      HALT;
end;

function privatenode(qnet,qnode:integer):boolean;
begin
   scanTo(Qnet,Qnode);
   if ((node.netnumber=qnet) and (node.nodenumber=qnode)) then
      privatenode := upstring(node.keyword) = 'PVT'
   else
      HALT;
end;

function holdnode(qnet,qnode:integer):boolean;
begin
   scanTo(Qnet,Qnode);
   if ((node.netnumber=qnet) and (node.nodenumber=qnode)) then
      holdnode := (node.hold) or
                  ((upstring(node.keyword) = 'HOL') and         { this is a hold node and }
                   (IsThisNode(node.routetonet,node.routetonode) ) { this is the host }
                  )
   else
      HALT;
end;

procedure readstats;
var df : ^text; i,err : integer;
begin
   if fileexists(generaldrv+':SYSDATA.BBS') then
   begin
      new(df);
      assign(df^,generaldrv+':SYSDATA.BBS');
      reset(df^);
      readln(df^,datestr);
      readln(df^,totalcalls);
      close(df^);
      dispose(df);
   end
   else
      datestr := '01 Jan 80 00:00';

   val(copy(datestr,8,2),todayyear,err);

   TodayMonth := (pos(upstring(copy(datestr,4,3)),'JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC')+2) div 3;

   val(copy(datestr,1,2),todayday,err);

   val(copy(datestr,11,2),NowHour,err);
   val(copy(datestr,14,2),NowMin,err);

   assign(Areasf,areasdrv+':AREAS.BBS');
   reset(areasf);
   i := 0;
   mailarea := 0;
   mailwork := 0;
   numareas := 0;
   while not eof(areasf) do
   begin
      inc(i); inc(numareas);
      seek(areasf,i);
      blockread(areasf,area,1);
      areatag[i] := upstring(area.selector);
      areaattribute[i] := area.attribute;
      if area.attribute = 1 then
         if mailarea = 0 then
            mailarea := i
         else
            if mailwork = 0 then mailwork := i
   end;
   close(areasf);
   if mailarea = 0 then
   begin
      writeln('No mail area!');
      halt
   end
   else
   begin
      writeln('Mail area is #',mailarea);
      if mailwork <> 0 then writeln('Mail work area is #',mailwork);
   end;
end;
