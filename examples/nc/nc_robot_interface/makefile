JFLAGS = -g
JC = javac
.SUFFIXES: .java .class
.java.class:
	$(JC) $(JFLAGS) $*.java

SOURCES = \
	FileSender.java \
	FileReceiver.java \
	ByteStream.java 

default: classes

classes: $(SOURCES:.java=.class)

clean:
	$(RM) *.class