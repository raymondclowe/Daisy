{ 3/5/88 }
overlay procedure ViewUserList;
var ViewUser : ^person;
    NameSt   : ^st40;
begin
   new(ViewUser); new(NameSt);
   NameSt^ := upstring( ask('Name or part of name (CR for all): ',40) );
   UWriteln('Wait ...');
   reset( UserFile );
   uinterupted := false;
   while not ((EOF( UserFILE )) or (uinterupted) or (EUS)) do
   begin
      read( UserFile, ViewUser^ ); tick;
      if (ViewUser^.Name[1] <> 'd') and
         ((pos(NameSt^,upstring(ViewUser^.Name)) > 0) or (NameSt^='')) and
         (length(viewuser^.password) > 1) then
         uwriteln(ViewUser^.Name+' '+copy(dashes,1,37-length(ViewUser^.Name))+' '+ViewUser^.lastcalled );
      UCtrlSC;
      monitor
   end;
   close(Userfile);
   dispose(ViewUser); dispose(NameSt);
   Uwriteln('')
end; { ViewUserlist }
