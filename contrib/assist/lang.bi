#IFNDEF __LANG_BI__
    #DEFINE __LANG_BI__
    dim shared as string MasterDir
    MasterDir = curdir
    const Strings = 5000
    const l = 99
    const u = 98
    const UpdateSwitch = "-lang"
    dim shared as string LangFile, Unmodded(Strings), Existant(Strings), _
        Converted(Strings)
    dim shared as ushort LangUpdateID

    function remainder(Byval Param1 as double, Byval Param2 as double) as double
        'Returns the remainder of a division.
        if Param2 = 0 then
            return 1e+300
        else
            return Param1-(int(Param1/Param2)*Param2)
        end if
    end function

    function word_wrap(Text as string, Cap as ushort = 80) as string
    	'Wraps the text automatically.
        dim as ushort RefChar, RaisedChar, TotalRaised
        dim as string OutText = Text
        for WID as ushort = 1 to len(Text)
            if mid(Text,WID,1) = chr(32) then
                RefChar = WID
            end if
            if remainder(WID+TotalRaised,Cap) = 0 then
                if RefChar > 0 then
                    RaisedChar = Cap - remainder(RefChar+TotalRaised,Cap)
                else
                    RaisedChar = 0
                end if
                if RaisedChar = Cap then
                    RaisedChar = 0
                end if
    
                OutText = left(OutText,RefChar+TotalRaised-1)+space(RaisedChar)+right(OutText,len(OutText)-TotalRaised-RefChar+1)
                TotalRaised += RaisedChar
                RefChar = 0
            end if
        next WID
        return OutText
    end function

    function lang(Text as string, SWidth as ushort = 80) as string
        dim as ubyte ConvertID, Duplicate
        if Command(1) = UpdateSwitch then
            Duplicate = 0
            for ConvertID = 1 to Strings
                if Converted(ConvertID) = Text then
                    Duplicate = 1
                    exit for
                end if
            next
            if LangFile = "" AND LangUpdateID < Strings AND Duplicate = 0 then
                LangUpdateID += 1
                print #u, Text
                print #u, ""
                Converted(LangUpdateID) = Text
            end if
        else
            for ConvertID = 1 to Strings
                if Unmodded(ConvertID) = Text AND _
                    Converted(ConvertID) < > "" then
                    return word_wrap(Converted(ConvertID),SWidth)
                end if
            next ConvertID
        end if
        return word_wrap(Text,SWidth)
    end function

    sub lang_select
        dim as ubyte ConvertID
        dim as integer Result
        close #u
        cls
        input "Which language file";LangFile
        if LangFile < > "" then
            #IF defined(__FB_WIN32__) OR defined(__FB_DOS__)
                Result = open(MasterDir + _
                    "\" + LangFile + ".txt" for input as #l)
            #ELSE
                Result = open(MasterDir + _
                    "/" + LangFile + ".txt" for input as #l)
            #ENDIF
            if Result = 0 then
                for ConvertID = 1 to Strings
                    Unmodded(ConvertID) = ""
                    Converted(ConvertID) = ""
                next ConvertID
                for ConvertID = 1 to Strings
                    line input #l, Unmodded(ConvertID)
                    line input #l, Converted(ConvertID)
                    if eof(l) then exit for
                next ConvertID
                close #l
            else
                print "Unable to open "+MasterDir+"/"+LangFile+" for reading."
                sleep
            end if
        end if
    end sub

    sub inilang
        dim as ubyte ConvertID
        dim as integer Result
        if Command(1) = UpdateSwitch then
            for ConvertID = 1 to Strings
                Unmodded(ConvertID) = ""
                Converted(ConvertID) = ""
            next
            LangFile = ""
            open "default.txt" for output as #u
        else
            if LangFile < > "" then
                #IFDEF __FB_WIN32__
                    Result = open(MasterDir + _
                        "\" + LangFile + ".txt" for input as #l)
                #ELSE
                    Result = open(MasterDir + _
                        "/" + LangFile + ".txt" for input as #l)
                #ENDIF
                if Result = 0 then
                    for ConvertID = 1 to Strings
                        Unmodded(ConvertID) = ""
                        Converted(ConvertID) = ""
                    next ConvertID
                    for ConvertID = 1 to Strings
                        line input #l, Unmodded(ConvertID)
                        line input #l, Converted(ConvertID)
                        if eof(l) then exit for
                    next ConvertID
                    close #l
                else
                    print "Unable to open "+MasterDir+_
                        "/"+LangFile+" for reading."
                    sleep
                end if
            end if
        end if
    end sub
#ENDIF
