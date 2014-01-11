#################################
# Makefile for whoami programs. #
#################################

all : whoami.exe pmwhoami.exe

whoami.exe : whoami.c whoami.def whoami.ico whoami.rc
	gcc whoami.c whoami.def -Wall -Zomf -Zmtd -s -O2 -lsocket -DVIO_APP\
-o whoami.exe
	rc whoami.rc whoami.exe

pmwhoami.exe : whoami.c pmwhoami.def whoami.ico whoami.rc
	gcc whoami.c pmwhoami.def -Wall -Zomf -Zmt -Zmtd -s -O2 -lsocket -DPM_APP\
-o pmwhoami.exe
	rc whoami.rc pmwhoami.exe

# end of whoami.mak #

