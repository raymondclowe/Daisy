{ 23/11/88 }

overlay procedure readperams;

var PassedStr : st80;
    PS : st80 absolute passedstr;

    SymP : integer;
    key : st2;

{function hextodec ( str : st4 ) : integer;
var result,digit : integer;
    i : byte;
    err:integer;
begin
   result := 0;
   for i := 1 to 4 do
   begin
      str[I] := upcase(str[I]);

      digit := (pos(str[I],'0123456789ABCDEF')-1) ;
      if digit = -1 then
      begin
         writeln(str,' is not a hex number');
         halt
      end;
      case I of
        4 : result := result + digit;
        3 : result := result + (digit * 16);
        2 : result := result + (digit * 256);
        1 : result := result + (digit * 4096);
      end;

   end;

(*   writeln(str,' = ',result);*)
   hextodec := result;
end;}

function strng(len:integer):st80;
var result: st80; j : integer;
begin
   result := '';
   j := 1;
   repeat
      result := result + PS[SymP];
      inc(SymP); inc(j)
   until (j > len) or
         (PS[SymP] in ['/','#']);
   if (len = 1) and (result='') then {oops, 1= char}
      result := ' ';
(*   writeln('strng(',len,') = ',result);*)
   strng:=result;
end;
function character:char;
var result : char;
begin
   result := PS[SymP];
   inc(SymP);
(*   writeln('character = ',result);*)
   character := result
end;
function num:integer;
var temp: st80; result: integer; code : integer;
begin
   temp := '';
   result := 0;
   while (ps[Symp] in ['0'..'9','-','+']) do
   begin
      temp := temp + PS[SymP];
      inc(SymP)
   end;
   val(temp,result,code);
   if code <> 0 then result := 0;
(*   writeln('num = ',result);*)
   num:=result;
end;
function truth:boolean;
var result : boolean;
begin
   result := character in ['1','Y','T'];
(*   writeln('truth = ',result);*)
   truth := result;
end;


begin
   if sizeof(config) > 256 then writeln('Config size problem');
   config := defaultconfig;
   passedstr := '';
   for SymP := 1 to 80 do {19 used }
      PassedStr := PassedStr + upcase(passing[SymP]);
   while pos(' ',PassedStr) > 0 do                  { strip spaces }
      delete(PassedStr,pos(' ',PassedStr),1);
   if pos(';',PassedStr) > 0 then                  { blank comments }
      for SymP := pos(';',PassedStr) to 80 do
         passedstr[SymP] := nul;

   if PassedStr[1] in ['/','#'] then { new style }
   begin

      SymP := 1;
      repeat
         case character of
           '/' : begin
                    key := strng(2);
                    if key = 'CF' then
                    begin
                       assign(configf,'CONFIG.MLR');
                       reset(configf);
                       seek(configf,num*2);
                       blockread(configf,configB,2);
                       close(configf);
                    end;
                    if key = 'BS' then
                    begin
                       CFG.BossNet := num; inc(SymP);
                       CFG.BossNode := num;
                    end;
                    if key = 'FW' then CFG.DoFullWindow := truth;
                    if key = 'IF' then CFG.IgnoreFiles := truth;
                    if key = 'WW' then CFG.WindowLength := num;
                    if key = 'WD' then CFG.WorkDrv := character;
                    if key = 'FD' then CFG.FileAttachDrv := character;
                    if key = 'ND' then CFG.nodelstdrv := character;
                    if key = 'ET' then CFG.DoTossMail := truth;
                    if key = 'ES' then CFG.DoScanMail := truth;
                    if key = 'AH' then CFG.AllowHuman := truth;
                    if key = 'WC' then CFG.WaitCarrierDelay := num;
                 end;
           '#' : begin
                    CFG.PrimaryEventCode := character;
                    if PS[SymP] in validEventCodes then
                       CFG.SecondaryEventCode := character;
                 end
         end;
      until (SymP > 80) or (symp > length(PS));

   end
   else
   begin { old style }
      writeln('Old style perameters not supported!'); halt;
(*
      { peramaters }
      CFG.PrimaryEventCode := passedstr[1];
      CFG.nodelstdrv := passedstr[10];
      CFG.workdrv := passedstr[11];

      CFG.BossNode := hextodec(PassedStr[2]+PassedStr[3]+PassedStr[4]+PassedStr[5]);
      CFG.BossNet := hextodec(PassedStr[6]+PassedStr[7]+PassedStr[8]+PassedStr[9]);
      CFG.WindowLength := hextodec(PassedStr[12]+PassedStr[13]+PassedStr[14]+PassedStr[15]);
      CFG.dofullwindow := (PassedStr[16] = '1');

      CFG.DoTossMail := false; CFG.DoScanMail := false;
      if PassedStr[17] in ['1','3'] then CFG.DoTossMail := true;
      if PassedStr[17] in ['2','3'] then CFG.DoScanMail := true;

      CFG.SecondaryEventCode := PassedStr[18];
      CFG.Ignorefiles := (PassedStr[19] = '1');
*)
   end;
   if not (CFG.NodelstDrv in ['A'..highestdrv]) then
      CFG.NodelstDrv := BBSNodelistDrv;
   if not (CFG.workdrv in ['A'..highestdrv]) then
   begin
      if ramdrv <> ' ' then
         CFG.workdrv := ramdrv
      else
         CFG.workdrv := CurrentDrive;
   end;
   if not (CFG.FileAttachDrv in ['A'..highestdrv]) then
      CFG.FileAttachDrv := CFG.WorkDrv;
   CFG.maxbaud := sysmaxbaud
end;


