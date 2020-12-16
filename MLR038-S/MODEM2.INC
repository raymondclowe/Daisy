
{ This is the file MODEM2.INC, it should be used with MODEM1.INC }

{ Wait for an initial NAK or CRC }

function waitnak:boolean;
var c, errors : byte;

label exist;
begin
  errors:= 0;
  repeat                     { 60 seconds max }
    c:= modin(100);
    if (not carrier) then goto exist;
    if (not timeout) then
    begin
      if c = NAK then
      begin
        waitnak:= false;
        goto exist;
      end;
      if c = integer ('C') then
      begin
        waitnak:= true;
        goto exist;
      end;
    end;
    errors:= errors + 1;
  until  errors > 60;
  exist:
end;

{ Get an acknowledge character or timeout }

function getack:byte;
var c:byte;
label exist;

begin
  c:= modin(1000);
  if (not timeout) and ((c = ACK) or (c = NAK)) then
  begin
    getack:= c;
    goto exist;
  end
  else getack:= 255;
  exist:
end;

procedure updcrc(t:integer);
begin
  t:= t shl 8;
  crc:=crc xor t;

  for t:= 1 to 8 do
     if (crc and $8000) <> 0 then
     begin
       crc:=crc shl 1;
       crc:=crc xor $1021;
     end
     else
       crc:= crc shl 1;
end;

{ Transmit a filename for batch mode tranmission. Timeout if can't do it/
  It is assumed that the receiver is ready to receive the filename we have
  to send. }

procedure sendfn(name:st14);
var localname : st14;
    c,i       : byte;
    start     : integer;
label exist;
label badblk;
label loop;
begin
  start := elapsedtime; { mark external time }
  modout(integer('u')); { get off to a good start }
