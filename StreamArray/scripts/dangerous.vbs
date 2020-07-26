Dim message, sapi
message=InputBox("What do you want me to say?","Speak to Me")
Set sapi=CreateObject("sapi.spvoice")
sapi.Speak message
