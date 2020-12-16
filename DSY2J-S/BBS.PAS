{Copyright (C) 4/11/1988 by Raymond Lowe}

{$B+} {default + i/o mode selection}
{$C-} {default + control s and c}
{$I+} {default + i/o error handling}
{$R-} {default - index range Check}
{$V+} {default + var paramenter type Checking}
{$U-} {default - User interrUpt}
{$A+} {default + absolute code}
{$W0} {default 2 nesting of with statements}
{$X+} {default + array optimization}


Program BBS;

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

{$I StrngTyp.Dif }

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
    userlistFN,
    SystemlogFN,
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
    areasmpriv,
    newuserpriv,
    outsidepriv,
    sysopmenupriv,
    yellpriv,
    filerpriv,
    fidowait,
    timeadjust: integer;
    NextEventTime : array[1..4] of char;      { hhmm of next event }
    NextEventDay : array[1..2] of char;      { dd of next event }

    totalcalls :integer;
    altnet1,altnode1: integer;
    altnet2,altnode2: integer;
    altnet3,altnode3: integer;
    altnet4,altnode4: integer;
    filesDrive : char;
    welcomefn:st14;
    eventtag : char;
    setdatepriv:integer;
    templogFN,
    questionFN :st14;
    kludgepriv : integer;
    sysmaxbaud : integer;

{ *********** end things for passing **************** }


const version = 'Version 2j';

const CR = ^M;
      LF = ^J;
      BS = ^H;
      BELL = ^G;
      ESC= ^[;
      NAK= ^U;
      CAN= ^X;
      sub= ^Z;
      DEL= #127;
      nul= #00;
      tsynch= #174;
      yoohoo= #$f1;
      oerr:st14 = 'Error opening ';
      cerr:st15 = 'Error creating ';

{$i bultyp.dif }

{$i areas.dif }
{$I userfile.Dif }
{$i MsgFile.dif }

{$i clock.inc }
var presentdate : date;
procedure readdate;
var t:^st15;
begin
   if clock then
   begin
      new(t);
      t^ := clockdate;
      move(t^[1],presentdate[1],15);
      dispose(t);
(*      for i := 1 to 15 do
         presentdate[I] := t[I]*)
   end
end;


{ carrier }
var CarrierLatch : boolean;

var offhook : boolean;

{ ask/first }
var SeperatorSet : set of char; { note; this can act as a "Perameter" to
                                  chose what is a seperator in any given situation }
    askstrip : boolean;

{ uio }
var UserLocation : (remote,local); { determins if carrier is real ? }

{ general ioresult holder }
var ioerr : integer;

{ main routine }
var exitbbs,
    YELLDONE,
    inyell   : boolean;
    i : integer;
    returning: boolean; { returning from an outside/filer }


procedure inc( var i : integer );
begin i := SUCC(i) end;

procedure dec( var i : integer );
begin i := PRED(i) end;

{ Monitor }
var EUS : boolean; { see file Monitor.Inc }
    CursorMoved:boolean; { set by display status line when it moves cursor }
procedure Monitor; FORWARD;
procedure preparemonitor; FORWARD;

var insession : boolean;

{$I Case.Inc }


var user : person; { the callers data var }
    Usernumber : integer; { the position in the user list of the current user }
    UserChanged : boolean;

procedure displaystatusline; FORWARD;

{ logstuff }
var TempLog : text;


var uinterupted :boolean;
procedure UCtrlSC; FORWARD;

{$i sio10.inc }

{$i lowio.inc }

function askYN(prompt:st128; default:char):st1; FORWARD;

{$I uio3.Inc }

{$I ask.Inc }

{$i uterm.inc }

procedure callusersection; forward;

const localconsolewidth = 79;

procedure calluserlisteditor; FORWARD;

function FileExists(FN:st14):boolean;
type pfile=file;
var f:^pFile;
begin
  new(f);
  assign(f^,FN);
  {$i-} reset(f^); {$i+}
  Fileexists := (ioresult = 0);
  dispose(f);
end;

procedure NotACommand;
begin
   uwriteln('Not a command');
   flushinputbuffer;
end;

procedure submit(fn:st14); FORWARD;


procedure callterm; FORWARD;
var mussed : boolean; { used by setupSysopinterface }

var newmailmsg : boolean;
var f : text;
    externalevent : char;

{$i c:Sysopstf.inc }
{$i Monitor.Inc }

{$i logstuff.inc }

(*************************************************************************)


{$i C:setup.Inc}           { these six in an overlay }

OVERLAY procedure UserSection; {}
label exitusersection,
      JumpLogOnStuff;
var LogOnOK : Boolean;

    maincommandset : set of char;
    mainprompt : st50;

    area : areatype; { present area }

    yellbeep : integer;

{ for interface between read and enter }
var reply : boolean;                { these two vars used to pass the info }
    replymsgheader : msgheadertype; { when entering a reply }
    replymsgnum : integer;

    shouldsearch : array[1..maxareas] of boolean;

procedure updateuser;
begin
   reset( UserFile );
(*   if IOResult <> 0 then
   begin
      UWriteLn(oerr+UserListFN);
      HALT
   end;*)
   seek(Userfile, UserNumber);
   write( UserFile, User );
   close(Userfile);
   userchanged := false;
end;

{$i mfoc.inc }
{$i dispfile.inc } { overlays }
{$i viewusrl.inc }

{ vars for xxxxmsg.inc }
const maxLines = 46;

type linetype = array [1..maxLines] of st128;

var Line : ^linetype;
    LineNum : integer; { this is also the Number of Lines }
    Msgabort : boolean;
    savemsgerr : byte;
    NewMsgHeader : MsgHeadertype;


{$i msgstf2.inc } { not overlay! used directly by msgread }

procedure callDisplayMsg( DisplayMsgNum : integer); FORWARD;

{$I b:LogON.Inc }            {<--- overlay }
{$i dispmsg.inc }
{$i B:editmsg.inc }
{$i change.Inc }
{$i hurl.inc}
{$i CHNGBuls.inc }
{$i dispbuls.inc }
{$i B:chngaras.inc }
{$i savemsg.inc }
{$i stats.inc }
{$i msgkill.inc }
{$i msgstf.inc }
{$i renum.inc }
{$i setdate.inc }
{$i typefile.inc }

procedure callDisplayMsg; {forward}
begin displaymsg(displaymsgnum)
end;


{$I Msgenter.Inc }

{$i Msgread.Inc }

{$i menus.inc } {sysop and main}

begin { user section }
   {$i usersec.inc }
end; { UserSection }

{$i B:edusrlst.inc }       {}

{$i term.inc }             {}

{$i submit.inc }           {}

{$i main.inc }

