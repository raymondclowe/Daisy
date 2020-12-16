{ 26/9/88 }

var logf:file;
var logbuf : st128;
    logbuflen : byte absolute logbuf;
    logopenf : boolean;

procedure logwrite(l:st128);
begin
   while l > '' do
   begin
      if logbuflen =128 then
      begin
         blockwrite(logf,logbuf[1],1);
         fillchar(logbuf,sizeof(logbuf),0);
         if ioresult <> 0 then
         begin
            writeln('Error writing log');
            rewrite(logf);
            logbuf := '* Old log erased due to disk error'+cr+lf;
         end
      end;
      logbuf := logbuf + l[1];
      delete(l,1,1)
   end
end;


procedure logopen;
label exit;
var i : integer;

begin
   if logopenf then goto exit;
   logopenf := true;
(*   if cfg.appendlogtodaisylog then
      assign(logf,systemlogfn)
   else*)
      assign(logf,CFG.WorkDrv+':MAIL.LOG');
   fillchar(logbuf,sizeof(logbuf),0);
   {$i-} reset(logf); {$i+}
   if ioresult = 0 then
   begin
      i := filesize(logf)-1;
      if i < 0 then
      begin
         close(logf);
         rewrite(logf)
      end
      else
      begin
         seek(logf,i); { move to lastblock }
         blockread(logf,logbuf[1],1); logbuf[0]:=#128; { read in }
         while logbuf[logbuflen] = ^Z do
            logbuflen := pred(logbuflen); { strip away fill }
            { set up buffer with content of last block }

         seek(logf,i); { position pointer ready to write here }
      end
   end
   else
      rewrite(logf);
exit:
end;

procedure logclose;
begin

   while logbufLen<128 do
      logbuf := logbuf + ^Z;
   blockwrite(logf,logbuf[1],1);

   close(logf);
end;

procedure inc( var i : integer );
begin i := succ(i) end;

procedure dec( var i : integer );
begin i := pred(i) end;

var elapsedtime,mscounter:integer;
procedure rawdelay(ms:integer);
begin delay(ms) end;

procedure Delay(ms:integer);
begin
   msCounter := msCounter + ms;
   while msCounter > 1000 do
   begin
      elapsedTime := elapsedTime + 1;
      msCounter := msCounter - 1000
   end;
   rawDelay(ms);
end;

procedure passtime(ms:integer); { inc time counter }
begin
   msCounter := msCounter + ms;
   while msCounter > 1000 do
   begin
      elapsedTime := elapsedTime + 1;
      msCounter := msCounter - 1000
   end
end;

(*
procedure rawhalt;
begin halt end;

procedure halt;
begin writeln('System HALT'); rawhalt end;
*)

function upstring( l : st255):st255;
var i : integer;
begin
  for i := 1 to length(l) do
     l[i] := upcase(l[i]);
  upstring := l;
end;

function fileexists(fn:st14):boolean;
var f:file;
begin
   passtime(250);
   assign(f,fn);
   {$i-} reset(f); {$i+}
   fileexists := (ioresult = 0)
end;

procedure DiskReset;
var i:integer;
begin
   passtime(1000);
   i := bdos(25);
   bdos(13);
   bdos(14,i)
end;

procedure dispch(c:char); { display if printable else <ASCIIn> }
begin
   if c in [nul,cr,lf,' '..'~'] then write(c) else write('<',ord(c),'>')
end;

procedure show(l:st255; t:byte);
begin
(*   gotoxy(1,16 + t);*)
   writeln(l (*,esc,'Y'*) );
end;

procedure report(l:st255; t:byte );
begin
   show('> '+l,0);
   if CFG.KeepMailLog then logwrite(l+cr+lf)
end;

var DMA_buffer      : array [0..127] of byte;

var
current_drive   : integer;
{ 25/7/88 }
{ Returns the amount of free space on drive in K }
{ Based on the RawDir directory routine by Stanley Yau }
{ drive = ' ' = current drive }
function freespace(drive:char):integer;
label exit;
var
DMA             : integer;
DPB             : integer;
i           : integer;
record_block    : integer;
counter         : integer;
alloc           : integer;
remain          : integer;


function NumOfBit(t:integer):integer;
var x,y:integer;
begin
  y:=0;
  for x:= 0 to 7 do
  begin
    y:= y + (t mod 2);
    t:= t div 2;
  end;
  NumOfBit:= 8 - y;
end;

