{ 4/11/88 }
{ includes setupsystemOptions and setupfiles }
OVERLAY procedure setup;
label jumplogo;
var f :text;
    l : st80;
    CmdLine: string[127] absolute $80; { CP/M command line tail }
    i : integer;

procedure openLogFile;
var l : st128; lLen : byte absolute l;
    ioerr : integer;
    mainLog : file;
begin
   assign( MainLog, SystemlogFN );
   templogfn := SystemlogFN;
   if pos('.',templogfn) = 0 then templogfn := templogfn + '.';
   delete(templogfn,pos('.',templogfn)+1,3);
   templogfn := templogfn + '$$$';
   assign(templog, templogfn);

   fillchar(l[1],26,128);
   lLen := 128;
   if fileexists( systemlogfn ) then
   begin
      reset(MainLog);
      if filesize(mainlog) > 0 then
      begin
         seek( MainLog, filesize(MainLog) -1);
         blockread(MainLog, l[1], 1 );
         while l[lLen] = sub do
            lLen := pred(lLen)
      end
      else
         l := ''
   end
   else
   begin
      rewrite( MainLog );
      l := '';
   end;
   close(mainlog);
   rewrite( tempLog );
   write( TempLog, L );  { start the file with the last block of the main
                           file so that it may start exactly at the begining
                           of the last block of main file }

   if logeverything then writeln(templog,'* Logging everything');
   if logcallers then writeln(templog,'* Logging callers');
   if logactivity then writeln(templog,'* Logging activity')
end;

procedure SetUpSystemOptions;
const maxnumSyms = 2;
var systemOptions : text;
    Sym : array[1..maxnumSyms] of st40;
    OptionsOK,
    debug     : boolean;
    code      : integer;
    lineprocessed: boolean;

    sysoptfn: st14;
    sysoptfnlen: byte absolute sysoptfn;

   procedure GetLine; { get next Symbol, stored in Sym }
   var Line : st128;
       I,p : integer;
   begin { GetLine }
      repeat
         readln(systemOptions, Line);
         if debug then writeln(line);
         p := pos(';',Line);
         if p > 0 then Line[0] := chr(p-1);
         while pos(' ',Line) > 0 do
            delete(Line,pos(' ',Line),1);
         Line := upstring(Line)
      until (line > '') or (eof(systemoptions));

      fillchar(sym,sizeof(sym),0);