loop:
  show('Waiting for name NAK',3);
  i:= 0;
  repeat
    c:= modin(100);                    { get initial character }
    if (timeout) then c:= 255;
    i:= i + 1;
  until (i > 5) or (c = NAK) or (not carrier);  { got name NAK }
 if timeout then show('timeout name NAK',4);
  if (timeout) or (not carrier) then goto badblk;

  cvttofcb(name,localname);            { convert name }
  chksum:= SUB;                        { init checksum with a SUB }
  modout (ACK);                        { all ready }
  i:= 1;
  repeat
    c:= integer(upcase(localname[i]));         { get name char }
    delay(50);                                 { slow it down a bit? }
    modout(c);                         { send name char }
    chksum:= chksum + c;               { maintain checksum }
    i:= i + 1;
    c:= modin(200);
    if (c <> ACK) or timeout then
    begin
      if timeout then show('timeout on char ACK',4) else show('not ACK',4);
      goto badblk    { if not an ACK }
    end
  until (i > 11);                      { if all send OK }

  modout(SUB);                         { end of name }
  c:= modin(100);                      { get recvr's checksum }
  if timeout then show('timeout on checksum',4);
  if c<>chksum then show('bad checksum',4);
  if (not timeout) and (c = chksum) then
  begin
    modout (ACK);                      { if good, say so }
    goto exist;                        { return happy :-) }
  end;
  badblk:
  modout(integer('u'));                { else not send OK }
  if (elapsedtime < (start+60)) and (carrier) then goto loop;
  timeout:= true;                      { return sane :-( }
  exist:
end;

{ Get a filename from the sender. }

function getfn:st14;
var c,i:byte;
    newname:st14;

label exist;
label finish;
(**)label loop;
begin
(**)fillchar(newname,sizeof(newname),0);  { blank entire string }
(**)loop:
  modout(NAK);     { sync sender }
  c:= modin(200);  { get the sync character }
  if (c = EOT) and (not timeout) then
  begin
(**)fillchar(newname,sizeof(newname),0);  { blank entire string }
(*    show('No more file.',3);*)
    goto exist;
  end;
(**)if (c = $75{'u'}) and (not timeout) then goto loop; { reset }
  if (c <> ACK) then timeout:= true;
  if timeout then goto exist;
  chksum:= SUB; { init it with a SUB }
  for i:= 1 to 12 do         { notice: the last char is SUB }
  begin                      { 11 char max }
    c:= modin(100);          { get a char }
    if timeout then goto exist;        { timeout }
    case c of
    $75 : begin              { 'u' }   { error }
            timeout:= true;
            goto exist;
          end;
    SUB : goto finish;                 { end of name }
    EOT : begin                        { no more files }
(**)        fillchar(newname,sizeof(newname),0);  { blank entire string }
(*            show('No more file',3);*)
            goto exist;
          end;

    else
      begin
        chksum:= chksum + c;           { filename char I guess }
        newname:= newname + chr(c);    { stash it }
        delay(50);                     { slow it down a bit }
        modout (ACK);                  { say 'ok' }
      end;
    end;
  end;
  finish:
  begin
    modout(chksum);                    { send checksum }
    if (modin(200) = ACK) and (not timeout) then
    begin
      cvtfromfcb(newname,newname);     { fix name }
      goto exist;
    end;
  end;
  timeout:= true;
  exist:
  getfn:= newname;
end;

{ Receive a MODEM7 filename, timeout if error. }

function getfname:st14;
var newname:st14;
    i : byte;
    start : integer;
begin
  i:= 0;
  start := elapsedtime;  { mark external timer }
  repeat
    newname:= getfn;
    if (not timeout) then i:=60;
    i:= i + 1;
(**)  until (elapsedtime > start+15) or (i > 20) or (not carrier);
(*if (elapsedtime > start+15) then show('15 sec. timeout, no file',4);*)
  getfname:= newname;
end;

{ Transmit a XMODEM block with the specified sector number and
  checksum/CRC mode. }

procedure sendblock(sector:byte ;crcflag:boolean; buffer:bufftype);
var i      : byte;
begin
  modout(sector);            { send the sector number }
  modout(not sector);        { sned its complement }

  chksum:= 0;                { maintain both CRC and checksum }
  crc:= 0;
  i:= 1;
  repeat                     { output data and do CRC/checksum }
    modout(buffer[i]);
    chksum:= chksum + buffer[i];
    updcrc(buffer[i]);
    i:= i + 1;
  until i > 128;
  passtime(1152); { at 1200 baud }
                        { must add a 50ms for the receiver  }
  delay(50);            { this value tested under 1200/2400 }
                        { do NOT change it !!               }

  if ModKeypressed then ModInC(ch); { Flush garbage }

  if crcflag then
  begin
    modout(hi(crc));         { MS byte first }
    modout(lo(crc));         { then LS byte }
  end
  else
    modout(chksum);          { send checksum }
end;

{ Get an XMODEM block, return its sequence number, or timeout if error.
  This gets the sector number, its 1's complement, the 128 data bytes and
  does the cecksum or CRC. }

function getblock(crcflag:boolean; var buffer:bufftype):byte;
var i,v    : byte;
    c1,c2  : byte;
    sector : byte;

label exist;
begin
  sector:= modin(100);       { get the sector number }
  chksum:= modin(100);       { 1's compl (use chksum as temp) }
  if ((sector or chksum) <> 255) or (timeout) then
  begin
     show('bad sector',4);
     goto exist;{ bad sector ! }
  end;
  chksum:=0;       { initialize checksum and CRC }
  crc:=0;          { we maintain both in parallel }
  i:=1;
  repeat
    v:= modin(100);
    buffer[i]:= v;
    chksum:= chksum + v;
    updcrc(v);
    i:=i + 1;
  until timeout or (i>128);
 if timeout then show('timeout',4);
  if crcflag then            { if CRC mode, get the two }
  begin
    c1:= modin(100);         { CRC bytes and do those }
    if timeout then goto exist;
    c2:= modin(100);
    if timeout then goto exist;
    if CRC <> c1*256 + c2 then
    begin
      timeout:= true;        { timeout if bad CRC }
      goto exist;
    end;
  end
  else
  begin
    v:= modin(100);          { get the checksum }
    if timeout then goto exist;
    if v <> chksum  then
    begin
      timeout:= true;
      goto exist;
    end;
  end;
  getblock:= sector;         { good block }
  exist:
end;

{ Attempt to send the TELINK data block. Do only 4 attempts. }

procedure sendtel(fn:st14);
var sendprogram : string[16];
    i,c         : byte;
    h,l      : real;
(**)FS       : integer;
begin
  fillchar(telbuf,128,0);
  sendprogram:='Daisy Mailer';
{full name including d:}
assign (f,transfn);
  reset (f);
  fs:= filesize(f);
(*  fs:= fs * 128;             { each record contain 128 bytes }
  l:= ((fs / 65536.0) - int(fs / 65536.0)) * 65536.0; { Low byte }
  h:= (fs - l) / 65536.0;                             { high byte }
  telink.flengthl:= round(l);
  telink.flengthh:= round(h);*)

(**)telink.FLengthL := fs shl 7;
(**)telink.FLengthH := hi(fs) shr 1;

  close (f);
  for i:= 1 to 16 do telink.fname[i]:= ' '; { install filename }
  for i:= 1 to length(fn) do telink.fname[i]:= upcase(fn[i]);
  for i:= 1 to length(sendprogram) do
  telink.sendingProg[i]:= sendprogram[i];
  telink.telcrc:= 1;                        { send the CRC flag }
  i:= 0;
  repeat
    modout(SYN);
    sendblock(0,false,telbuf);              { send in checksum }
    i:= i + 1;
    c:= modin(1000);
  until ((c = ACK) and (not timeout)) or (i = 4) or (not carrier);
end;

function sendfile(fn:st14): integer;

var i : byte;
    sector : byte;
    aborts : byte;
    errors : byte;
    buff : bufftype;

label exist;
label goodblk;
begin
  {full filename including d:}
  assign (f,transfn);
  reset(f);                  { open file }
  sector:= 1;                { first XMODEM block number }
  totl_blocks:= 1;
  errors:= 0;
  aborts:= 0;

  if mode <> 2 then  { if this is telink then the telink block has }
  begin              { already been ACKed so go ahead and send first xmB }
(**)     crcmode := (ch=req_crc); { ch was received by s.inc }
(*     repeat
       crcmode:= waitnak;         { get the initial NAK/CRC mode }
       errors:= errors + 1;
     until (not timeout) or (errors = 10);
     if timeout then
     begin
        show('No NAK/CRC, error',4);
        goto exist;{ if not NAK/CRC, error }
     end;*)
  end;

  if (mode = 2) then crcmode:= true;  { CRC for Telink !! }

  errors:= 0;
  repeat
    blockread(f,buff,1);     { read some file data }
    while errors<10 do
    begin
      write(cr,'Sending #',totl_blocks);        { stats for the nosy sysop }
      modout(SOH);           { tell rcver a block comes }
      sendblock(sector,crcmode,buff);
      errors:= succ(errors);
      i:= getack;            { get an acknowledge }
      if i = ACK then
      begin
        sector:= succ(sector);
        totl_blocks:= succ(totl_blocks);
        errors:= 0;
        goto goodblk;
      end;
      totl_errors:= succ(totl_errors);           { another error }
      if timeout then errors:= 10;
    end;
  goodblk:
  until eof(f) or (errors = 10);
  exist:
  close(f);

  { All blocks sent, or too many errors. Tell the remote bo more. }

  writeln;
  i:= 5;
  repeat
(*    show('File EOT',3);*)
    modout(EOT);
    i:= i - 1;
(**)  until ((modin(100) in [ACK,NAK]) and (not timeout)) or
            (i = 0) or
            (not ModCarrier);

(*if (timeout) then show('Timeout EOTing',4);*)
(*if (not carrier) then show('No carrier',4);*)
(*if (errors = 10) then show('10 errors',4);*)
  if (timeout) or (not carrier) or (errors = 10) then sendfile:= -1
  else sendfile:= 0;
end;

function getfile(fn:st14): integer;

var i,j    : byte;
    x      : byte;
    sector : byte;
    buff   : bufftype;
    errors : byte;
    ackchar: byte;
    aborts : byte;
(**)fs     : real;
(**)start  : integer;

label goodblk;
label exist;
(**)Label badblock;
begin
    assign(f,fn);
    rewrite(f);
    aborts:= 0;
    errors:= 0;
    totl_blocks:= 1;
    sector:= 1;              { XMODEM block number }

    ackchar:= NAK;
    if (crcmode) then ackchar:= integer('C');    { use right initial char }

    repeat
      modout(ackchar);
      errors:= succ(errors); { assume an error }
      j:= modin(400);        { get initial character }
      if not timeout then
      begin
        case j of            { see what it is }
          SOH : begin
                  i:= getblock(crcmode,buff);    { get sect number }
                  write(cr,'Receiving #',totl_blocks);
                  ackchar:= NAK;                 { assume bad }
                  if (not timeout) then
                  begin
                    if (i < sector) then         { duplicate }
                    begin
                      show('Duplicate',4);
                      goto goodblk;              { ACK it }
                    end;
                    if (i > sector) then         { out of sync }
                    begin
                      show('Block Sync Error',4);
(**)                  goto badblock
                    end
                    else
                    begin
                      if not CFG.IgnoreFiles then
                         {$I-} blockwrite(f,buff,1); {$I+}
                      if ioresult <> 0 then
                      begin
                         Errors := 255;
(**)                     goto badblock
                      end;
                      totl_blocks:= succ(totl_blocks);
                      sector:= succ(sector);
                    end;

                  goodblk:
                    aborts:= 0;
                    errors:= 0;
                    ackchar:= ACK;
(**)              badblock: { leave ackchar = nak and ++errors }
                  end;
                end;

          SYN : begin        { TELINK block }
                  ackchar:= NAK;                 { assume bad }
                  if (getblock(false,telbuf) = 0) and (not timeout) then
                  begin

                    show('Telink Block',3);
                    aborts:= 0;
                    errors:= 0;
                    ackchar:= ACK;
(**)                fs := lo(telink.FLengthL) + (hi(telink.flengthL)*256.0) +
                          (lo(telink.flengthh)*65536.0) + (hi(telink.flengthh)*16777216.0);
(**)writeln(copy(fn,3,12),' is ',round((fs / 1024.0)+0.5),'K (',trunc(fs / 128),' blocks) long');
(**)                if (not CFG.IgnoreFiles ) and
                       ((freespace(CFG.fileattachdrv)-1) < (fs / 1024)) then
                    begin
                       report('No space for this file!',4);
                       errors := 11
                    end
                  end;
                end;

          EOT : begin        { end of file }
                  writeln;
(*                  show('End of file EOT',3);*)
                  modout(ACK);
                  return:= 0;
                  goto exist;
                end;
(**)      174,241 {tsynch} : begin        { stray character }
(**)                  modout(ackchar);
                   end;
         else
         begin
            writeln;
            show('Bad header',4);
            { Wait clear line }
(**)        start:=elapsedtime;
(**)        repeat x := modIn(50)
(**)        until (timeout) or (elapsedtime > start+30) or (not ModCarrier);
(**)        if not timeout then errors := 11;
         end
        end;
      end
      else
      begin
         show('Timeout block header',4);
         if (totl_blocks=1) and         { still not got first data block }
            (mode=2) and                { is telink }
            (ackchar=ack) and           { we've got telink block oaky }
            (errors>5) then             { isn't responding to ack }
            ackchar := integer('C');    { maybe fool needs a Req_CRC! }
      end;

      if (timeout) or (errors <> 0) then totl_errors:= succ(totl_errors);

    until errors > 10;
    writeln;
    return:= -1;
    exist:
    if filesize(f)=0 then return := -1;
    getfile:= return;
    close(f);
    if (return < 0) then erase(f);
end;
