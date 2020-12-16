{ 16/6/88 }

{ this coontains the monitor, it is called frequently by the usersection and
  so serves to provide all the "background" functions }

{ declared FORWARD }

{ ESU = ExitUserSection }
{ var EUS : boolean; } { very important that this is set false before Usersection }

var
    jiffy,             { small moment }
    seconds,           { seconds }

    SecMin : integer;  { adjusting factor }

{    CursorMoved : boolean;}

procedure tick;
var h,m,err,err2 : integer; hour,min : st2; pd : st15;
begin
   jiffy := succ(jiffy); if insession then jiffy := succ(jiffy);
   if jiffy > SecMin then
   begin (* X := MEM[$E030]; { click }*)
      if (not insession) or
         (insession and (not echoon) and nottermControl and (userlocation = remote)) then
      begin
         if (OldInSession <> InSession) or
            (OldCarrier <> ModemCarrier) or
            (OldHookCarrier <> (OffHook or ModemCarrier)) or
            (OldDate <> PresentDate) then
            displaystatusline;
      end
      else
         cursormoved:=false; {didn't move it this tick}
      jiffy := 0;
      seconds := Seconds + 1;
      if seconds >= 60 then { one min. }
      begin
         seconds := 0;
         if (not clock) and (copy(presentdate,11,5) <> '     ') then
         begin
            val(copy(presentdate,11,2),h,err);
            val(copy(presentdate,14,2),m,err2);
            if err+err2 = 0 then
            begin
               h := (h*60)+m+1;
               if h < 1440 then
               begin
                  str(h div 60,hour);
                  str(h mod 60,min);

                  if length(hour) = 1 then hour := '0'+hour;
                  if length(min) = 1 then min := '0'+min;
                  pd := '          '+hour+':'+min;

                  for h := 11 to 15 do
                    presentdate[h] := pd[h]
               end
            end
         end
      end
   end
end;

procedure monitor; { declared FORWARD }
begin
   tick;
   if not eus then
   begin
      if (userlocation = remote) and (not carrier) then EUS := true;

      if userlocation = remote then
         SysopInterface; { don't want Cursor flashing around if local or spymode}

      if sysopcommand <> none then processsysopcommand
   end
end; { monitor }

procedure preparemonitor; { declared FORWARD }
begin
   EUS := false; { exit user section }
   if (userlocation = remote) and (not echoon) then setupSysopInterface;
   seconds := 0; jiffy := 0;
   SecMin := round( 1000 * (timeadjust/100));
   cursorMoved := true;
end;
