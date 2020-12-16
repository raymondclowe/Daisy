{ 10/8/88 }
procedure killmsgs;
label abort;
var areanumstr : st2;
    err : integer;
    startnum,
    stopnum,
    i : integer;
    yn: char;

procedure getnum(var n:integer; low,High : integer);
var t:st10; per : integer;
begin
   repeat
      repeat
         buflen := 6;
         readln(t);
         val(t,n,err);
         if t[1] = '%' then
         begin
            val(copy(t,2,maxint),per,err);
            if err =0 then
            begin
               n := trunc(highestmsgnum * (per / 100))
            end
         end;
      until (err=0) and (t>'');
      if not ( (n >= low) and (n <= high)) then writeln('Must be ',low,'-',high);
   until ((n >= low) and (n <= high)) or (high=0);
   writeln(n);
end;

begin
   writeln;
   writeln('Kill Messages');
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
      write('Start deleting where? (number=msg number, %n=n percent): ');
      getnum(startnum,1,HighestMsgnum);
      write('Stop deleting where? (number=msg number, %n=n percent): ');
      getnum(stopnum,startnum,HighestMsgnum);
      repeat
         write('Kill only RECEIVED msgs (not all)? ');
         buflen := 1;
         fillchar(tempch,sizeof(tempch),0);
         readln(tempch);
         yn := upcase(tempch[1]);
      until yn in ['Y','N'];
      RcvdOnly := ( yn = 'Y');

      writeln;
      write('Ready to kill ');
      if RcvdOnly then
         write('only received')
      else
         write('all');
      writeln(' messages in ',area.Selector);
      writeln('from ',startnum,' to ',stopnum,'.');
      writeln;

      write('Continue? ');
      buflen := 1;
      fillchar(tempch,sizeof(tempch),0);
      readln(tempch);
      yn := upcase(tempch[1]);

      if nummsgs = 0 then goto abort;
      if yn in ['y','Y'] then
      begin
         killed:=0;
         for i := startnum to stopnum do
            msgkill(i);
         updatemsgstats;
         Writeln('Done.');
         writeln(killed,' messages erased');
         writeln;
      end
      else
         writeln('Aborted');
abort:
      CloseMsgFiles
   end
   else
      writeln('No such area')
end;
