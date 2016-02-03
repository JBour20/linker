SOURCES = Table.c Linker.c
OBJECTS = Table.o Linker.o
HEADERS = Table.h
EXEBIN  = Linker

all: $(EXEBIN)

$(EXEBIN) : $(OBJECTS) $(HEADERS)
	gcc -o $(EXEBIN) $(OBJECTS)

$(OBJECTS) : $(SOURCES) $(HEADERS)
	gcc -c $(SOURCES)

clean :
	rm -f $(EXEBIN) $(OBJECTS)