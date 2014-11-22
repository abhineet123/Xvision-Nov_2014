XVISION  =../../../
EXTENSION=mexglx
MEX = $(MATLAB)/bin/mex 
MYCFLAGS = -I$(XVISION)/include\
	   -I$(SVS_PATH)/src\
           -I$(MATLAB)/extern/include 
LIBS = -L$(XVISION)/lib -lXVDevs -lXVImages -L$(IPPROOT)/sharedlib -lippi -lippcc 

# files to generate
FILES= 
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


