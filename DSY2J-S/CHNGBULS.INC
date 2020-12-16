{ 8/10/87 }
OVERLAY procedure changeBulletins;

var command:st1;

    bul : bultype;
    bulmenu,
    temp : st14;
    numbuls,
    i,d,err : integer;
    s : st1;
    tempreturn, ok : boolean;

begin {changeBulletins}
   assign(bulsf,bulslistfn);
   {$i-} reset(bulsf); {$i+}
   if ioresult > 0 then
      uwriteln(oerr+bulslistfn)
   else
   begin
      read(bulsf, bul);
      bulmenu := bul.bulfile;
      numbuls := filesize(bulsf)-1;
      uwriteln('');
      repeat { outer loop; until user goodbye }
         repeat { Getting a valid Command }
            UWriteln('');
            Command := upstring( Ask('Change Bulletins: L)ist M)enu A)dd C)hange or Q): ',1));
            if command = '' then command:='Q';
            ok := Command[1] in ['A','C','L','M','Q'];
            if not ok then NotACommand;
            Monitor
         until (ok) or (EUS);
         Monitor;
         fillchar(bul,sizeof(bul),0);
         if not EUS then
            Case Command[1] of
               'L' : begin (* display all settings *)
uclrscr;
if bulmenu = '' then
   uwriteln('There is no bulletin menu specified')
else
begin
   seek(bulsf,0);
   read(bulsf,bul);
   uwriteln('Menu          : '+bul.bulfile);
   uwriteln('Exit selector : '+bul.selector)
end;
uwriteln('');
if numbuls = 0 then
   uwriteln('There are no bulletins specified')
else
begin
   uwriteln('  : Selector : File');
   for i := 1 to numbuls do
   begin
      seek(bulsf,i);
      read(bulsf,bul);
      str(i:2,uwtemp);
      uwrite(uwtemp+': '+bul.selector+'        : '+bul.bulfile+' (');
      if bul.return then uwriteln('Return)') else uwriteln('Exit)');
   end
end;
seek(bulsf,0);
read(bulsf,bul);
uwriteln('')
                     end;
               'M' : begin (* change the file name of the menu *)
seek(bulsf,0);
read(bulsf,bul);
uwriteln('Menu          : '+bul.bulfile);
uwriteln('Exit selector : '+bul.selector);

bulmenu := upstring(ask('Bulletin menu file name: ',14));
if bulmenu = '' then
begin
   bulmenu := bul.bulfile;
   uwriteln('Menu : '+bul.bulfile)
end;

repeat
   s := upstring(ask('Selector to exit ("*" for automatic exit): ',1))
until (s>'') or (eus);
if not eus then
begin
   bul.selector := s;
   bul.bulfile := bulmenu;
   seek(bulsf,0);
   write(bulsf,bul)
end
                     end;
               'A' : if askYN('Add new bulletin','N')='Y' then
                     begin (* add a new Bulletin to the list *)
fillchar(bul,sizeof(bul),0);
bul.bulfile := upstring(ask('File name: ',14));
repeat
   s := upstring(ask('Selector : ',1));
until (s>'') or (eus);
bul.selector := s;
bul.return := (askyn('Return','Y')='Y');
inc(numbuls);
seek(bulsf,numbuls);
write(bulsf,bul)
                     end;
               'C' : begin (* Change *)
temp := ask('Change what entry (by number): ',2);
val(temp,d,err);
if (d>=1) and (d<=numbuls) and (temp > '') and (not eus) then
begin
   seek(bulsf,d);
   read(bulsf,bul);
   str(d:2,uwtemp);
   uwrite(uwtemp+': '+bul.selector+'        : '+bul.bulfile+' (');
   if bul.return then uwriteln('Return)') else uwriteln('Exit)');
   repeat
      temp := upstring(ask('File name: ',14))
   until (temp>'') or (eus);
   repeat
      s := upstring(ask('Selector : ',1))
   until (s>'') or (eus);
   tempreturn := (askyn('Return','Y')='Y');
   if not eus then
   begin
      bul.bulfile := temp;
      bul.selector := s;
      bul.return := tempreturn;
      seek(bulsf,d);
      write(bulsf,bul)
   end
end
                     end
            end;
         Monitor
      until (Command[1] in ['Q']) or (EUS);

      close(bulsf)
   end
end; {changeBulletins}
