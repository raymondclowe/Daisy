{ 8/7/88 }

procedure MsgRead;
var exit : boolean;
    InputLine : st4;
    ReadMsgNum,
    Numerr : integer;
    Commandst1 : st1;
    msgprompt : ^st80;

type soc = set of char;
var tempcharSet : ^soc;

   {userconsoletype:byte;  declared in msgstf2.inc}

procedure introarea;
begin
   if logactivity then writeln(TempLog,'. Area :'+area.selector);

   if user.highestread[areanum] > highestmsgnum then
   begin { this is used occasionaly when highestmsgnum scans down past other users highestread }
      user.highestread[areanum] := highestmsgnum;
      userchanged := true
   end;
   LastReadMsgNum := user.highestread[areanum];
   if shouldsearch[areanum] then MailSearch else uclrscr;
   shouldsearch[areanum] := false;
   uwriteln(area.name);
   UWriteln('Read messages.');
   Uwriteln('');
   replyMsgheader := nulMsgheader;
   replymsgnum := 0;
   readmsg^ := nulmsgheader;
   OldDirection := 'N';
   maskstring := '';
   brief := false;
   Continuous := false
end;


begin { Msg read }

   new(readmsg);
   introarea;
   exit := false;
   repeat
      str(HighestMsgNum, strHighestMsgNum );
      str(LastReadMsgNum, strLastReadMsgNum );
      if user.consoletype = dumb then UWriteln(''); { blank line before prompt }

      new(msgprompt);
      if olddirection = 'N' then MsgPrompt^ := '>' else MsgPrompt^ := '<';
      MsgPrompt^ := MsgPrompt^ + '['+strLastReadMsgNum+']';
      if maskstring > '' then MsgPrompt^ := MsgPrompt^ + '"' + maskstring + '"';
      if brief then MsgPrompt^ := MsgPrompt^ + '<B>';
      MsgPrompt^ := MsgPrompt^ + ' 1 - '+StrHighestMsgNum+' ';
      if user.singlekey then MsgPrompt^ := MsgPrompt^ + '# ';
      case user.help of
       3 : MsgPrompt^ := MsgPrompt^ + 'A)rea E)nter N)ext Q)uit or ?) for other commands : ';
       2 : MsgPrompt^ := MsgPrompt^ + 'A E R N P K Q M T B = or ? for help : ';
       1 : MsgPrompt^ := MsgPrompt^ + ': '
      end;
      if user.consoletype <> 0 then
      begin
         paginate := false;
         Ugotoxy(user.consolewidth-length(area.selector),0);
         uwrite(area.selector);
         ugotoxy(0,user.consolehight-3);
         uclreol
      end;

      if user.singlekey then
      begin
         InputLine := Ask(MsgPrompt^,1);
         if (inputline >= '0') and (inputline <= '9') then
         begin
            uwriteln('Use "#" to select by number');
            inputline := ' ' {' '::=do nothing}
         end;
         if inputline = '#' then
         begin
            uclreol;
            InputLine := Ask('Msg #: ',4);
            ugotoxy(0,user.consolehight-2);
         end
      end
      else
         InputLine := Ask(MsgPrompt^,4);
      dispose(msgprompt);

      uclreol;
      if inputline = '' then inputline := olddirection;
      monitor;
      if not EUS then
      begin
         val( InputLine, ReadMsgNum, NumErr);
         if (Numerr = 0) and (inputline<>'-') then
         begin
            if ReadMsgNum in [1..HighestMsgNum] then
               callDisplayMsg( readMsgNum )
            else
               UWriteln( 'Must be 1 - '+StrHighestMsgNum)
         end
         else
         begin
            monitor;
            if not EUS then
            begin
               Commandst1 := UpString( InputLine[1] );
               if Commandst1 in ['A','E','Q','N','P','K','H','S','I','F','M',
                                 'T','B','*','<','>','=','-','R','?',' ',^H,^I,^U] then
               begin

if commandst1 = '-' then
   if replymsgheader.replyto <> -1 then calldisplaymsg(replymsgheader.replyto);
if commandst1 = 'A' then
begin
   changearea;
   introarea
end;

if Commandst1 = 'Q' then exit := true;
IF (Commandst1 in ['E','R']) and (user.priv >= area.enterpriv) then
begin
   reply := (commandst1 = 'R');
   Msgenter
end;

if commandst1 in ['N',^I,^U] then
begin
   olddirection := 'N';
   callDisplayMsg( LastreadMsgNum + 1 )
end;
if commandst1 in ['P',^H] then
begin
   olddirection := 'P';
   callDisplayMsg( LastreadMsgNum - 1 )
end;
if commandst1 = '=' then
begin
   uclrscr;
   userconsoletype := user.consoletype; user.consoletype := 0;
   paginate := (Askyn('Continuous','Y')='N'); { as opposed to paged }
   continuous := true;
   if olddirection = 'N' then
      callDisplayMsg( LastreadMsgNum + 1)
   else
      callDisplayMsg( LastreadMsgNum - 1);
   Continuous := false; { just in case? }
   user.consoletype := userconsoletype;
   paginate := user.more
end;

if Commandst1 = '>' then olddirection := 'N';
if Commandst1 = '<' then olddirection := 'P';

if (commandst1 = 'H') and (user.priv >= area.hurlpriv) then hurl(LastReadMsgNum);

if (Commandst1 = 'K') and (lastreadmsgnum <> 0) and (user.priv >= area.killpriv) then
begin
   MsgKill(LastReadMsgNum);
   if olddirection = 'N' then
      callDisplayMsg( LastReadMsgNum + 1 )
   else
      callDisplayMsg( LastReadMsgNum - 1 );

   if lastreadmsgnum > highestmsgnum then
   begin
      lastreadmsgnum := highestmsgnum;
      updatemsgstats
   end;

   if user.highestread[areanum] > highestmsgnum then
   begin
      user.highestread[areanum] := highestmsgnum;
      userchanged := true
   end
end;
if Commandst1 = 'S' then MailSearch;
if Commandst1 in ['I','F'] then Inquire;
if Commandst1 = 'M' then
begin
   new(tempcharset);
   tempcharSet^ := seperatorSet; seperatorSet := []; askstrip := false;
   maskstring := upstring(Ask('Mask string: ',40));
   seperatorSet := tempcharSet^; askstrip := true;
   dispose(tempcharset)
end;
if (Commandst1 = 'T') and
   (replymsgheader.pointertobody <> -1) and
   (replymsgheader.fromperson[1] <> 'd') then
    displaytext(replymsgheader.pointertobody);
if commandst1 = 'B' then
begin
   brief := not brief;
   uwrite('Brief mode o');
   if brief then
      uwriteln('n')
   else
      uwriteln('ff')
end;
if Commandst1 = '*' then
begin
   if user.highestread[areanum] = HighestMsgNum then
      callDisplayMsg( user.highestread[areanum] )
   else
      callDisplayMsg( user.highestread[areanum] + 1 )
end;
if commandst1 = '?' then
begin
   paginate := user.more;
   displayfile(helpdrv+':HELP1.BBS');
   upressenter; uclrscr
end
               end
               else
                  callDisplayMsg( LastreadMsgNum )
            end
         end
      end;
      monitor
   until exit or (EUS);
   dispose(readmsg);

(*   if Userchanged then { do update User file , even if no carrier }
      updateuser;*)

   paginate := user.more
end; { MsgRead }
