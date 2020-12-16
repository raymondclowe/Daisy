{ 11/10/88 }

{ note this contains both changeareas (edit area file) and
  changearea (move from one area to another) }

OVERLAY procedure changeareas;

var command:st1;

    temparea : areatype;
    temp : st40;
    temp2: st4;
    tempn,
    numareas,
    i,d,err : integer;
    tempcharset : set of char;
    ok : boolean;

{$w1}
procedure showarea(i:integer);
var stri:st4;
begin
   seek(areasf,i);
   blockread(areasf,Temparea,1);
   str(i:2,uwtemp);
   str(Temparea.accesspriv:2,temp);
   with Temparea do
   begin
      case attribute of
       0 : uwrite('  ');
       1 : uwrite('* ');
       2 : uwrite('E ')
      end;
      uwriteln(uwtemp+': '+selector+' : '+name+' ('+indexdrive+') ('+
               textdrive+') Priv = '+temp)
   end
end;
{$w0}

procedure EditPriv(var priv:byte; prompt:st128);
var temp, temp2 :st4;
    tempn, err :integer;
begin
   repeat
      str(priv,temp);
      temp2 := ask(prompt+' ('+temp+'): ',3);
      if temp2 = '' then temp2 := temp;
      val(temp2,tempn,err);
      if not (tempn in [1..100]) then err := 1
   until (err=0) or (eus);
   if err = 0 then priv := tempn;
end;
procedure EditInteger(var priv:integer; prompt:st128);
var temp, temp2 :st4;
    tempn, err :integer;
begin
   repeat
      str(priv,temp);
      temp2 := ask(prompt+' ('+temp+'): ',3);
      if temp2 = '' then temp2 := temp;
      val(temp2,tempn,err);
   until (err=0) or (eus);
   if err = 0 then priv := tempn;
end;

begin {changeareas}

   reset(areasf);
   blockread(areasf, Temparea, 1);
   numareas := filesize(areasf)-1;
   uwriteln('');
   repeat { outer loop; until user goodbye }
      repeat { Getting a valid Command }
         UWriteln('');
         Command := upstring( Ask('Message areas: L)ist A)dd E)dit or Q): ',1));
         if command = '' then command:='Q';
         ok := Command[1] in ['A','E','L','Q'];
         if not ok then notacommand;
         Monitor
      until (ok) or (EUS);
      Monitor;
      if not EUS then
         Case Command[1] of
           'L' : begin (* display all settings *)
uclrscr;
uinterupted := false;
i := 1;
repeat
   showarea(i);
   inc(i);
   monitor
until (i>numareas) or (uinterupted) or (eus)
                     end;
               'A' : if (numareas < maxareas) and
                        (askYN('Add new area','N')='Y') then
                     begin  (* add a new area to the list *)
inc(numareas);
Temparea:=nularea;
seek(areasf,numareas);
{$i-} blockwrite(areasf,Temparea,1); {$i+}
if ioresult <> 0 then
begin
   uwriteln('No room');
   dec(numareas)
end
else
begin
   close(areasf); reset(areasf); {ensure directory is updated while leaving file open}
   str(numareas,uwtemp);
   uwriteln('Blank area #'+uwtemp+' added.')
end
                     end;
               'E' : begin (* Edit *)
