/'
 ' This simple FB program checks source files for length and vertical
 ' whitespace related violations.
 '/
#include "file.bi"
dim as ushort MaxLen, MaxVertWhitespace
dim as uinteger VioCount(2,2)
dim as string File

/'
 ' The violationrc format is this.
 ' * First line is maximum length.
 ' * Second line is the maximum number of consecutive vertical lines. A blank
 ' line will break this combo.
 ' * Subsequent lines are the filesnames and (if applicable) their relative
 ' paths. For platform independency, use forward slashes.
 '
 ' Will spit out information to vioout.txt
 '/
open "vioout.txt" for output as #1
open "violationrc" for input as #2
input #2, MaxLen
input #2, MaxVertWhitespace
input #2, ""

if eof(2) = 0 then
	do
	    VioCount(1,1) = 0
	    VioCount(2,1) = 0
	    line input #2, File
	    if FileExists(File) then
	        dim as string PresentLine
	        dim as uinteger CurLine, LineCombo
	        print #1, "File ";File;" ("& FileLen(File);" bytes) opened."
	        open File for input as #3
	        do
	            CurLine += 1
	            line input #3, PresentLine
	            if PresentLine = "" then
	                if LineCombo > MaxVertWhitespace then
	                    VioCount(2,1) += 1
	                    VioCount(2,2) += 1
	                    print #1, "* Vertical whitespace violation found ";_
	                        "within Lines "& CurLine-LineCombo;"-"&_
	                        CurLine-1;"."
	                    print #1, "Consider adding a break somewhere within";
	                    print #1, " this section."
	                end if
	                LineCombo = 0

	            else
	                LineCombo += 1
	                if len(PresentLine) > MaxLen then
	                    VioCount(1,1) += 1
	                    VioCount(1,2) += 1
	                    print #1, "* Length violation found at Line ";CurLine;"."
	                    print #1, string(MaxLen,"-");"|"
	                    print #1, PresentLine
	                    print #1, string(MaxLen,"-");"|"
	                end if
	            end if
	        loop until eof(3)
	        close #3
	        print #1, "File ";File;" closed. ";
	        print #1, VioCount(1,1);" length and ";VioCount(2,1);_
	            " vertical whitespace violation(s) found."
            print #1, "Per kilobyte (SI-wise), there are ";
            print #1, using "##.##";VioCount(1,1)/FileLen(File)*1000;
	        print #1, " length violations and ";
            print #1, using "##.##";VioCount(2,1)/FileLen(File)*1000;
	        print #1, " vertical whitespace violations."
	        print #1, ""
	    else
	        print #1, "File ";File;" is non-existant."
	    end if
	loop until eof(2)
end if

close #2
print #1, "Checking complete. A total of ";VioCount(1,2);" length and ";_
	VioCount(2,2);" vertical whitespace violation(s) found."
close #1