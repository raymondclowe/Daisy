{ 30/6/88 }

Overlay procedure systemstats;
label exit;
var itf, ttf, i, j, used,err : integer; stri:st4;
    fsize, nummatches, lastmatch : integer;
    temparea : areatype;
begin
   uclrscr;
   uwriteln('');
   uwriteln('System Statistics');

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
str(areaNum,stri);
uwriteln('Message area (#'+stri+') ' +area.selector);

itf := filesize(msgindex) -1; str(itf,uwtemp);
Uwriteln('Index file:-');
uwriteln('');
uwriteln(' Total file size '+uwtemp+' blocks');

str(highestmsgnum,uwtemp);
uwriteln(' Highest used record : '+uwtemp);

str(itf-highestmsgnum,uwtemp);
uwriteln(' Free (to be used) records : '+uwtemp);

str(highestmsgnum-nummsgs,uwtemp);
uwriteln(' Deleted records (would be made available by renumber) : '+uwtemp);

ttf := filesize(msgtext);
str(ttf,uwtemp);
uwriteln('');
uwriteln('Text file:-');
uwriteln('');
uwriteln(' Total file size '+uwtemp+' blocks');

used := 0;
for i := 0 to ((ttf+1) div 8) do
begin
   for j := 0 to 7 do
      if odd(blockmap[i] shr j) then used := succ(used)
end;

str((Ttf-used),uwtemp);
uwrite(' Un-used : '+uwtemp+' blocks (');

str((((ttf-used)/ttf)*100):5:2,uwtemp);
uwriteln(uwtemp+'%)');

   closemsgfiles;

   if recordnumcalls then
   begin
      uwriteln('');
      str(totalcalls,uwtemp);
      uwriteln('Total number of received calls ('+uwtemp+')')
   end;

exit:
end;