temp := ask('Edit what entry (by number): ',2);
val(temp,d,err);
if (d>=1) and (d<=numareas) and (temp > '') and (err=0) and (not eus) then
begin
   showarea(d);
   if logactivity then writeln(TempLog,'   Change areas menu '+area.selector);

   temp := namecase(ask('Selector ('+Temparea.selector+'): ',14));
   if temp > '' then Temparea.selector := temp;

   tempcharSet := seperatorSet; seperatorSet := [];
   temp := ask('name ('+Temparea.name+'): ',40);
   seperatorSet := tempcharSet;
   if temp > '' then Temparea.name := temp;


   repeat
      temp := upstring(ask('Index Drive ('+Temparea.indexdrive+'): ',1));
      if temp = '' then temp := temparea.indexdrive;
      if not (temp in ['A'..HighestDrv]) then temp := ''
   until (length(temp)=1) or (eus);
   Temparea.indexdrive := temp;

   repeat
      temp := upstring(ask('Text Drive ('+Temparea.Textdrive+'): ',1));
      if temp = '' then temp := temparea.indexdrive;
      if not (temp in ['A'..HighestDrv]) then temp := ''
   until (length(temp)=1) or (eus);
   Temparea.textdrive := temp;

   EditPriv(Temparea.accesspriv,'Access priv');
   EditPriv(Temparea.EnterPriv,'E)nter priv');
   EditPriv(temparea.killpriv,'K)ill priv');
   editPriv(TempArea.KillAnyPriv,'Kill any Priv');
   EditPriv(TempArea.ReadPrivatePriv,'Read private priv');
   EditPriv(TempArea.HurlPriv,'Hurl from priv');

   repeat
      str(Temparea.attribute,temp);
      temp2 := ask('Attribute ('+temp+'): ',1);
      if temp2 = '' then temp2 := temp;
      val(temp2 ,tempn,err);
      if not (tempn in [0..2]) then err := 1
   until (err=0) or (eus);
   Temparea.attribute := tempn;

   if tempArea.attribute = 1 then
   begin
      EditPriv(Temparea.KillSentPriv,'Kill/Sent priv');
      EditPriv(Temparea.FileAttachPriv,'File Attach priv');
   end;

   if tempArea.attribute = 2 then {Echo}
   begin
      EditInteger(Temparea.ForwardNet,'Forward to Net#');
      EditInteger(Temparea.ForwardNode,'Forward to Node#');
   end;

   Temparea.fill := nularea.fill;

   if (not eus) and (AskYN('Okay to save changes','Y')='Y') then
   begin
      seek(areasf,d);
      blockwrite(areasf,Temparea,1) { important: write only done of not EUS }
   end
end
                  end
         end;
      Monitor
   until (Command[1] in ['Q']) or (EUS);

end; {changeareas}

overlay procedure changearea;
label exitchangearea;
var toarea: st14;
    toareal : byte absolute toarea;
    i : integer;
    temparea:areatype;
    nummatches,lastmatch,fsize:integer;
begin
   uclrscr;
{   reset(areasf);   surely it is already open!}
   fsize := filesize(areasf)-1;
   repeat
      toarea := namecase(ask('New area or ''?'' for list : ',14));
      if toarea = '' then goto exitchangearea;
      if toarea = '?' then
      begin
         uclrscr;
         uwriteln('Now in area '+area.selector);
         uwriteln('Message areas');
         i := 0;
         uinterupted := false;
         repeat
            inc(i);
            seek(areasf,i);
                        blockread(areasf,temparea,1);
            if user.priv >= temparea.accesspriv then
            begin
{ display }    fillchar(uwtemp[1],14,#32); uwtemp := temparea.selector; Uwtemp[0] := #14;
{ area menu }  uwrite(uwtemp+'-> ');
{ with }       case temparea.attribute of
{ flags}        0 : uwrite(' ');
                1 : uwrite('*');
                2 : uwrite('E')
               end;
               if shouldsearch[i] then
                  uwrite('!')
               else
                  uwrite(' ');
               uwriteln(' '+temparea.name);
            end;

            monitor
         until (i = fsize) or (uinterupted) or (eus);
      end;

      monitor
   until (toarea <> '?') or (eus);
   if eus then goto exitchangearea;

   i := 0; nummatches:=0; lastmatch :=0;
   repeat
      i := i + 1;
      seek(areasf,i);
      UServeBuff;
      blockread(areasf,temparea,1);
      UServeBuff;

{ this would give presedence to an area.selector starting '!' }
      if (
            (copy(temparea.selector,1,toareal) = toarea )
            and
            (user.priv >= temparea.accesspriv )
         )
         or
         (
            (toarea =  '!' )
            and
            (
               ( shouldsearch[i] )
               and
               ( user.priv >= temparea.accesspriv )
            )
         )
         or
         (
           (toarea = '*')
           and
           (
             (temparea.attribute = 1)
             and
             (user.priv >= temparea.accesspriv)
           )
         ) then

      begin
         inc(nummatches); lastmatch :=i
      end;
      monitor
   until (i = fsize) or
        ((temparea.selector = toarea)and(user.priv >= temparea.accesspriv)) or
         ((toarea='!')and(lastmatch>0)) or
         (eus);

   if ((temparea.selector = toarea)and(user.priv >= temparea.accesspriv)) or
      (nummatches = 1) then
   begin
      uclrscr;
      uwriteln('Wait ...');
      closemsgfiles;
      UServeBuff;
      monitor;
      if nummatches = 1 then
         areanum := lastmatch
      else
         areanum := i;
      user.lastarea := areanum;
      userchanged := true;
      monitor;
      openmsgfiles;
   end
   else
   begin
      uwriteln('No such area '''+toarea+'''');
      upressenter
   end;
exitchangearea:
end; { changearea }
