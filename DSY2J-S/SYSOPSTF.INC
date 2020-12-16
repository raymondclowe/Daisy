{ 11/7/88 }

{ Sysop stuff }
{ has to have compiler directives c- and u- if the Sysop interface is to work
  correctly }
var SysopCommand : (none, StartSession, dispUserinfo, ToggleSpy,
                    event, toggleAllowYell,editUserlist, Terminalmode,Exit);
    SysopPeram1  : (r,L);  { Remote, local }
    sysopPeram2  : char;

{var justHadCall : boolean;}

var Position : Char;  { On screen cursor Position }
procedure WriteOption(Option:char); FORWARD;

procedure eraseCursor; FORWARD;
procedure putCursor; FORWARD;

{var InSession : boolean;       in bbs.pas }

{ Command to Start Session ::= StartSession r|L }
Procedure ProcessSysopCommand;
var ch:char;
begin
   if (not insession) or
      (insession and (not echoon) and (userlocation = remote)) then
      displaystatusline;

                   {can't Start a Session if one is already going }
   if (SysopCommand = StartSession) and (not InSession) then
   begin
      erasecursor;
      InSession := TRUE;
      offhook := true;
      if SysopPeram1 = L then
      begin
         clrscr;
         UserLocation := Local; { only switch to Local if you are sure a Local sesssion was called }
         modemoffhook;
         displaystatusline;
         gotoxy(1,2);writeln('>>',copy(dashes,1,30),' Local Session ',copy(dashes,1,30),'<<');
         preparemonitor
      end
      else { there is carrier, must have had phone pickup so must have had Ring }
      begin
         UserLocation := Remote;
         CarrierLatch := true; { you have just Logged on, the Carrier is true }
         displaystatusline;
         if EchoON then
         begin
            gotoxy(1,2); write(esc+'Y');
            writeln('Use the <ESC> key to return to the Sysop menu')
         end;
         preparemonitor
      end;

      callUserSection;

   end;

   if SysopCommand = dispUserinfo then
   begin
      GOTOXY(1,15);WRITE(ESC,'Y');
      gotoxy(1,17);
      if user.name > '' then
      begin
         if insession then
            write('User is ',user.name)
         else
            write('Last user was ',user.name);
         writeln;
         write('Privilage level : ',user.priv)
      end
   end;

   if SysopCommand = ToggleSpy then
   begin { this command never used while spying, spymode changes echoon directly }
      EchoON := not EchoON;
      writeOption('3');
      if (EchoON) and (InSession) then
      begin
         clrscr; MUSSED := TRUE;
         writeln('Use <ESC> to return to the Sysop menu')
      end
   end;

   if SysopCommand = event then
   begin
      GOTOXY(1,15);WRITE(ESC,'Y');
      if not insession then
      begin
         gotoxy(1,17);write('Select event (A..Z,0..9) or <Esc>');
         if sysopPeram2 in ['A'..'Z','0'..'9'] then
         begin
            ch := sysopPeram2;
            sysopperam2:=' '
         end
         else
            repeat read(kbd,ch); ch:=upcase(ch) until ch in ['A'..'Z','0'..'9',Esc];
         gotoxy(1,15);write(ESC,'Y');gotoxy(1,17);

         if ch <> esc then
         begin
            if fileexists(eventdrv+':EVENT.'+ch) then
            begin
               write('Starting event ',ch,', please wait ...');
               eventtag := ch;
               submit(eventdrv+':EVENT.'+ch)
            end
            else
               write('No file '+eventdrv+':EVENT.'+ch)
         end
      end
   end;

   if SysopCommand = toggleAllowYell then
   begin
      allowyell := not allowyell;
      writeOption('5')
   end;

   if (SysopCommand = editUserlist) and (not insession)  then
   begin
      erasecursor;
      CALLuserlisteditor;
   end;

   if (SysopCommand = terminalmode) and (not insession) then
      callterm;

   if SysopCommand = exit then
   begin
      gotoxy(1,15);write(ESC,'Y');
      gotoxy(1,20);
      if not exitbbs then
      begin
         write('Exit');
         if insession then write(' when user logs off');
      end
      else
         write('Don''t exit');
      exitBBS := not ExitBBS;
      writeoption('8')
   end;

  putcursor;
  SysopCommand := none { set it to none having finished Command }
end; { Sysop Command processor }


{ declared FORWARD }
procedure WriteOption;
begin
   if (not EchoON) or (not InSession) then
      case Option of
       '1' : begin gotoxy(12,4); write('Start Local Session') end;
       '2' : begin gotoxy(12,6); write('Display User information') end;
       '3' : begin
              gotoxy(12,8);
              if EchoON then write('De-');
              write('Activate spy mode   ')
             end;
       '4' : begin
              gotoxy(12,10);
              write('Start Event')
             end;
       '5' : begin
              gotoxy(53,4);
              if allowyell then write('Don''t ');
              write('Allow Yell      ')
             end;
       '6' : begin gotoxy(53,6); write('Edit User list') end;
       '7' : begin gotoxy(53,8); write('Terminal mode') end;
       '8' : begin
              gotoXy(53,10);
              if exitbbs then write('Don''t ');
              write('Exit      ')
             end
      end
end;

procedure Positioncursor;
begin
   if (not EchoON) or (not InSession) then
   begin
      case Position of
       '1' : gotoxy(6,4);
       '2' : gotoxy(6,6);
       '3' : gotoxy(6,8);
       '4' : gotoxy(6,10);
       '5' : gotoxy(47,4);
       '6' : gotoxy(47,6);
       '7' : gotoxy(47,8);
       '8' : gotoxy(47,10)
      end;
      cursorMoved:=true;
   end
end;

procedure eraseCursor; { forward }
begin
   if not ((echoOn and inSession) or
      ((not echoOn) and inSession and (userLocation=local))) then
         begin Positioncursor; write('  ') end
end;

procedure PutCursor; { forward }
begin
   if not ((echoOn and inSession) or
      ((not echoOn) and inSession and (userLocation=local))) then
         begin Positioncursor; write('->') end
end;

{ declared FORWARD }

var oldInSession, OldCarrier, OldHookCarrier : boolean;
    OldDate : Date;
procedure displaystatusline;
begin
   gotoxy(9,1); if InSession then write('On-Line') else write('Waiting');
   gotoxy(29,1); if offhook or modemcarrier then write('Off-Hook') else write('On-Hook ');
   gotoxy(46,1); if modemcarrier then write('Carrier') else write('       ');
   readdate;
   gotoxy(62,1); write(presentdate);
   OldInsession := Insession;
   OldCarrier := ModemCarrier;
   OldHookCarrier := (offhook or modemcarrier);
   OldDate := PresentDate;
   Positioncursor
end;

{ Var Mussed : boolean; true if menu has been disturbed, needs rewriting }
procedure setupSysopInterface;
var i : char;
begin
   if keypressed then read(kbd,i); { clear buffer }
   lowvideo;normvideo;
   parallel := false;
   displaystatusline;
   if mussed then
   begin
      gotoxy(1,2);
      writeln('>',copy(dashes,1,77),'<');
      gotoxy(9,4);write('1)');gotoxy(50,4);write('5)');
      gotoxy(9,6);write('2)');gotoxy(50,6);write('6)');
      gotoxy(9,8);write('3)');gotoxy(50,8);write('7)');
      gotoxy(9,10);write('4)');gotoxy(50,10);write('8)');
      gotoxy(1,12);
      writeln('Select Option with cursor or by number');
      writeln('Activate Option with <Return>');
      writeln('>>',copy(dashes,1,75),'<<');

      for i := '1' to '8' do
         WriteOption(I);
   end;
   SysopCommand := none;

   Putcursor;
   mussed := false;
end; { setupSysopinteface }

{ Sysopinterface manages the menu/cursor arangment, it does NO processing,
  it only sets the SysopCommand, it is up to the calling procedure to process
  the Command (by calling SysopCommandprocessor) }
{ Sysop interface may operate in two distinc modes, which mode it is operating
  in is determined by EchoON. When false true a menu driven interface is used
  to generate commands for the Sysopcommandprocessor. When true a single key
  senstive interface allows other commands to be activated, including
  returning to the menu interface }
procedure SysopInterface;
type movetype = array ['1'..'8',1..4] of Char;
const MoveMat:movetype = ( ( ^@,'2','5', ^@),
                           ( ^@,'3','6','1'),
                           ( ^@,'4','7','2'),
{ this makes moving the }  ( ^@, ^@,'8','3'),
{ cursor easier }          ('1','6', ^@, ^@),
                           ('2','7', ^@,'5'),
                           ('3','8', ^@,'6'),
                           ('4', ^@, ^@,'7') );
VAR InputChar : Char;
    Direction : integer;

    YellChar : char;
    exityell : boolean;
    remotcur,
    localcur : integer;

begin { Sysop interface }
   if (keypressed) and
      (not parallel) then  { don't read at this point of local keyboard is }
   begin                   { supposed to be active! }
      read(kbd, Inputchar);
      InputChar := UpCase(InputChar);

      if (EchoON) and (InSession) then
      begin { exit spymode }
         case inputchar of
          'Y', ^Y
              : begin
                   inyell := true;
                   yelldone := true;
                   writeln;Uwriteln('');
                   writeln('Use the <ESC> key to leave Yell');
                   Uwriteln('-------------- Yell at Sysop ----------------');
                   flushinputbuffer;
localcur := 1; remotcur := 1; exityell := false;
repeat
   YellChar := ^@;
   repeat
      if modemkeypressed then
         modemgetchar(YellChar)
      else
         if {console} keypressed then
         begin
            read(kbd,YellChar);
            if yellchar = ESC then exityell := true
         end
   until (YellChar <> ^@) or (not carrier);
   if YellChar in [#8,#10,#13,' '..'~',#127] then
   begin
      modemwritechar(YellChar); inc(remotcur);
      write(yellchar);          inc(localcur);
      if YellChar = CR then
      begin
         modemwritechar(lf);modemwritechar(lf); { double linefeed }
         writeln(lf);
         remotcur := 1; localcur := 1;
      end;
      if YellChar in [' ', ',', '.', '-', '/'] then
      begin
         if remotcur > user.consolewidth - 15 then
         begin
            modemwritechar(CR);modemwritechar(lf);
            remotcur := 1
         end;
         if localcur > localconsolewidth - 15 then
         begin
            writeln; localcur := 1
         end;
      end;
      if YellChar in [#8,#127] then
      begin
         remotcur := remotcur - 2; if remotcur < 1 then remotcur := 1;
         localcur := localcur - 2; if localcur < 1 then localcur := 1;
      end
   end
until (exityell) or (not carrier); {have to check carrier directly because cannot call monito }
                   uwriteln('');
                   UwriteLn('-------- Finished Yelling at Sysop ----------');
                   writeln('Use <ESC> to return to the Sysop menu');
                   flushinputbuffer;
                   inyell := false
                end;
         'P', ^P :
               begin
                  Parallel := true;
                  write('<Parallel ON>');
               end;
         ESC : begin
                  EchoON := false;
                  clrscr; mussed := true;
                  setupSysopinterface
               end
         end {case}
      end { EchoON commands }
      else
      begin { echo off commands/menu/no user stuff }
         if InputChar in ['1'..'8','S','L',',','<',^H,'D',^J,'F',^U,'>','.','R',^I,'E',^K,'[','U',CR] then
            case InputChar of
                '1'..'8','S','L',',','<',^H, 'D',^J, 'F',^U,'>','.','R',^I, 'E',^K,'[',
                'U' : begin
                      case InputChar of
                       'S','L',',','<',^H:Direction := 1;
                       'D',^J:Direction := 2;
                       'F',^U,'>','.','R',^I:Direction := 3;
                       'E',^K,'[','U':Direction := 4
                      end;
                      eraseCursor;
                      if InputChar in ['1'..'8'] then
                         Position := InputChar
                      else
                         if MoveMat [Position,Direction] <> ^@ then
                            Position := MoveMat [Position,Direction];
                      PutCursor
                      end;
            CR  : case Position of
                    '1' : begin SysopCommand := StartSession; SysopPeram1 := L end;
                    '2' : SysopCommand := dispUserinfo;
                    '3' : SysopCommand := ToggleSpy;
                    '4' : SysopCommand := event;
                    '5' : SysopCommand := ToggleAllowYell;
                    '6' : SysopCommand := editUserlist;
                    '7' : SysopCommand := terminalmode;
                    '8' : SysopCommand := exit
                   end {case CR}
            end {case}
      end {echoOn ... }
   end {if keypressed ... }
end; { Sysop interface }

