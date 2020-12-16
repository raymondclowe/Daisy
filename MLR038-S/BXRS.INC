{ 23/12/88 }

var xrok:boolean;
procedure xr(fn:st14); { receive xmodem }
type fp = file;
var i : integer;
    stri:st4;
    f : ^fp;
begin
   if modcarrier then xrOK := transfer(false,0,fn);
   if not modcarrier then xrok := false;
   if not fileexists(fn) then xrok := false;
   if xrok then
   begin
      new(f);
      assign(f^,fn);
      reset(f^);
      if filesize(f^) = 0 then xrok := false;
      close(f^);
      if not xrok then erase(f^);
      dispose(f)
   end;
end;

var xSok : boolean;
procedure xs(fn:st14);
var stri:st4;
begin
   writeln;
   if (ModCarrier) and (fileexists(fn) ) then
   begin
      xsok := transfer(true,0,fn);
      if not xsok then show('Transfer failed',4);
   end;
   if not modcarrier then
   begin
      xsok := false;
      show('Lost carrier',4);
   end;
   if not fileexists(fn) then xsok := false;
end;

{ 9/9/87 }

var brok:boolean;
procedure br; { receive batch }
begin
   brOK := transfer(false,2,'');
end;

{ 12/7/88 }

var bSok : boolean;
procedure bs(fn:st14);
var t : integer;
    files:text;
    outfile:st14;
begin
   bsok := true;
   delay(1000);
   if fileExists(fn) then
   begin
      assign(files,fn);
      reset(files);
      while (not eof(files)) and (bsok) and (modcarrier) do
      begin
         readln(files,outfile);
         if fileexists(outfile) then
         begin
            bsOK := transfer(true,2,outfile);
         end
      end;
      close(files);
   end;
   if not modcarrier then bsok := false;
   if bsok then bsOK := transfer(true,2,''); { eoT }
end;
