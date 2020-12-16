{ 9/9/88 }

overlay procedure InitPackets(speak:boolean);
var blk : array [1..128] of byte; pkt : packetheadertype absolute blk;
    i : integer; stri, stri2:st5;
    f : file;
    out,hut : boolean;
Var NowTimestr : st4;
    nowtime : integer;
    err : integer;
    openingtime, closingtime : integer;


begin

   NumOutboundPkts := 0;
   PktsYetToSend := 0;

   i := 1; str(i,stri);
{   writeln(i);}
   out := fileExists(CFG.WorkDrv+':'+stri+'.OUT');
   Hut := fileExists(CFG.WorkDrv+':'+stri+'.HUT');
(*          xor ?    ******************************************)
   while out or hut do
   begin
      passtime(500);

      { gives HUT priority if both exist (!) }
      if out then assign(f,CFG.WorkDrv+':'+stri+'.OUT');
      if hut then assign(f,CFG.WorkDrv+':'+stri+'.HUT');

      reset(f);
      {$I-} blockread(f,blk,1); {$I+}
      if ioresult <> 0 then
      begin
         report('Error: '+CFG.WorkDrv+':'+stri+'.?UT',4);
         fillchar(pkt,sizeof(pkt),0);
      end;

      if Okaynetnode(pkt.destnet,pkt.destnode) then
      begin
         inc( NumOutboundPkts );
         calls[NumOutboundPkts].packetnum := i;
         calls[NumOutboundPkts].destnet := pkt.destnet;
         calls[NumOutboundPkts].destnode := pkt.destnode;
         calls[NumOutboundPkts].sent := false;
         calls[NumOutboundPkts].hold := Hut;
         calls[NumOutboundPkts].connects := 0;
         calls[NumOutboundPkts].trytosend := true;

         scanto(pkt.destnet,pkt.destnode);
         NowTimeStr := copy(datestr,11,2) + copy(datestr,14,2);
         val(copy(Nowtimestr,1,3),nowtime,err);
         if (nowTimeStr <> '0000') and
            (*(nowTimeStr <> '2359') and*)
            (node.opentime <> 255) and
            (err=0) then
         begin
            OpeningTime := node.OpenTime;
            ClosingTime := node.CloseTime;
{writeln(Openingtime:6,nowtime:6,closingtime:6);}
            if ClosingTime < Openingtime then Closingtime := closingtime + 236;
            if Nowtime < Openingtime then NowTime := Nowtime + 236;
{writeln(Openingtime:6,nowtime:6,closingtime:6);}
            if (OpeningTime <= Nowtime) and (Nowtime <= closingtime) then
{               writeln('Right time of day')}
            else
            begin
               calls[NumOutboundPkts].trytosend := false;
{               writeln('Wrong time of day');}
            end
         end;

         if speak then
         begin
            str(pkt.destnet,stri); str(pkt.destnode,stri2);
            if hut then
               report('Hold packet for '+stri+'/'+stri2,2)
            else
               report('Outgoing packet for '+stri+'/'+stri2,2);
         end;

         if (not hut) and
            (calls[NumOutboundPkts].trytosend) then inc( PktsYetToSend );

      end
      else
      begin
         str(pkt.destnet,stri); str(pkt.destnode,stri2);
         report('There is no node '+stri+'/'+stri2,1)
      end;
      i := i + 1; str(i,stri);
      out := fileExists(CFG.WorkDrv+':'+stri+'.OUT');
      Hut := fileExists(CFG.WorkDrv+':'+stri+'.HUT');

   end;
end;


(**********************************************************************)
(*                                                                    *)
(* This takes any unsent packets and renames them so they are numberd *)
(* consequtivly from 1.out to ensure that it can find them again.     *)
(*                                                                    *)
(* Handles OUT and HUT files as well as attached FIL lists            *)
(*                                                                    *)
(**********************************************************************)
overlay procedure renamePackets;
var f:file;
    i,j:integer;
    stri,strj:st5;
    out, hut : boolean;
begin
   j := 1;
   if numoutboundpkts > 32765 then halt;  { that's gotta be a mistake }
   for i := 1 to NumOutboundPkts do
   begin
      str(i,stri);
      out := fileExists(CFG.WorkDrv+':'+stri+'.OUT');
      Hut := fileExists(CFG.WorkDrv+':'+stri+'.HUT');
      if out or hut then
      begin
         passtime(500);
         if i<>j then
         begin
            if out then
               assign(f,CFG.WorkDrv+':'+stri+'.OUT')
            else
               assign(f,CFG.WorkDrv+':'+stri+'.HUT');
            str(j,strj);
            show('Renaming '+CFG.WorkDrv+':'+stri+'.?UT to '+CFG.WorkDrv+':'+strj+'.?UT',3);
            if out then
               rename(f,CFG.WorkDrv+':'+strj+'.OUT')
            else
               rename(f,CFG.WorkDrv+':'+strj+'.HUT');
            if fileexists(CFG.WorkDrv+':'+stri+'.FIL') then
            begin
               assign(f,CFG.WorkDrv+':'+stri+'.FIL');
               rename(f,CFG.WorkDrv+':'+strj+'.FIL');
            end
         end;
         inc(j)
      end
   end;
end; { rename }