begin
  if not (drive in [' ','A'..'F']) then
  begin
     freespace := 0;
     goto exit
  end;
  current_drive:= bdos(25);
  if drive <> ' ' then bdos(14,ord(drive)-65);
  DMA:= addr(DMA_buffer[0]);
  bdos(26,DMA);               { Function 26: Set DMA Address  }

  { Calculate the disk space }

  DPB:= bdosHL(31);           { Function 31: Get Addr (Disk Parms) }
  record_block:= mem[DPB+2];
  counter:= mem[DPB+5] + mem[DPB+6] * 256 + 1;
  counter:= (counter div 8) - 1;
  alloc:= bdosHL(27);         { Function 27: Get Addr (Alloc) }
  remain:= 0;
  for i:= 0 to counter do
  begin
    remain:= remain + NumOfBit(mem[alloc+i]);
  end;
  remain:= remain + remain * (record_block-3);

  { Remain contain the number of bytes remained in the current drive }

  freespace := remain;
  bdos(14,current_drive);
exit:
end;


const maxfiles = 15;
var
filename        : array [1..maxfiles] of st12;
filecounter     : integer;
type st11 = string [11];
procedure rawdir (drv:char; find: st11; mask : st12);

label exit;
var
file_buffer     : array [0..35] of byte;
length_buffer   : array [0..35] of byte;
DMA             : integer;
FCB             : integer;
DPB             : integer;
i,j,t           : integer;
index           : integer;
record_block    : integer;
counter         : integer;
drive           : char;
temp            : st12;
flag            : boolean;
begin
  current_drive:= bdos(25);
  bdos(14,integer(drv)-65);

  fillchar(file_buffer,36,0);
  move(find,file_buffer,12);
  file_buffer[0]:=0;
  DMA:= addr(DMA_buffer[0]);
  FCB:= addr(file_buffer[0]);
  bdos(26,DMA);               { Function 26: Set DMA Address  }
  index:= bdos(17,FCB);       { Function 17: Search for First }
  filecounter := 0;
  while (index <> 255) and (filecounter < maxfiles) do
  begin
   if integer(DMA_buffer[ index * 32 ] + DMA_buffer[ (index * 32) + 12 ]) = 0 then
   begin
     filecounter:=filecounter+1;
     for i:=1 to 11 do
     begin
       filename[filecounter][i]:=chr(DMA_buffer[ i + (index * 32) ]);
       filename[filecounter][0]:=chr(11);
     end;
     insert('.',filename[filecounter],9);
     while pos(' ',filename[filecounter]) > 0 do
        delete(filename[filecounter],pos(' ',filename[filecounter]),1);
     if mask > '' then
     begin
        flag := false;
        if length(filename[filecounter]) < 12 then
           flag := true
        else
           for i := 1 to 12 do
           begin
              if (copy(copy(find,1,8)+'.'+copy(find,9,3),i,1) = '?') and
                 (copy(mask,i,1) = 'h') then {wild and hex}
                 if not (filename[filecounter][i] in ['0'..'9','A'..'F']) then
                    flag := true;
           end;
        if flag then filecounter := filecounter - 1;
     end

   end;

   index:= bdos(18);       { Function 18: Search for Next  }
  end;
  bdos(14,current_drive);
end;
type sltype = array[1..2] of st128;
var subline: ^sltype;
    numsublines : integer;

procedure submit;
type pfile=file;
var f2:^pfile;
    subfn:ST14;
    l,I:integer;
begin
   passtime(1000);
   if cpam then subfn := '$@$.SUB' else subfn := 'A:$$$.SUB';
   new(f2);
   { open the sub file }
   assign(f2^,subFN);
   {$I-} reset(f2^); {$I+}
   if ioresult = 0 then
      seek(f2^,filesize(f2^))
   else
      rewrite(f2^);

   { write into the sub file }
   for l := numsublines downto 1 do
   begin
      subLINE^[L] := subLINE^[L] + #0'$';
      blockwrite(f2^,subLINE^[L],1)
   end;

   close(f2^);
   dispose(f2)
end;


var arcpkt : st12; { nul or filename }

function arctest : boolean;
var result,flag : boolean;
    i,j : integer;
begin
   result := false;

   rawdir(cfg.FileAttachDrv,'???????????', 'hhhhhhhh.xxh');
   if filecounter > 0 then
   begin
      for i := 1 to filecounter do
      begin
         if POS(filename[i][10]+filename[i][11]+'_', 'MO_TU_WE_TH_FR_SA_SU_') > 0 then
         begin
            flag := true;
            for j := 1 to 8 do
               if not (filename[i][j] in ['0'..'9','A'..'F']) then flag := false;
            if flag then
            begin
               result := true;
               arcpkt := filename[i]
            end
         end
      end
   end;

   arctest := result;
end;