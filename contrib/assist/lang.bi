dim shared as string MasterDir
MasterDir = curdir
const Strings = 5000
const l = 99
dim shared as string LangFile, Unmodded(Strings), Existant(Strings), _
                     Converted(Strings)
declare function lang(Text as string) as string
declare sub lang_select
declare sub inilang

function lang(Text as string) as string
    dim as ubyte ConvertID
    for ConvertID = 1 to Strings
        if Unmodded(ConvertID) = Text AND _
            Converted(ConvertID) < > "" then
            return Converted(ConvertID)
        end if
    next ConvertID
    return Text
end function

sub lang_select
    dim as ubyte ConvertID
    dim as integer Result
    cls
    input "Which language file";LangFile
    if LangFile < > "" then
        #IF defined(__FB_WIN32__) OR defined(__FB_DOS__)
        Result = open(MasterDir + "\" + LangFile + ".txt" for input as #l)
        #ELSE
        Result = open(MasterDir + "/" + LangFile + ".txt" for input as #l)
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
    if LangFile < > "" then
        #IFDEF __FB_WIN32__
        Result = open(MasterDir + "\" + LangFile for input as #l)
        #ELSE
        Result = open(MasterDir + "/" + LangFile for input as #l)
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