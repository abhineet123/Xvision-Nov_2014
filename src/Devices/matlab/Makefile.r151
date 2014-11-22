XVISION  =../../../
EXTENSION=mexglx
MEX = $(MATLAB)/bin/mex 
MYCFLAGS = -I$(XVISION)/include\
	   -I/usr/src/linux-2.4/drivers/ieee1394\
           -I$(MATLAB)/extern/include 
LIBS = -L$(XVISION)/lib -lXVDevs -lXVImages 

# files to generate
FILES= grab_analog.mexglx grab_digit.mexglx
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


