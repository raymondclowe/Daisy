Function date2str(datestr: st9):st6;
{ accept a string of type date and return a string YYMMDD for comparision }

const
   month = 'JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC';

var
   ordmth  : string[2];
   chmth : string[3];

begin
   chmth := copy(datestr,4,3);
   chmth := upcase(chmth[1]) + upcase(chmth[2]) + upcase(chmth[3]);
   str((pos(chmth, month)+2) div 3:2, ordmth);
   date2str := copy(datestr,8,2) + ordmth + copy(datestr,1,2);
end;
