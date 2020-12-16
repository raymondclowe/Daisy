{ 11/6/88 }

{$B+} {default + i/o mode selection}
{$C-} {default + control s and c}
{$I+} {default + i/o error handling}
{$R-} {default - Index range Check}
{$V+} {default + var paramenter type Checking}
{$U+} {default - User interrUpt}
{$A+} {default + absolute code}
{$W0} {default 2 nesting of with statements}
{$X+} {default + array optimization}


program PACKUSERLIST;

const maxareas = 20;
type st6 = string[6];
     st9 = string[9];
     st20 = string[20];
     st36 = string[36];
     st40 = string[40];

type date = Array [1..15] of char; { don't need a length indicator! }
     Nametype = St36; { down from 40... important in upgrader! }
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

var newuserfile : file of person;
    User  : person;


const
      userLISTFN :string[14] = 'userLIST.bbs';

var numusers,
    i, j : integer;
    sdrv, tdrv, ch: char;
    olddate : st9;
    dateretain : st6;
    privretain : integer;

{$i dtos.inc}

begin

   writeln('PACKUSER 4 - Raymond Lowe, Simmons Kwan');
   writeln('Pack Daisy v2 USERLIST.BBS file by selective copy');
   writeln;
   write('Source drive : ');
   buflen := 1;
   readln(sdrv); sdrv := upcase(sdrv);
   write('Target drive : ');
   buflen := 1;
   readln(tdrv); tdrv := upcase(tdrv);
   if not ((tdrv in ['A'..'F']) and (sdrv in ['A'..'F'])) then
   begin
      writeln('Invalid drive specs');
      halt
   end;
   if tdrv = sdrv then
   begin
      writeln('Can''t copy to same drive');
      halt
   end;

   write('Remove inactive user ? ');
   repeat
      read(kbd, ch);
      ch := upcase(ch);
   until ch in ['Y', 'N'];
   writeln(ch);
   if ch = 'Y' then
   begin
      repeat
         write('Enter earliest last called date (DD MMM YY) : ');
         readln(olddate);
      until length(olddate) = 9;
      dateretain := date2str(olddate);

      repeat
         write('Minimum privilage level excempted from deletion : ');
         readln(privretain);
      until privretain in [1..100];
   end
   else
   begin
      dateretain := '000000';
      privretain := 0;
   end;

   writeln;
   writeln;
   write('Press <Enter> to go ahead or <Esc> to exit ');
   repeat
      read(kbd,ch)
   until ch in [#13,#27];
   if ch = #27 then halt;
   writeln;

   assign( userfile, sdrv+':'+userlistFN );
   {$i-}
   reset( userfile );
   {$I+}
   if IOResult <> 0 then
   begin
      WriteLn('Error Opening source file');
      HALT
   end;
   assign( newuserfile, tdrv+':'+userlistFN );
   {$I-}
   rewrite( newuserfile );
   {$I+}
   if IOResult <> 0 then
   begin
      WriteLn('Error Opening target file');
      HALT
   end;


   writeln;
   numusers := filesize( userfile );
   writeln('There are ',numusers,' records in the original file');
   writeln('<Each dot represents 10 records>');
   writeln;
   i := 0;
   while i < numusers do
   begin
      if i mod 10 = 0 then write('.');

      {$i-}
      seek( userfile, i );
      if ioresult = 0 then
         read( userfile, user )
      else
      begin
         writeln;
         writeln('Error: Skipping record, #',i)
      end;
      if ioresult = 0 then
      {$i+}
      begin
         olddate := '';
         for j := 1 to 9 do
            olddate := olddate + user.lastcalled[j];
         if (user.name[1] <> 'd') and
            (length(user.name) in [1..36]) and
            (length(user.password) in [1..36]) and
            (user.numcalls > 0) and
            ((date2str(olddate) >= dateretain) or
            (user.priv >= privretain)) then
            write( newuserfile, user )
      end
      else
      begin
         writeln;
         writeln('Error while reading: Skipping record, #',i)
      end;

      i := i + 1
   end;
   writeln;
   numusers := filesize( newuserfile );
   writeln('There are ',numusers,' users in the new file');
   close( userfile );
   close( newuserfile);

   writeln('Done.');
   delay(1000)

end.