(*for i := 1 to maxnumsyms do Sym[I][0] := #00;*)

      i := 1;
      while (Line > '') and (i<=maxnumSyms) do
      begin
         p := pos('=',line);
         if p > 0 then
         begin
            Sym[i] := copy(Line,1,p-1);
            Line := copy(Line,p+1,maxint);
            inc(i)
         end
         else
         begin
            sym[I] := line;
            line := ''
         end
      end;

      if debug then
         for i := 1 to maxnumsyms do
            writeln('Sym[',I,']=(',Sym[I],')')
   end; { GetLine }



   { it is important to note that the following "valid" functions are capable
     of setting OPTIONSOK to false if they are given an argument that is not
     valid }
   function ValidFN( name : st40 ) : boolean;
   var result : boolean;
       extention : boolean;
       i : integer;
       nameLen : byte absolute name;
   begin
      result := true;
      if not (namelen in [1..14]) then result := false;
      if pos(':',name)=0 then
      begin
         if namelen > 12 then result := false
      end
      else
      begin
         if pos(':',name) <> 2 then result := false;
         if not( name[1] in ['A'..highestdrv] ) then result := false;
         name := copy(name , 3, maxint)
      end;
      extention := false;
      if not (name[1] in ['A'..'Z','-','0'..'9','$']) then result := false;
      i := 2;
      repeat
         while (i <= namelen) and result do
         begin
            if not (name[I] in ['A'..'Z','-','0'..'9','$','.']) then result := false;
            if name[I] = '.' then
               if extention then
                  result := false
               else
                  extention := true;
            i := succ(i)
         end
      until i > namelen;
      if (extention) and
         ((pos('.',name) > 9) or
          (pos('.',name) < (namelen-4))) then result := false;
      if not result then optionsok := false;
      ValidFN := result
   end; { Valid filename }

   function Validboolean(Line:st40):boolean;
   begin
      if (line > '') and (Line[1] in ['F','T']) then
         Validboolean := true
      else
      begin
         Validboolean := false;
         optionsok := false
      end
   end;

   function Validinteger(Line:st40):boolean;
   var temp:integer;
   begin
      val(Line,temp,code);
      if code <> 0 then optionsok := false;
      validinteger := (code=0)
   end;

   function ValidDRIVE(Line:st40):boolean;
   begin
      if (line >'' ) and (line[1] in ['A'..highestdrv]) then
         validdrive := true
      else
      begin
         optionsok := false;
         validinteger := false
      end
   end;

   procedure setdrv(keyword:st40; var drv:char);
   begin
      if Sym[1] = keyword then
      begin
         lineprocessed := true;
         if Validdrive( Sym[2] ) then drv := Sym[2][1]
      end
   end;

   procedure setbool(keyword:st40; var bool:boolean);
   begin
      if Sym[1] = keyword then
      begin
         lineprocessed := true;
         if Validboolean ( Sym[2] ) then bool := (Sym[2][1] = 'T')
      end
   end;

   procedure setint(keyword:st40; var result:integer);
   begin
      if Sym[1] = keyword then
      begin
         lineprocessed := true;
         if ValidINTEGER( Sym[2] ) then VAL(Sym[2], result, code )
      end
   end;

   procedure setFN(keyword:st40; var result:st14);
   begin
      if Sym[1] = keyword then
      begin
         lineprocessed := true;
         if ValidFN( Sym[2] ) then result := Sym[2]
      end;
   end;

{ First defaults are setup, then sysopt.bbs is given a chance to change things
  if there is any error in sysopt.bbs then the program HALTs.
  Note that at the halt point the Options vars may contains illeagle data. If
  the program fails to halt those illeagle entries MUST be changed }
begin { setupsystemOptions }
   { set defults }
   Logeverything := false;
   logcallers := false;
   logactivity := false;
   AllowNewUsers := true;
   AllowYell := false;
   counttimesread := true;
   Quiet := true;
   alwaysexit := false;
   recordnumcalls := true;
   recordlastuser := false;
   cpam := false;
   mailer := false;
   clock := false;
   clockslot := -1;
   userlistFN := 'USERLIST.BBS';
   SystemlogFN:= 'DAISYLOG';
   questionFN:='';
   systemlogoFN := '';
   bulletinFN := '';
   welcomefn := '';
   bulslistfn := '';
   areasfn := 'AREAS.BBS';
   outsidefn := '';
   helpdrv := 'A';
   Generaldrv := 'A';
   eventdrv := 'A';
   highestdrv := 'F';
   ramdrv := ' ';
   nodelistdrv := 'A';
   editorialfn := '';
   thisnet := 1; thisnode := -1;
   altnet1 := 1; altnode1 := -1;
   altnet2 := 1; altnode2 := -1;
   altnet3 := 1; altnode3 := -1;
   altnet4 := 1; altnode4 := -1;
   fidowait := 0;
   sysmaxbaud := 1200;
   Schedular := false;
   areasmpriv := 95;
   kludgepriv := 80;
   newuserpriv := 20;
   outsidepriv := 60;
   setdatepriv := 60;
   sysopmenupriv := 100;
   yellpriv := 30;
   filerpriv := 60;
   filer := false;
   timeadjust := 100;
   filesdrive := ' ';

   areasdrv := 'A'; { default passing peram }

   sysoptfn := upstring(copy(cmdline,2,14));  {command line}
   while sysoptfn[sysoptfnlen] = ' ' do
      sysoptfnlen := pred(sysoptfnlen);
   if pos('/',sysoptfn) > 0 then sysoptfnlen := pos('/',sysoptfn)-1;

   if sysoptfn = '' then {no command line tail}
   begin
      sysoptfn := 'SYSOPT.BBS'; {on default drive}
      if not fileexists(sysoptfn) then sysoptfn := 'A:' + sysoptfn; {on a: drive}
      if not fileexists(sysoptfn) then sysoptfn := '' {use defaults}
   end;

   if sysoptfn > '' then
      if fileexists( sysoptfn ) then
      begin
         ASSIGN(systemOptions,sysoptfn);
         RESET( systemOptions );
         OptionsOK := true;
         debug := false;
         repeat
            GetLine;

lineprocessed := false;

if Sym[1] = 'USERLISTDRIVE' then
begin
   lineprocessed := true;
   if Sym[2] > '' then
      if Validdrive( Sym[2] ) then userlistFN := Sym[2]+':USERLIST.BBS'
end;

if Sym[1] = 'BULLETINLISTDRIVE' then
begin
   lineprocessed := true;
   if Sym[2] > '' then
      if Validdrive( Sym[2] ) then bulslistFN := Sym[2]+':BULSLIST.BBS'
end;

if Sym[1] = 'AREASDRIVE' then
begin
   lineprocessed := true;
   if Sym[2] > '' then
      if Validdrive( Sym[2] ) then
      begin
         areasFN := Sym[2]+':AREAS.BBS';
         areasdrv := sym[2]
      end
end;

setdrv('HELPDRIVE',helpdrv);
setdrv('GENERALDRIVE',generaldrv);
setdrv('EVENTDRIVE',eventdrv);
setdrv('HIGHESTDRIVE',highestdrv);
setdrv('RAMDRIVE',ramdrv);
setdrv('NODELISTDRIVE',nodelistdrv);
setdrv('FILESDRIVE',filesdrive);

{ in this and the next section it is vital to avoid calling validfn if Sym[2]
  is nul }

setfn('SYSTEMLOGOFILENAME',systemlogoFN);
setfn('BULLETINFILENAME',bulletinFN);
setfn('WELCOMEFILENAME',WELCOMEfn);
setfn('OUTSIDEFILENAME',outsidefn);
setfn('EDITORIALFILENAME',editorialFN);
setfn('SYSTEMLOGFILENAME',systemlogFN);
setfn('QUESTIONFILENAME',questionFN);

setbool('LOGEVERYTHING',logeverything);
setbool('LOGCALLERS',logcallers);
setbool('LOGACTIVITY',logActivity);
setbool('ALLOWNEWUSERS',allownewusers);
setbool('ALLOWYELL',allowyell);
setbool('COUNTTIMESREAD',counttimesread);
setbool('QUIET',quiet);
setbool('ALWAYSEXIT',alwaysexit);
setbool('RECORDNUMBEROFCALLS',recordnumcalls);
setbool('RECORDLASTUSER',recordlastuser);
setbool('CP/AM',cpam);
setbool('MAILER',mailer);
setbool('SCHEDULAR',schedular );

if Sym[1] = 'CLOCK' then
begin
   lineprocessed := true;
   if (Sym[2] > '') and (Validinteger( Sym[2] )) then
   begin
      clock := true;
      VAL(Sym[2], clockslot, code )
   end;
end;

if Sym[1] = 'FILERPRIVILAGELEVEL' then
begin
   lineprocessed := true;
   if (Sym[2] > '') and (Validinteger( Sym[2] )) then
   begin
      filer := true;
      VAL(Sym[2], filerpriv, code )
   end;
end;

setint('NET', thisnet );
setint('NODE', thisnode );
setint('ALTERNATENET1', altnet1 );
setint('ALTERNATENODE1', altnode1 );
setint('ALTERNATENET2', altnet2 );
setint('ALTERNATENODE2', altnode2 );
setint('ALTERNATENET3', altnet3 );
setint('ALTERNATENODE3', altnode3 );
setint('ALTERNATENET4', altnet4 );
setint('ALTERNATENODE4', altnode4 );
setint('FIDOWAIT',fidowait );
setint('SYSMAXBAUD',sysmaxbaud );
setint('AREASMAINTPRIVILAGELEVEL', areasmpriv );
setint('NEWUSERPRIVILAGELEVEL',newuserpriv );
setint('OUTSIDEPRIVILAGELEVEL',outsidepriv );
setint('SETDATEPRIVILAGELEVEL',SetDatePriv );
setint('SYSOPMENUPRIVILAGELEVEL',sysopmenupriv);
setint('YELLPRIVILAGELEVEL', Yellpriv);
setint('VIEWKLUDGEPRIVILAGELEVEL', kludgepriv);
setint('TIMEADJUST', timeadjust);

if Sym[1] = 'DEBUG' then
begin
   lineprocessed := true;
   debug := not debug
end;

if Sym[1] = 'END' then lineprocessed := true;

if not lineprocessed then optionsok := false

         until (Sym[1] = 'END') or eof( systemOptions ) or (not optionsok);
         close (systemOptions);
         if not OptionsOK then
         begin
            writeln(BELL,'There is an error in ',sysoptfn,', correct it, or erase it and use defaults');
            halt
         end
      end
      else
      begin
         writeln(BELL,'Can''t find ',sysoptfn);
         halt
      end
end; { set up Options }

procedure setupfiles;

var bul: bultype;
    area : areatype;
    tempdate : st15;
    i:byte;
    f : text;

begin { setupfiles } { as in make them if they don't exist }

   { check for msg files done in msg file opener }

   Assign(userfile,userlistfn);
   {$I-} reset( userfile ); {$I+}
   if ioresult <> 0 then
   begin
      {$I-} REwrite( userfile ); {$I+}
      if IOResult <> 0 then
      begin
         writeln(cerr+UserListFN);
         HALT
      end;
   end;
   close (userfile);

   if bulslistfn > '' then
   begin
      assign(bulsf,bulslistfn);
      if not fileexists(bulslistfn) then
      begin
         {$I-}REwrite( bulsf );{$I+}
         if IOResult <> 0 then
         begin
            writeln(cerr+bulslistfn);
            HALT
         end;
         fillchar(bul,sizeof(bul),0);
         write(bulsf, bul);
         close(bulsf)
      end
   end;

   assign(areasf,areasfn);

   if not fileexists(areasfn) then
   begin
      {$I-}REwrite( areasf );{$I+}
      if IOResult <> 0 then
      begin
         writeln(cerr+areasfn);
         HALT
      end;
      area := nularea; { data block? }
      blockwrite(areasf,area,1);

      Area := nularea; { first, default, message area }
      blockwrite(areasf,area,1);
      close(areasf)
   end;

   assign(f,generaldrv+':SYSDATA.BBS');
   {$i-} reset(f); {$i+}
   if ioresult = 0 then
   begin
      Readln(f,tempdate);
      for i := 1 to 15 do
         presentdate[I] := tempdate[I];
      Readln(F,totalcalls);
      close(f)
   end
   else
   begin
      {$I-}REwrite( f );{$I+}
      if IOResult <> 0 then
      begin
         writeln(cerr+generaldrv+':SYSDATA.BBS');
         HALT
      end;
      writeln(f, '               ');
      writeln(F, 0);
      close(f)
   end;

end; { setupfiles }

procedure setupnextevent;
var f:text;
    nextevent:st6;
    i : byte;
begin           {hhmmdd}
   nextevent := '000000';
   if fileexists('NXTEVNT.BBS') then
   begin
      assign(f,'NXTEVNT.BBS');
      reset(f);
      readln(f,nextevent);
      close(f);
      for i := 1 to 4 do
         NextEventTime[I] := NextEvent[i];
      for i := 1 to 2 do
         NextEventDay[I] := NextEvent[i+4]
   end
end; { set up next event }

begin { setup }

   i := bdos(25);
   bdos(13);
   bdos(14,i);

   if pos('/R',cmdline)>0 then
   begin
      returning := true;
      writeln;
      writeln('Returning to Daisy');
      goto jumplogo
   end
   else
      returning := false;

   clrscr;

gotoxy(1,3);
writeln('DDD       AA      III     SSSSS    Y      Y     III III':67);
writeln('D  D     A  A      I     S     S    Y    Y       I   I':66);
writeln('D   D    A  A      I      SSS        Y  Y        I   I':66);
writeln('D   D    AAAA      I        SSS       YY         I   I':66);
writeln('D  D    A    A     I     S     S      YY         I   I':66);
writeln('DDD    A      A   III     SSSSS       YY        III III':67);
writeln;
writeln;
writeln(version:38+(length(version) div 2));
writeln;
writeln('by':39);
writeln;
writeln('Raymond Lowe and Ken Lo':49);
writeln('Copyright (C) 1987,1988':49);
writeln('All Rights Reserved':46);
writeln;

jumplogo:

   modemcheck;

   if pos('/F',cmdline)=0 then setupsystemOptions; { FAST! }

   if not fileexists(eventdrv+':EVENT.D') then schedular := false;
   if not fileexists(eventdrv+':EVENT.A') then mailer := false;
   if not fileexists(eventdrv+':EVENT.C') then filer := false;

   setupfiles;
   if Logeverything or logcallers or logactivity then openLogFile;

   if schedular then SetUpNextEvent;

   externalevent := nul;
   assign(f,'EVENT.$$1');
   {$i-} reset(f); {$i+}
   if ioresult = 0 then
   begin
      read(f,externalevent);
      close(f);
      erase(f);
      if not (externalevent in ['0'..'9','A'..'Z']) then externalevent := nul
   end;

   IOInit;
   echoon := false;

   fillchar(dashes,80,45);

   position := '1'; { initial sysop cursor position }

   user.name := '';

   exitbbs := alwaysexit; { might be true, cause exit at once, or after session }

   clrscr;
   MUSSED := TRUE;
   newmailmsg := false;
   SysopCommand := none;
   sysopperam2 := ' ';
   preparemonitor;

end; { setup }
