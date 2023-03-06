START 			= gcc
SERVER_FILE		= server/server.c
CLIENT_FILE		= client/client.c
EXE_S_NAME		= server.o
EXE_C_NAME		= client.o
OPT				= -o

all :
		${START} ${SERVER_FILE} ${OPT} ${EXE_S_NAME}
		${START} ${CLIENT_FILE} ${OPT} ${EXE_C_NAME}