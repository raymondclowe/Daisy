{ 22/6/88 }

{ begin module ask }
{ var seperatorset : set of char
      askstrip     : boolean }
{if askstrip is true then all leading and trailing spaces in the read line}
{will be striped, this is normally true.}
{it is turned off only to enter the subject lines (which may be self-formated)}
{and the "old/new" strings as these may contain spaces ect}
function First(line:st128) : st128;
var i : Integer;
    Result : ^St128;
begin
   new(result);
   Result^ := '';
   monitor;
   if not EUS then
   begin
      I := 1;
      repeat
         if line[I] in SeperatorSet then
            Result^ := COPY( line, 1 , I - 1);
         I := I + 1
      until (I > length( line )) or ( Result^ <> '' );
(*WriteLn('I=(',I,'):LINE=(',LINE,'):Result=(',Result^,')');*)
      if Result^ = '' then Result^ := line
   end;
   First := Result^;
   dispose(result)
end;

function BUTFirst(line:st128) : st128;
var i : Integer;
    Result : ^St128;
    flag : boolean;   { state1: false : scanning for a seperator  : go state2}
                      { state2: true  : scanning for a ~seperator }
begin
   new(result);
   Result^ := '';
   monitor;
   if not EUS then
   begin
      I := 1;
      flag := false;
      repeat
         if line[I] in SeperatorSet then
            flag := true  { scan up to first seperator }
         else
            if flag then Result^ := copy ( line, I, MAXINT );
         I := I + 1
      until (I > length( line)) or (Result^ <> '')
(*;WriteLn('I=(',I,'):LINE=(',LINE,'):Result=(',Result^,')');*)
   end;
   BUTFirst := Result^;
   dispose(result)
end; { but first }

{------------------------------------------------------------}

{ carries tail of input line not yet used }
var inputbuffer : st128;
    Inputbufferlen : byte absolute inputbuffer;

procedure FlushInputBuffer;
begin InputBuffer := '';
(*      buffhead := bufftail;*)
end;

{ Every prompted Input first Checks to see if there is a suitable
answer already waiting in the Input line if there is then it
takes that and returns, if not then it prints the prompt and gets
a line }
function Ask(Prompt:St128; MaxLen:Integer): st128;
var I : byte;
    ch: char;
begin
   monitor;
   if not EUS then
   begin
{ if required input is one char and user wants single key input then
  until a printable is received write the prompt, get a char. }
      if (maxlen=1) and (user.singlekey) then
      begin
         linecount := 0;
         UWrite(Prompt);
         repeat
            UWrite('_'+bs);
            Ugetchar(ch);
            monitor
         until (ch in [cr,^H,^J,^U,^I,^K,' '..'~']) or (eus);
         if ch in [' '..'~'] then uwrite(ch);
         UWriteln(''); linecount := 0;
         if ch = cr then ask := '' else ask := ch
      end
      else
      begin
         if InputBuffer > '' then
            if length(First(InputBuffer)) <= MaxLen then
            begin
               Ask := First(InputBuffer);
               InputBuffer := BUTFIRST(InputBuffer)
            end
            else
            begin
               UWriteLn('Too Long!');
               InputBuffer := '';
               for I := 1 to length(PROMPT)-1 do UWrite(' ');
               UWriteln('|'+copy(dashes,1,maxlen)+'|');
               Ask := Ask(Prompt, MaxLen)
            end
         else
         begin
            UWrite(Prompt);
            UReadLn(InputBuffer);
            if askstrip then
            begin
               while (inputbuffer[1] = ' ') and (inputbufferlen>0) do  { strip leading }
                delete(inputbuffer,1,1);
               while inputbuffer[Inputbufferlen] = ' ' do { strip trailing }
                Inputbufferlen := pred(Inputbufferlen);
            end;
            if InputBuffer = '' then {this allows empty line }
               Ask := ''
            else
               Ask := Ask(Prompt, MaxLen) {recurcive tail-end call}
         end
      end
   end
   else
      ask := ''
end; { Ask }

(* nothing seems to use this! *)
(*function AskChar(Prompt:St128):st1;VAR Result : ST1;begin repeat Result := Ask(PROMPT, 1 );
monitor until (Result > '') or (EUS);if EUS then askchar := ^D else AskChar := Result
end; { Askchar }*)

{ note that it is up to the programmer to ensure a valid default }
function AskYN; { FORWARD }
var YN : st1;
begin
   if default = 'Y' then
      prompt := prompt + '? [Y,n]: '
   else
      prompt := prompt + '? [y,N]: ';
   repeat
      YN := upstring(Ask(prompt,1));
      if YN = '' then YN := default;
      if not (YN in ['Y','N']) then ucurup;
      monitor
   until (YN in ['Y','N']) or (EUS);
   askYN := YN
end;
{ end module ask }
