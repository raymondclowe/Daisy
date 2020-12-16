
{ This file must used with MODEM2.INC }

var fnew : st14;
begin
  transfer:= true;
  mode:= trans;
  crcmode:= true;
  case mode of
   0  : begin
          if dir then
          begin
            if sendfile(transfn) <> 0 then transfer:= false;
          end
          else
          begin
            if getfile(transfn) <> 0 then transfer:= false;
          end;
        end;
(**)(*   1  : begin
          if dir then
          begin
            if transfn = '' then
            begin
              show('Last file',3);
              chksum:= modin(100);
              if (not timeout) and (chksum = NAK) then
              begin
                modout(ACK);
                modout(EOT);
              end
              else modout(EOT);
            end
            else
            begin
              show('Sending '+transfn,2);
              if pos(':',transfn) <> 0 then fnew:= copy(transfn,pos(':',transfn)+1,length(transfn))
              else fnew:= transfn;
              sendfn(fnew);
              if timeout then transfer:= false
              else if sendfile(transfn) <> 0 then transfer:= false;
            end
          end
          else
          begin
            repeat
              transfn:= getfname;
              if transfn = '' then timeout:= true;
              if not timeout then
              begin
                transfn:= wrkdrv + ':' + transfn;
                transfn:= checkf(transfn);
                show('Receiving '+transfn,2);
                crcmode:= true;
                if getfile(transfn) <> 0 then transfer:= false;
              end;
            until timeout;
          end;
        end;*)
   2  : begin
          if dir then
          begin
            if transfn = '' then
            begin
(*              show('Last file',3);*)
(**)          chksum:= modin(2500);
              if (not timeout) and (chksum = NAK) then
              begin
                modout(ACK);
(**)            delay(100);
                modout(EOT);
              end
              else modout(EOT);
            end
            else
            begin
(**)(*              writeln('Sending ',transfn);*)
(**)report('Sending '+transfn,2);
              if pos(':',transfn) <> 0 then fnew:= copy(transfn,pos(':',transfn)+1,length(transfn))
              else fnew:= transfn;
              sendfn(fnew);
              if timeout then transfer:= false
              else
              begin
(**)            delay(200); { give other side a chance to get ready }
                sendtel(fnew);
                if (not timeout) and (carrier) then
                 if sendfile(transfn) <> 0 then transfer:= false;
              end;
            end
          end
          else
          begin
            repeat
              transfn:= getfname;
              if transfn = '' then timeout:= true;
              if not timeout then
              begin
                transfn:= CFG.FileattachDrv + ':' + transfn;
                transfn:= checkf(transfn);
(**)(*                writeln('Receiving ',transfn);*)
(**)report('Receiving '+transfn,2);
                crcmode:= true;
                if getfile(transfn) <> 0 then transfer:= false;
              end;
            until timeout;
          end;
        end;
  end;
end;
