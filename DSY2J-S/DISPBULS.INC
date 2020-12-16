{ 11/10/87 }
OVERLAY procedure DisplayBuls;
label exit, loop;
var
    bul : bultype;
    bulmenu : st14;
    numbuls, i : integer;
    b : st1;
    exitselector : st1;
    autoexit,displayed : boolean;
    prmt : st12;

begin
   if logactivity then writeln(TempLog,'   Bulletin');

   if bulslistfn > '' then
   begin
      assign(bulsf,bulslistfn);
      {$i-} reset(bulsf); {$i+}
      if ioresult = 0 then
      begin
         read(bulsf, bul);
         bulmenu := bul.bulfile;
         exitselector := bul.selector;
         autoexit := (exitselector='*');
         numbuls := filesize(bulsf)-1;

         if (bulmenu > '') and (numbuls > 0) then
         begin
repeat
   if inputbuffer = '' then displayfile(bulmenu);   { show menu }
   repeat                                           { get valid input or exit }
loop: b := upstring(ask('',1));
      if ((b = '') and (autoexit)) or
         (b = exitselector) or (eus) then goto exit;
      seek(bulsf,1);
      repeat
         read(bulsf,bul);
         monitor
       until (eof(bulsf)) or (bul.selector = b) or (eus);

       if ((b='')and(not autoexit)) or { CR and not autoexit or }
          (not ((b = bul.selector) or (b=exitselector))) then
       begin
          uwriteln('');
          uwrite('Try again: ');
          goto loop;
       end;
    until ((b = bul.selector) or (b=exitselector)) or (eus);
    if b=bul.selector then
    begin
       displayfile(bul.bulfile);
       if bul.return then
          upressenter
       else
          goto exit;
    end
    else
    begin
       if b <> exitselector then
          uwriteln('No such bulletin')
       else
          goto exit;
    end;

until (eus)
         end
         else
            uwriteln('No menu or no bulletins')
      end
      else
         uwriteln('Not able to open '+bulslistfn)
   end
   else
      displayfile(bulletinfn);

exit:
   uwriteln('')
end;
