PROGS           =  showlive play_avi show_v4l\
		showdig 

install: ; @for i in $(PROGS); do (cd $$i; $(MAKE) install); done
clean: ; @for i in $(PROGS); do (cd $$i; $(MAKE) clean); done
.depend: ;	@for i in $(PROGS); do (cd $$i; $(MAKE) .depend); done
