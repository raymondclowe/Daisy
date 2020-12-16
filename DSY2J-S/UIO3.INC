{ 30/6/88 }

var parallel : boolean; { Parallel keyboards }
var linecount : integer;
var UWtemp : st128; { temporary string used for converting integer to str }
const bufflen = 32; { no more tha 4 or 5 chars ever seem to be properly buffed }
var buffhead, bufftail : byte;
    iobuff : array [0..bufflen] of char;
    NotTermControl : boolean;
    validset : set of char;

{ getchar unbuffered }
procedure Ugetchardir( var ch:char );
var chbyte : byte absolute ch;
begin
   if not EUS then
   begin
      if (userlocation = local) or ((parallel) and (keypressed)) then
      begin
         read(kbd,ch);
         if (parallel) then
            if (ch in [ESC,^P]) then
            begin
               parallel := false;
               write('<Parallel OFF>');
               ch:=nul;
            end
      end
      else
         Modemgetchar(ch) { this is the ONLY actual reading of the modem }
   end;
   if not (ch in [tsynch,yoohoo]) then chbyte  := chByte and $7f;
end;

{--------------- keypressed ----------------}
{ user keypressed }
function ukeypressed:boolean;
begin
   if userlocation = remote then
   begin
      if parallel then
         ukeypressed := modemkeypressed or keypressed
      else
         ukeypressed := modemkeypressed
   end
   else
      ukeypressed := keypressed
end;

procedure uclreol; FORWARD;
procedure ucurup; FORWARD;

{-- interupt & pause --}
{ UInterupted should be set false before something you want to check on,
  then while displaying the interuptable/pausable thing keep calling UCtrl SC,
  it will pause on a CTRL-S, and will set UInterupted on a ^C. So your displaying
  routine must look at UInterupted, and stop if it goes true }
{ this routine now also buffers an received chars that are not break or pause }
{ This routine handles ^S, ^C and buffering. - SK }
{ ^S and ^C can be controled via ctrls, ctrlc:boolean }
{ any character not a ^S or ^C is stored in the buffer }

var ctrls, ctrlc : boolean; { are these active? }
procedure UCtrlSC; { FORWARD }
var ch: char; flag : boolean;
begin
   monitor;
   while ukeypressed and (not eus) do
   begin
      flag := false;
      ugetchardir(ch);
      if (ch in [^S]) and ctrls then
      begin
         repeat
            monitor
         until (Ukeypressed) or (EUS)
      end
      else
      begin
         if (ch in [Esc,^C,^K,^X]+ValidSet) and ctrlc then
         begin
            buffhead := bufftail;        { Clear buffer when interrupted }
            UInterupted := true;
            if ch in validset then flag := true; { this is a valid command, add to buff }
         end
         else
            flag := true; { this char is not recognised, just add to buff }
      end;
      if flag then
      begin
         if (succ(bufftail) mod bufflen) <> buffhead then { only save if room in buff }
         begin
            iobuff[bufftail] := ch;
            bufftail := succ(bufftail) mod bufflen;
         end
      end
   end
end;


{
1234
cdab

head 3
tail 3

never let buffer fill because you cannot then tell if buffer is full or empty

adding a char at tail : inc tail

removing a char from head : inc head

if head = tail then empty or full
}

procedure uservebuff;
var ts,tc:boolean;
begin
   ts:=ctrls; tc:=ctrlc;
   UCtrlSC;
   ctrls:=ts; ctrlc:=tc
end;

procedure UWrite( Line:st128 ); FORWARD;

procedure Ugetchar( var ch:char );
const LineIdleLimit = 50; { in 10ths of a second, 5 minute time out }
      warning = 45;
var ms, sec2 : integer;
begin
   if not eus then
   begin
      if bufftail <> buffhead then { not empty }
      begin
         ch := iobuff[buffhead];
         buffhead := succ(buffhead) mod bufflen
      end
      else
      begin
         ms:=0; sec2:=0;
         repeat
            delay(1);
            ms := succ(ms);
            if ms = 2000 then
            begin
               ms := 0;
               inc(sec2);

               { If Line idle trigger ExitUserSection }
               if sec2 > LineIdleLimit then EUS := True;
               if sec2 = warning then Uwrite(cr+lf+'PLEASE RESPOND:'+cr+lf+bell)

            end;
            uservebuff;   { look out for a char }
            monitor
         until (bufftail <> buffhead) or   { something in the buffer }
               (eus);
         if not eus then ugetchar( ch )
      end;
   end
end;


procedure uwritechar(ch:char);
begin
  UCtrlSC;
  if userlocation = remote then
  begin
     modemwritechar(ch);
     if echoon then write(ch)
  end
  else
     write(ch);
  UCtrlSC;
  monitor
end;

procedure UWrite; {( Line:st128 ); FORWARD;}
var lineLen : byte absolute line;
begin
   if not EUS then
   begin
      i:=1;
      while (i <= lineLen) do
      begin
         uwritechar(line[I]); i := succ(i)
      end;
      if logeverything then
         if line <> nul then write(TempLog,Line)
   end
end;

var paginate : boolean;
{ user write Line }
procedure UWriteln( Line:st128 );
var i : byte;
begin
   UWrite(Line+CR+lf);
   for i := 1 to user.nuls do uwrite(nul);
   inc(linecount);
   if paginate then
      if linecount = user.consolehight-1 then
         Uinterupted := (AskYN('More','Y') = 'N');
end;


procedure Ureadln( var Line:st128 );
var ch: char; i:integer;
    T:ST3;
begin

   linecount := 0;
   i:=0;
   ctrls := false; ctrlc := false;
   repeat
      ugetchar(ch);
      if ch in [' '..'~'] then
      begin
         uwritechar(ch);
         i:=succ(i);
         line[i]:=ch;
      end
      else
         if ch in [bs,del] then
         begin
            if i>0 then
            begin
               uwrite(bs+' '+bs);
               i:=pred(i);
            end
         end
         else
            if ch = cr then
               uwriteln('')
            else
               if ch in [CAN,NAK] then
               begin
                  uwriteln('\');
                  i:=0;
               end;
       monitor
   until (ch= cr ) or (i>126) or (eus);
   line[0]:=chr(i);
   ctrls := true; ctrlc := true;

(*;WRITELN('>',LINE,'<');*)
end; { ureadln }



procedure IOInit;
begin
   uinterupted := false;
   NotTermControl := true;
   Parallel := false;
   ctrls := true; ctrlc := true;
   buffhead := 0; bufftail := 0;
   validset := [];
   if not (returning and modemcarrier) then modeminit
end;
