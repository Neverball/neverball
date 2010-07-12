/'
 ' This simple FB program checks source files for length and vertical
 ' whitespace related violations.
 '
 ' The vbcompat.bi is a compiler file.
 '/
#include "vbcompat.bi"
dim as ushort MaxLen, MaxVertWhitespace
dim as uinteger VioCount(2,2), TotalSize, FileCount
dim as string ShortFile, LongFile, FilePattern, DirReference, TrimSet
const FileOut = "stdout.txt"
const Resources = "violationrc"

/' ------------------------------------------------------------------------- '/

/'
 ' The violationrc format is this.
 ' * First line is maximum length.
 ' * Second line is the maximum number of consecutive vertical lines. A blank
 ' line will break this combo.
 ' * Third line is the trim set for purposes of breaking the combo above.
 ' Lines checked for purposes of the combo have the configured characters
 excluded. If this resultant is blank, it will break the combo above as well.
 ' * Fourth line should be blank, as it isn't loaded.
 ' * Subsequent lines are the file patterns and (if applicable) their relative
 ' paths. For platform independency, use forward slashes.
 '
 ' Will spit out information to stdout.txt
 '/
open FileOut for output as #1
open Resources for input as #2
input #2, MaxLen
input #2, MaxVertWhitespace
input #2, TrimSet
input #2, ""

/' ------------------------------------------------------------------------- '/

if eof(2) = 0 then
    do
        VioCount(1,1) = 0
        VioCount(2,1) = 0
        line input #2, FilePattern
        for NPID as ushort = 1 to len(FilePattern)
            if mid(FilePattern,NPID,1) = "/" then
                DirReference = left(FilePattern,NPID)
            end if
        next NPID
        ShortFile = dir(FilePattern, fbNormal)
        while len(ShortFile) > 0
            dim as string PresentLine
            dim as uinteger CurLine, LineCombo
            LongFile = DirReference + ShortFile
            print "Opening file ";LongFile;"... ";
            print #1, "File ";LongFile;" ("& _
                FileLen(LongFile);" bytes) opened."
            TotalSize += FileLen(LongFile)
            FileCount += 1
            open LongFile for input as #3
            do
                CurLine += 1
                line input #3, PresentLine
                if PresentLine = "" OR _
                    (trim(PresentLine, Any TrimSet) < > "" AND _
                    TrimSet < > "") then
                    if LineCombo > MaxVertWhitespace then
                        VioCount(2,1) += 1
                        VioCount(2,2) += 1
                        print #1, "* Vertical whitespace violation found ";_
                            "within Lines "& CurLine-LineCombo;"-"&_
                            CurLine-1;"."
                        print #1, "Consider adding a break somewhere within ";
                        print #1, "this section."
                    end if
                    LineCombo = 0

                else
                    LineCombo += 1
                    if len(PresentLine) > MaxLen then
                        VioCount(1,1) += 1
                        VioCount(1,2) += 1
                        print #1, "* Length violation found at Line ";_
                            CurLine;".";
                        if len(PresentLine)-MaxLen = 1 then
                            print #1, " There is 1 character too many."
                    	else
                            print #1, " There are "& _
                                len(PresentLine)-MaxLen;" characters ";_
                            "too many."
                        end if
                        print #1, string(MaxLen,"-");"|"
                        print #1, PresentLine
                        print #1, string(MaxLen,"-");"|"
                    end if
                end if
            loop until eof(3)
            close #3
            print "closed."
            print #1, "File ";LongFile;" closed. ";
/' ------------------------------------------------------------------------- '/
            if VioCount(1,1) > 0 OR VioCount(2,1) > 0 then
                print #1, VioCount(1,1);" length and ";VioCount(2,1);_
                    " vertical whitespace violation(s) found."
                print #1, "Per kilobyte (SI), there are ";
                print #1, using "##.##";VioCount(1,1)/FileLen(LongFile)*1000;
                print #1, " length violations and ";
                print #1, using "##.##";VioCount(2,1)/FileLen(LongFile)*1000;
                print #1, " vertical whitespace violations."
            else
                print #1, "No length or vertical whitespace violations found."
            end if
            print #1, ""
            ShortFile = dir()
        wend
    loop until eof(2)
end if

close #2
print #1, "Checking complete."
print #1, "-- Statistics --"
print #1, "Total files: ";FileCount;" files"
print #1, "Total project size: ";TotalSize;" bytes"
print #1, "Average file size: ";
print #1, using "#######.### bytes/file";TotalSize/FileCount
print #1, "Total Length violations: ";VioCount(1,2)
print #1, "Length violations per kilobyte: ";
print #1, using "##.###";VioCount(1,2)/TotalSize*1000
print #1, "Length violations per file: ";
print #1, using "##.###";VioCount(1,2)/FileCount
print #1, "Total Vertical whitespace violations: ";VioCount(2,2)
print #1, "Vertical whitespace violations per kilobyte): ";
print #1, using "##.###";VioCount(2,2)/TotalSize*1000
print #1, "Vertical whitespace violations per file: ";
print #1, using "##.###";VioCount(2,2)/FileCount
close #1