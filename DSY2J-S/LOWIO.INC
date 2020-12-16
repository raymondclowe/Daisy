{ 16/6/88 }

{---------------------------- Carrier.inc --------------------------}
{ there is no latching with this function, you get the actual Carrier status }
function RawCarrier:boolean;
begin
   RawCarrier := ModemCarrier
end;

{ 1/4/87 }
{ returns Carrier, latched so that once it detects Carrier false it will always
  return Carrier false until Carrierlatch is reset. must reset Carrier latch
  before first use or random things may cause crash (?) }
function Carrier:boolean;
var Result : boolean;
begin
   if not Carrierlatch then { Carrier loss has already happened, not yet reset }
      Result := false
   else
      Result := RawCarrier;
   if not Result then Carrierlatch := false;
   Carrier := Result
end;

{-------------------------- on/off hook -------------------}
procedure goonhook; { dropCarrier }
begin
   if userlocation = remote then
      Modemhangup
   else
      modemonhook;
   offhook := false
end;

