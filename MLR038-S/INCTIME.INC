{ 21/8/88 }

overlay procedure inctime;
const mname = 'JanFebMarAprMayJunJulAugSepOctNovDec';
type mlen = array [0..12] of integer;
const monthday : mlen = (1,31,28,31,30,31,30,31,31,30,31,30,31);

function zeroed(n : integer): st2;
var l: st2; lenl: byte absolute l;
begin
   str(n,l);
   if lenl = 1 then zeroed := '0'+l else zeroed := l;
end;


begin
   nowseconds := nowSeconds + (elapsedtime - lastUpdated);
   { handle overuns }
   while nowseconds > 59 do
   begin
      inc(nowmin);
      nowseconds := nowseconds - 60
   end;
   while nowmin > 59 do
   begin
      inc(nowhour);
      nowmin  := nowmin - 60
   end;
   while nowhour > 23 do
   begin
      inc(todayday);
      nowhour  := nowhour - 24
   end;
   if todayday > monthday[todaymonth] then
   begin
      inc(todaymonth);
      todayday := 1;
   end;
   if todaymonth > 12 then
   begin
      inc(todayyear);
      todaymonth := 1;
   end;
   datestr := zeroed(todayday) + ' ' + copy(mname,(todaymonth*3)-2,3) +
              ' ' + zeroed(todayyear) + ' ' + zeroed(nowhour) + ':' + zeroed(nowmin);
   lastupdated := elapsedtime;
end;
