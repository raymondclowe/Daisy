{ 23/9/88 }

{----------- user terminal controls -----------}
const dumb = 0; soroc= 1; ansi = 2; datamedia=3;

var oe : boolean;

procedure uclrscr;
begin
  NotTermControl := false;
  oe := echoon; echoon := false;
  case user.consoletype of
   soroc : begin uwrite(^['*'); delay(400) end;
   ansi  : uwrite(^['[2J');
   datamedia : begin uwrite(^L); delay(400) end
  end;
  if user.consoletype <> dumb then linecount := 0;
  NotTermControl := true;
  echoon := oe
end;

procedure ugotoxy(x,y:integer);
begin
  NotTermControl := false;
   oe := echoon; echoon := false;
   case user.consoletype of
    soroc : begin uwrite(^['='); uwrite(chr(y+32));uwrite(chr(x+32));delay(6) end;
    ansi  : begin
               uwrite(^['[');
               str(y+1,uwtemp);
               uwrite(uwtemp+';');
               str(x+1,uwtemp);
               uwrite(uwtemp+'H');
            end;
    datamedia : begin uwrite(^^); uwrite(chr(x+32)); uwrite(chr(y+32)); delay(6) end
   end;
   if user.consoletype <> dumb then linecount := y;
  NotTermControl := true;
   echoon := oe
end;

procedure uhome;
begin
  NotTermControl := false;
   oe := echoon; echoon := false;
   case user.consoletype of
    soroc : begin uwrite(^^); delay(2) end;
    ansi  : uwrite(^['[0;0H');
    datamedia : begin uwrite(^Y); delay(2) end
   end;
   if user.consoletype <> dumb then Linecount := 0;
  NotTermControl := true;
   echoon := oe
end;

procedure uclreol; {forward}
begin
  NotTermControl := false;
   oe := echoon; echoon := false;
   case user.consoletype of
    soroc : begin uwrite(^['T'); delay(20) end;
    ansi  : uwrite(^['[K');
    datamedia : begin uwrite(^]); delay(20) end
   end;
  NotTermControl := true;
   echoon := oe
end;

procedure ucurup; { forward }
begin
  NotTermControl := false;
   oe := echoon; echoon := false;
   case user.consoletype of
    soroc : begin uwrite(^K); delay(2) end;
    ansi  : uwrite(^['[1A');
    datamedia : begin uwrite(^_); delay(2) end
   end;
   if user.consoletype <> dumb then Linecount := Linecount -1;
  NotTermControl := true;
   echoon := oe
end;

procedure upressenter;
var l: st1;
begin
   uwriteln('');
   l:=ask('Press ENTER to continue',1);
(*   linecount := 0;
   repeat
      ugetchar(ch);
      monitor;
   until (ch = cr) or (eus);
   uwriteln('')*)
end;

