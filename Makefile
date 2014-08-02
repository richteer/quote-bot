CC=clang

bot: bot.c quotereader.o
	${CC} $^ -lstrophe -o $@ -g -lexpat -lssl -lresolv

quotereader.o: quotereader.c quotereader.h
	${CC} $< -c -g
