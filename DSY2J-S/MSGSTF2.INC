{ 27/9/88 }

var continuous  : boolean;
    ReadMsg : ^MsgHeadertype;
    maskstring : st40;
var LastReadMsgNum : integer;
    strLastReadMsgNum : st4;
    brief      : boolean;
    OldDirection : char;
    topoftext : integer; { could be byte? }
    userconsoletype : byte; {used to maintain this value while continuous dumpig }

{ stuff for msgread and msgstff to use }
procedure clearrestofpage;
begin
   if user.consoletype <> dumb then
      while (linecount < user.consolehight - 3) and (not uinterupted) and (not eus) do
      begin
         uclreol; uwriteln('');
         UCtrlSC;
         monitor
      end
end;

procedure displaytext(b:integer);
var p,i : integer;
    line: st255;
    newline: st128;
    lastblock:boolean;

procedure outline(l:st128);
begin
   if not ((l > '') and
           (l[1] = ^A) and
           (user.priv < kludgepriv)
          ) then
      uwriteln(l)

end;

begin
   uinterupted := false;
   if not continuous then ugotoxy(0,topoftext);

   UCtrlSC; {$i-} seek(msgtext, b)(**91**); {$i+} tick; UCtrlSC;
   ioerr := ioresult;
   if ioerr <>0 then
   begin
      line := cr+cr+'* Daisy Seek error!'+#00;
      if logactivity then writeln(TempLog,'* Seek error');
   end
   else
   begin
      blockread(msgtext, line[1], 1); tick; UCtrlSC; line[0] := chr(128);
   end;
   p := pos(nul,line);
   if p>0 then
   begin
      line[0] := chr(p-1);
      lastblock := true
   end
   else
      lastblock := false;
   repeat
      if (length(line) < user.consolewidth) and (not lastblock) then
      begin
         UCtrlSC; blockread(msgtext, newline[1], 1); UCtrlSC; newline[0] := chr(128);
         line := line + newline;
         p := pos(nul,line);
         if p>0 then
         begin
            line[0] := chr(p-1);
            lastblock := true
         end
      end;
(*writeln('starting print process with');zwriteln('>'+line+'<');*)
      p := pos(cr,line);
      uclreol;
      if (p > 0) and (p <= user.consolewidth) then
      begin
         outline(copy(line,1,p-1));
         delete(line,1,p)
      end
      else
         if (length(line) < user.consolewidth) and lastblock then
         begin
            outline(line);
            line := ''
         end
         else
         begin
            p := user.consolewidth;
            repeat p := pred(p)
            until (line[p] in [' ', '-', '/']) or (p = 1);
            if p = 1 then
            begin
               outline(copy(line,1,user.consolewidth-1));
               delete(line,1,user.consolewidth-1)
            end
            else
            begin
               outline(copy(line,1,p));
               delete(line,1,p)
            end
         end;
      if (user.consoletype <> dumb) and (linecount = user.consolehight - 3) and (not continuous) then
      begin
         uclreol;
         uinterupted := (AskYN('More','Y') = 'N');
         ugotoxy(0,user.consolehight-3); uclreol;
         if not uinterupted then
            if not continuous then ugotoxy(0,topoftext)
      end;
      UCtrlSC;
      monitor
   until ((line = '') and lastblock) or (uinterupted) or (EUS);
   if not continuous then clearrestofpage;

end; { displaytext }
