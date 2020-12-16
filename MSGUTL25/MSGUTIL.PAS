{Copyright (C) 10/8/1988 by Raymond Lowe}

{ End address : $C940 }
{$B+} {default + i/o mode selection}
{$C+} {default + control s and c}
{$I+} {default + i/o error handling}
{$R+} {default - index range Check}
{$V-} {default + var paramenter type Checking}
{$U-} {default - User interrUpt}
{$A+} {default + absolute code}
{$W0} {default 2 nesting of with statements}
{$X+} {default + array optimization}


Program MessageUtility;

const version = 'Version 2.5';
      CHN = true;

{ things for passing }
{} var passing : array [1..80] of char;
{}     Dashes  : array [1..80] of char absolute passing;
{}
{}     areasdrv,
{}     generaldrv: char;
{}     thisnet, thisnode : integer;
{}     baudrate : integer;
{}     quiet,
{}     clock: boolean;
{}     clockslot:integer;
{}     nodelistdrv:char;
{}

type St255 = String [255];
     St128 = String [128];
     St80  = String [80];
     ST73  = STRING [73];
     St50  = String [50];
     St40  = String [40];
     St36  = String [36];
     St20  = String [20];
     st15  = string [15];
     st14  = string [14];
     st12  = string [12];
     St10  = String [10];
     st6   = string [6];
     St4   = String [4];
     st3   = String [3];
     st2   = String [2];
     St1   = String [1];


{ system options }
var Logeverything,
    logcallers,
    logActivity,
    AllowNewUsers,
    allowyell,
    counttimesread,
    alwaysexit,
    recordnumcalls,
    recordlastuser,
    cpam,
    mailer,
    filer,
    schedular : boolean;
    { FN = filename }
    userlistFN :st14;
    SystemlogFN:st14;
    systemlogofn,
    BulletinFn,
    Bulslistfn,
    areasfn,
    editorialfn,
    outsidefn:st14;
    helpdrv,
    eventdrv,
    highestdrv,
    ramdrv:char;

{ *********** end things for passing **************** }


const CR = ^M;
      LF = ^J;
      BS = ^H;
      BELL = ^G;
      ESC= ^[;
      DEL= #127;
      tsynch= #174;
      yoohoo= #$f1;
      oerr:st14 = 'Error opening ';
      cerr:st15 = 'Error creating ';

{$i script.inc }

{$i utilstuf.inc }

{$i clearsnt.inc }

{$i packmsgs.inc }

{$i killmsgs.inc }

var option : st1;

Begin { Message Utility }
   clrscr;
   writeln('Daisy Utility ',version);
   writeln('By Raymond Lowe, Simmons Kwan - 1988,1989');
   writeln;
   if chn then
      Writeln('Chain version')
   else
   begin
      writeln('Stand alone version');
      writeln('Not implimented');
      halt
   end;
   writeln;

   assign(areasf,areasfn);
   preparescript;

   repeat
      writeln;
      writeln('Main Menu');
      writeln;
      writeln('1)  Kill messages');
      writeln;
      writeln('2)  Pack message area');
      writeln;
      writeln('3)  List areas');
      writeln;
      writeln('4)  Set echo high water mark (HWM)');
      writeln;

      write('Your Choice or Q to quit: ');
      readln(option);
      if option = '' then fillchar(option,sizeof(option),0);
      option := upcase(option[1]);

      case option[1] of
         '1' : KillMsgs;
         '2' : packmsgs;
         '3' : listareas;
         '4' : clearsent;
      end;

   until (option = 'Q');

   if scripting <> none then close(scriptf);
   if keypressed then read(kbd,option); { clear }
End.