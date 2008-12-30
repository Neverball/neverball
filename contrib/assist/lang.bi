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
    declare function lang(Text as string) as string
    declare sub lang_select
    declare sub inilang

    function lang(Text as string) as string
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
                    return Converted(ConvertID)
                end if
            next ConvertID
        end if
        return Text
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
                    print "Unable to open "+MasterDir+"_
                        /"+LangFile+" for reading."
                    sleep
                end if
            end if
        end if
    end sub
#ENDIF
