{ multiple areas }
type areatype = record
                  indexdrive,textdrive:char;
                  accesspriv,
                  enterpriv,
                  killpriv,
                  killanypriv,
                  readprivatepriv : byte;
                  attribute:integer; {0=normal,1=netmail,2=echomail}
                  selector:st14;
                  name:st40;
                  forwardnet, forwardnode: integer; {echomail}
                  hurlpriv:byte;
                  fill1:byte;
                  highmsg:integer;
                  KillSentPriv,
                  FileAttachPriv:byte;
                  fill:array [1..55] of byte
               end;
const maxareas = 20;

const nularea:areatype = ( { default values ? }
indexdrive:'A'; textdrive:'A';
accesspriv:20;
enterpriv:20;
killpriv:30;
killanypriv:100;
readprivatepriv:100;
attribute:0;
selector:'';
name:'';
forwardnet:1;forwardnode:-1;
hurlpriv:80;
fill1:0;
highmsg:0;
KillSentPriv:30;
FileAttachPriv:100;
fill:(
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
)
);

var areasf : file;
    area : areatype; { present area }
    areanum: integer; { present area number }


type nametype = st36;
     date = array [1..15] of char;

{ 21/4/88 }
 type attribtype = set of (private,crash,recd,sent,fileattached,intransit,
                           orphan,killSent,localMSG,holdforpickup,unused,
                           filerequest,returnreceiptrequest,IsReturnReceipt,
                           auditrequest,fileupdatereq);

type MsgHeadertype = record
 fromPerson,
 toPerson   : Nametype; {36, down from 40}
 SUBJ : ST73; { up from st70 }
 timesread: integer;
 dateentered: date;
 PointerTOBody : Integer;
 replyto : integer;

 attribute : attribtype;
 cost : integer;
 orignode : integer;
 destnode : integer;
 orignet : integer;
 destnet : integer;
 fill:array [1..20] of byte;
end;

const nulmsgheader:msgheadertype = (
 fromperson:'';
 toperson:'';
 subj:'';
 timesread:0;
DATEENTERED:'               ';
 pointertobody:-1;
 replyto:0;

 attribute:[];
 cost:0;
 orignode:0;
 destnode:0;
 orignet:0;
 destnet:0;
 fill:
 (0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0)
);

var Msgindex : file of MsgHeaderType;
    MsgText  : file;

const sizeofblockmap = 120; { that is bigger than is possible on a floppy! }
      sizeofdatablock = 512; (* 4 blocks *)

var fileDataBlock : array[ 1..sizeofdatablock ] of byte;
    Blockmap      : array[ 0..sizeofblockmap ] of byte absolute fileDataBlock;

var HighestMsgNum : integer; { the Number of the highest Msg, 0 when there is none }
    strHighestMsgNum : st4; { what happens if there are more than 9999 Msgs? }

    nummsgs : integer; { total number of messages excluding deleted ones }
    strnummsgs : st4;


type

     person = RECORD
                 Name : Nametype;
                 Password: St20;
                 ConsoleWidth : Integer;
                 consolehight : integer;
                 consoletype : byte;
                 nuls:byte;
                 more : boolean;
                 HighestReadMsg : Integer; { nolonger used for multi-area }
                 numcalls : integer;
                 lastcalled: date;
                 singlekey:boolean;
                 priv:integer;
                 lastarea: 1..maxareas; { area he was in }
                 highestread:array [1..maxareas] of integer;
                 help:byte; { 0,1..3 }
                 fill:array [1..19] of byte
              END;

