{ 1/1/88 }

function UpString(Line:st255):st255;
var i : Integer;
begin
   for i := 1 to length (Line) do
      Line[I] := UpCase( Line [I] );
   UpString := Line
end; {UpString}

function downCase(character:char):char;
begin
   if character in ['A'..'Z'] then
      downCase := chr( ord (character) + 32)
   else
      downCase := character
end; {downCase}

{ convert String to first letter UperCase all others lower }
function NameCase(Line:st128):st128;
var i : Integer;
begin
   Line[1] := UpCase( Line[1] );
   for i := 2 to length (Line) do
   begin
      if Line[I-1] in ['-','.',' ',''''] then
         Line[I] := upCase( Line [I] )
      else
         Line[I] := downCase( Line [I] )
   end;
   NameCase := Line
end; { NameCase }

