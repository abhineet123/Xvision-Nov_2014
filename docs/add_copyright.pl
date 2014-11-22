#!/usr/bin/perl -w

use strict;

($#ARGV == 1) or die "use: add_copyright.pl <FILENAME> <COPYRIGHT>\n";
my $filename = shift @ARGV;
my $copyright = shift @ARGV;

open(INFILE, $filename);

my @fileArray;

while(defined(my $line = <INFILE>)){

    push @fileArray, $line;
}

close(INFILE);

open(COPYRIGHT, $copyright);
open(OUTFILE, "> $filename");

my $notDone = 1;

for(my $i = 0; $i <= $#fileArray; ++$i){
    
    if($notDone){
	if($fileArray[$i] =~ /BEGIN_XVISION2_COPYRIGHT_NOTICE/){
	    
	    while(defined(my $cprtLine = <COPYRIGHT>)){
		print OUTFILE $cprtLine;
	    }
	    while(!($fileArray[$i] =~ /END_XVISION2_COPYRIGHT_NOTICE/)){ ++$i; }
	    ++$i;
	    $notDone = 0;
	}
    }
    print OUTFILE $fileArray[$i];
    ++$i;
}

if($notDone)
    print "did not update copyright in: ", $filename, "\n";
else
    print "updating copyright in: ", $filename, "\n";

close(COPYRIGHT);
close(OUTFILE);

exit(0);
