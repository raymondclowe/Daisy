{Copyright (C) 10/8/1988 by Raymond Lowe}

procedure clearsent;
label abort;
var areanumstr : st2;
    setstr : st4;
    setn,
    err,
    startnum,
    stopnum,
    i : integer;
    yn: char;
    msg : msgheadertype;

begin
   writeln;
   writeln('Set high water mark');
   writeln;
   write('Area #: ');
   buflen := 2;
   readln(areanumstr);
   val(areanumstr,areanum,err);
   if (err = 0) and (areanumstr>'') then
   begin
      OpenMsgFiles; { areanum }
      Writeln;
      Writeln('This is area ',area.selector,'.');
      writeln;
      writeln('The highest msg in this area is number ',HighestMsgnum,',');
      writeln('there are ',nummsgs,' messages in this area');

      writeln;

      write('Continue? ');
      buflen := 1;
      fillchar(tempch,sizeof(tempch),0);
      readln(tempch);
      yn := upcase(tempch[1]);

      if yn in ['y','Y'] then
      begin
         write('Set the high water mark to? ');
         buflen := 3;
         readln(setstr);
         val(setstr,setn,i);
         if (i <> 0) or (setn < 1) or (setn > HighestMsgnum) then goto abort;

         killed:=0;
         i:=1;
         repeat
            seek(msgindex,i);
            read(msgindex,msg);
            if i <= setn then
               msg.attribute := msg.attribute + [sent]
            else
               msg.attribute := msg.attribute - [sent];

            seek(msgindex,i);
            write(msgindex,msg);
            i:=i+1;
         until eof(msgindex);
         updatemsgstats;
         Writeln('Done.');
         writeln;
      end
      else
abort:
         writeln('Aborted');
      CloseMsgFiles
   end
   else
      writeln('No such area')
end;
