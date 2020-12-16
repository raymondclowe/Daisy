{ (c) Raymond Lowe 7/1/1989 All rights reserved }

{$B+}
{$C-}
{$I+}
{$R-}
{$V+}
{$U-}
{$A+}
{$W0}
{$X+}


Program Mailer;

{ things for passing }
{} var passing : array [1..80] of char;
{}     Dashes  : array [1..80] of char absolute passing;
{}
{}     areasdrv,
{}     generaldrv: char;
{}     thisnet, thisnode : integer;
{}     callersbaud : integer;
{}     quiet,
{}     clock: boolean;
{}     clockslot:integer;
{}     BBSnodelistdrv:char;
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

totalcalls,
altnet1,altnode1,
altnet2,altnode2,
altnet3,altnode3,
altnet4,altnode4: integer;
filesDrive : char;
welcomefn:st14;
eventtag : char;
setdatepriv:integer;
templogFN,
questionFN :st14;
kludgepriv,
sysmaxbaud : integer;
fill : array [1..50] of char;

{ end things for passing }

const version = 'Mailer 0.38 (for Daisy v2i)';
      MailerProductCode = $1e; { offical IFNA ! }


label exitmailer, jump2arc;


var todayyear,
    todaymonth,
    todayday,
    NowHour,
    NowMin,
    nowseconds,
    lastupdated :integer;

var uwtemp : st128;

const esc = ^[;
      nul = ^@;
      cr  = ^M;
      lf  = ^J;
      sp  = ' ';
      SOH = #$01;
      ENQ = #$05;
      ACK = #$06;
      SYN = #$16;
      SUB = #$1a;
      tsynch = #174;
      yoohoo = #$f1;

      NAK = ^U;
      EOT = ^D;
      CAN = ^X;
      ETB = ^W;
      req_CRC = 'C';

const oerr = 'Error opening ';
      cerr = 'Error creating ';


{$I Config.dif }
{$i areas.dif }
var mailarea : integer;
    mailwork,
    numareas : integer;
    areatag : array [1..maxareas] of st14;
    areaattribute : array [1..maxareas] of integer;

    CurrentDrive : char;

{ error codes }
const unknown = 0;
      Okay = 1;
      noConnect = 2;
      failed = 3;
      noCalls = 3; {?}
      garbage = 3; {?}

var CallResult : integer;

    ch : char; { lastest input from modem?}

const valideventcodes:set of char = ['H','C','W','P','G','R'];

var calls : array[1..15] of record
packetnum,
orignode,
destnode,
orignet,
destnet,
routetonet,
routetonode:integer;
sent,
hold,
trytosend:boolean;
connects : integer;
                            end;

var NumOutboundPkts:integer;
    PktsYetToSend:integer;
var datestr : st15;

{$i nodetype.dif }

{$I stuff.inc }

{$i sioFN.inc }


{$i pkttyp.dif }
{$i msgtyp.dif }

type date = array [1..15] of char;
     nametype = st36;


{$i msgfile.dif }


type linetype = array[1..46] of st128;
var line : linetype;
    linenum : integer;
    NewMsgHeader : msgheadertype;
    savemsgerr : byte;
    ioerr : integer;

var interactive : boolean;
    DidGotPkt : boolean;

{$i mfoc.inc }

{$i savemsg.inc }

{$i oknn.inc }

{$i msgkill.inc }


{$I Inctime.inc }
{$i unpack.inc }   {}
{$i perams.inc }
{$i pack.inc }     {overlays}
{$i initpkt.inc }  {}
{$i modem1.inc }
{$i modem2.inc }   { Stanley's stuff }
{$i modem3.inc }


{$i bxrs.inc }

procedure r2; FORWARD;
procedure s3; FORWARD;

var scan,ThisCall : integer;
    Rdone,Sdone:boolean;

var i:integer;stri:st8;
    ti:integer;
    eventcode:char;
    err:integer;
    tagnum:Integer;
    statsread : boolean;
    df : ^text;
    human : boolean;

{$i s.inc }

{$i r.inc }

{$i w.inc }

{$i mmain.inc }