const NulPerson:person =
                 (Name : '';
                 Password: '';
                 ConsoleWidth : 80;
                 consolehight : 24;
                 consoletype : 0;
                 nuls:0;
                 more : true;
HighestReadMsg : 0; { n/a v2 }
                 numcalls : 0;
lastcalled: '               ';
                 singlekey: false;
                 priv:20;
lastarea:1;
highestread:(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
help:3;
fill:(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0)
);

var userfile : file of person;

const highestrenumberable = 2000;
type Tabletype = array [ 0..highestrenumberable ] of integer;
     deltabletype = array [ 0..highestrenumberable ] of boolean;
var Table: Tabletype;
    deltable : deltabletype;

    ruser : person;
    msg   : MsgHeaderType;

const spin:st4 = '|/-\';



procedure inc( var i : integer );
begin i := SUCC(i) end;

procedure dec( var i : integer );
begin i := PRED(i) end;


function FileExists(FN:st14):boolean;
var f:File;
begin
  assign(f,FN);
  {$i-} reset(f); {$i+}
  Fileexists := (ioresult = 0)
end;

var uwtemp : st128;

{ 21/4/88 }

{ MFOC.INC = Message File Open/Close routines }

procedure openMsgfiles;
var msgtextfn, msgindexfn : ^st14;
    dummymsgheader:^msgheadertype;
    mif,mtf : boolean;
    ioerr:integer;
begin
(*   writeln('opening areas #',areanum);*)
   new(msgtextfn); new(msgindexfn);

   reset(areasf); { don't see why i have to do this, it is already open! }
   seek(areasf,areanum);
   blockread(areasf,Area,1); { area data is kept in memory while area is open }
   str(areanum,uwtemp);
   msgtextfn^ := Area.textdrive+':MSGTXT'+uwtemp+'.BBS';
   msgindexfn^ := Area.indexdrive+':MSGINX'+uwtemp+'.BBS';

   ASSIGN( MsgText, MsgTextFN^ );
   {$I-}RESet( MsgText );{$I+}
   ioerr := IOResult;
   MTF := ioerr=0;
   ASSIGN( MsgIndex, msgindexFN^ );
   {$I-}RESet( MsgIndex );{$I+}
   ioerr := IOResult;
   MIF := ioerr=0;

   if not (MIF and MTF) then
   begin
      writeln('Can''t find msg files');
      halt
   end;

   new(dummymsgheader);
   read( msgindex, dummymsgheader^);
   HighestMsgNum := dummymsgheader^.pointertobody;
   Nummsgs := dummymsgheader^.timesread;
   area.highmsg := highestmsgnum; { make sure this is correct }
(*;WRITELN('HIGHESTMSGNUM>',HighestMsgNum);*)
(*;WRITELN('NUMMSGS>',Nummsgs);*)
   dispose(dummymsgheader);

   blockread( MsgText, filedatablock, sizeofdatablock div 128 );

   dispose(msgtextfn); dispose(msgindexfn);
end;

Procedure updatemsgstats;
var dummymsgheader : ^msgheadertype;
begin
   new(dummymsgheader);
   dummymsgheader^.pointertobody := HighestMsgNum;
   dummymsgheader^.timesread := Nummsgs;
   seek( msgindex, 0);
   write( msgindex, dummymsgheader^);
   dispose(dummymsgheader)
end;

procedure closeMsgfiles;
var dummymsgheader : ^msgheadertype;
begin
   close( MsgIndex ); { pay attention to closing files; don't forget any! }
   close( MsgText);

   new(dummymsgheader);
   reset(areasf);
   area.highmsg := highestmsgnum;
   seek(areasf,areanum);
   blockwrite(areasf,area,1);
   close(areasf);
   reset(areasf);
   dispose(dummymsgheader)

end;


var RcvdOnly : boolean;
    killed:integer;


procedure MsgKill( KillMsgNum : integer);

var Msg : Msgheadertype;
    i   : integer;
    line:st128;

{ D)eA)llocate B)lock }
procedure DAB( I : integer);
begin
   Blockmap[ I DIV 8 ] := Blockmap[ I DIV 8 ] and (255 xor (1 shl (i mod 8)))
end;

begin {killmsg}
   str(KillMsgNum,uwtemp);

   seek( MsgIndex, KillMsgNum);
   Read( MsgIndex, Msg);

   if (Msg.fromperson[1] <> 'd') and { not already deleted and }
      ((not RcvdOnly) or
       ((RcvdOnly)and(recd in msg.attribute))) then
   begin
      inc(killed);
      Msg.fromperson[1] := 'd';
      seek( MsgIndex, KillMsgNum);
      write( MsgIndex, Msg);

      if Msg.pointertobody <> -1 then
      begin
         i := Msg.pointertobody;
         seek(Msgtext, i);
         blockread(Msgtext, Line[1], 1);
         dab(i);
         while (ord(Line[128]) <> 0) do
         begin
            inc(i);
            blockread(Msgtext, Line[1], 1 );
            dab(i)
         end;
      end;

      { update filedataBlock on disk }
      seek( MsgText, 0 );
      Blockwrite( MsgText, filedataBlock, sizeOfDataBlock div 128 );


      { scan highestmsgnum back }
      if killmsgnum = highestmsgnum then
      begin
         repeat
            dec( highestmsgnum );
            seek( MsgIndex, highestmsgnum);
            read( MsgIndex, Msg)
         until (msg.fromperson[1] <> 'd') or { complete scan even if carrier lost }
               (highestmsgnum < 2);
         if (highestmsgnum < 2) and (msg.fromperson[1]='d') then
            dec(highestmsgnum)
      end;
      dec( nummsgs);
{update msg stats} { do this after killing all }
   end

end; { msgkill }

procedure pass3;
label exit;
var numusers,
    i : integer;
begin
   Write(cr+'Pass 3  ');

   assign(userFile,userlistFN);
   {$I-}reset( UserFile );{$I+}
   if IOResult <> 0 then
   begin
      WriteLn(oerr+UserListFN);
      goto exit
   end;

   numusers := filesize( userfile );
   i := 0;
   while i < numusers do
   begin
      Write(bs+spin[(i mod 4)+1]);
      seek( userfile, i );
      read( userfile, ruser );

      if ruser.name[1] <> 'd' then { not deleted }
      begin
         ruser.HighestRead[areanum] := Table[ruser.highestread[areanum]];
         seek( userfile, i );
         write( userfile, ruser );
      end;

      i := succ( i )

   end;
   close( userfile );

   Writeln(bs+bs+'. ');
exit:
end;

procedure listareas;
var i : byte; ch:st1;
begin
   writeln;
   reset(areasf);
   i := 1;
   repeat
      seek(areasf,i);
      blockread(areasf,area,1);
      writeln(i:3,': ',area.selector:14,' (',area.indexdrive,') (',area.textdrive,')');
      i := i+1
   until eof(areasf);
   write('Press ENTER to continue');
   readln(ch);
end;