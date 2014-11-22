XVISION  =../../../
EXTENSION=mexglx
MATLAB=//usr/local/mm71
MEX = $(MATLAB)/bin/mex 
MYCFLAGS = -I$(XVISION)/include\
	   -I$(SVS_PATH)/src\
	   -I$(IPPROOT)/include\
           -I$(MATLAB)/extern/include 
LIBS = -L$(XVISION)/lib -lXVDevs -lXVImages -L$(IPPROOT)/sharedlib -lippi -lippcc -L$(SVS_PATH)/bin -lsvs -lsvscalc -lsvscap /usr/lib/libraw1394.so.8 -ldc1394 -lraw1394

# files to generate
FILES=grab_videre.mexglx 
SUFFIXES+=.mexglx

all: 	print $(FILES)

%.$(EXTENSION) : %.cc 
	@#echo making matlab interface file $@ from $< 
	$(MEX) $(MYCFLAGS) $< $(LFLAGS) $(LIBS) 

# prints what we are compiling
print:
	@echo COMPILING MATLAB INTERFACE FILES:
	@echo \   $(FILES)

clean:
	rm -f *~ .*~ core #*# ##*##
	rm -f *.$(EXTENSION) 


