#!fgrab_wish-0.8


#
#	Copyright (C) 1997 Heiko Teuber (heiko@pc10.pc.chemie.th-darmstadt.de).
#
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program; if not, write to the Free Software
#	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
#	I hope you enjoy using this program. Please report any bugs and useful
#	features you would like to find in this program to
#	heiko@pc10.pc.chemie.th-darmstadt.de



set FGrab(video) Off
set FGrab(path) "/home/scratch/heiko/"
set FGrab(name) "heiko"
set FGrab(incr) ""
set FGrab(image) ""
set FGrab(format) pal
set FGrab(device) svideo
set FGrab(yuv) planar
set FGrab(font) -Adobe-Times-Medium-R-Normal-*-180-*
set FGrab(corr) 1

set FGrab(video,columns) {320 0 768 8} 
set FGrab(video,rows) {240 0 576 2}
set FGrab(video,bright) {148 0 255 1}
set FGrab(video,chroma) {114 0 255 1}
set FGrab(video,contrast) {74 0 255 1}
set FGrab(video,hue) {0 -128 127 1}
set FGrab(video,fps) {12 1 25 1}

set FGrab(capture,columns) {320 0 768 8}
set FGrab(capture,rows) {240 0 576 2}
set FGrab(capture,bright) {148 0 255 1}
set FGrab(capture,chroma) {114 0 255 1}
set FGrab(capture,contrast) {74 0 255 1}
set FGrab(capture,hue) {0 -128 127 1}

set FGrab(capture,source) meteor
set FGrab(capture,depth) 24bpp

set FGrab(list1) {On Hold Off}
set FGrab(list2) {capture video}
set FGrab(list3,capture) {columns rows contrast bright chroma hue}
set FGrab(list3,video) {columns rows contrast bright chroma hue fps}
set FGrab(depth,list) {24bpp 15bpp b/w}
set FGrab(device,list) {svideo rca rgb dev0 dev1 dev2}
set FGrab(format,list) {pal secam ntsc}
set FGrab(yuv,list) {planar packed}
set FGrab(length) 2.5c




proc Video value {

    global FGrab

    switch $value {
	On {
	    if {![winfo exists .live]} {
		if {[winfo depth .] == 8} {
		    toplevel .live -colormap new -visual {staticgray 8}
		} else {
		    toplevel .live -colormap new 
		}		    
		fgrab .live.meteor -format $FGrab(format)\
			-device $FGrab(device)\
			-yuv $FGrab(yuv)
		pack .live.meteor
		.live.meteor videoconfigure -rows [.scalevideorows get]\
			-columns [.scalevideocolumns get]\
			-fps [.scalevideofps get]\
			-contrast [.scalevideocontrast get]\
			-bright [.scalevideobright get]\
			-chroma [.scalevideochroma get]\
			-hue [.scalevideohue get]
	    }
	    .live.meteor video on
	    DeviceFormatYuv Off
	    Correlate
	}
	Hold {
	    if {[winfo exists .live.meteor]} {
		.live.meteor video hold
		foreach k $FGrab(list3,video) {
		    .scalevideo$k configure -state disabled
		}
	    }
        }
	Off {
	    if {[winfo exists .live.meteor]} {
		.live.meteor video off
		destroy .live
	    }
            DeviceFormatYuv On
	}
    }
}


proc DeviceFormatYuv {flag} {

    global FGrab


    if {$flag == "Off"} {
	foreach k $FGrab(list3,video) {
	    .scalevideo$k configure -state normal
	}
	foreach k $FGrab(device,list) {
	    .radioDevice$k configure -state disabled
	}
	foreach k $FGrab(format,list) {
	    .radioFormat$k configure -state disabled
	}
	foreach k $FGrab(yuv,list) {
	    .radioYuv$k configure -state disabled
	}
	.radioVideoHold configure -state normal
    } else {
	foreach k $FGrab(list3,video) {
	    .scalevideo$k configure -state disabled
	}
	foreach k $FGrab(device,list) {
	    .radioDevice$k configure -state normal
	}
	foreach k $FGrab(format,list) {
	    .radioFormat$k configure -state normal
	}
	foreach k $FGrab(yuv,list) {
	    .radioYuv$k configure -state normal
	}
	.radioVideoHold configure -state disabled
    }
}


proc Quit {} {

    if {[winfo exists .live.meteor]} {
	destroy .live.meteor	
    }
    exit
}
proc Configure {modus what value} {

    global FGrab

   if {[winfo exists .live.meteor]} {
	.live.meteor ${modus}configure -$what $value 
    }
    if {$FGrab(corr)} {
	if {($modus == "video") && ($what != "fps")} {
	    .scalecapture$what set $value
	} else {
	    .scalevideo$what set $value
	}
     }
}

proc Capture {flag} {

    global FGrab


    if {$flag == "Start"} {
	if {[winfo exists .live.meteor]} {
	    .buttonCapt configure -text "Stop" -command {Capture Stop}
	    update idletasks
	    if {$FGrab(continous)} {
		if {$FGrab(fast)} {
		    for {set i 0} {$i < $FGrab(amount)} {incr i} {
			CaptureSingle
		    }
		    .buttonCapt configure -text "Start"\
			    -command {Capture Start}
		} else {
		    set FGrab(amount,startvalue) $FGrab(amount)
		    if {$FGrab(amount) > 0} {
			incr FGrab(amount) -1
			CaptureSingle
			if {$FGrab(amount) > 0} {
			    set FGrab(after) [after [expr $FGrab(intervall)\
				    * 1000] CaptureCont]
			}
		    }
		}
	    } else {
		CaptureSingle
		.buttonCapt configure -text "Start" -command {Capture Start}
	    }
	}
    } else {
	after cancel $FGrab(after)
	.buttonCapt configure -text "Start" -command {Capture Start}
	set FGrab(amount) $FGrab(amount,startvalue)
    }
}

proc CaptureCont {} {

    global FGrab


    incr FGrab(amount) -1
    CaptureSingle
    if {$FGrab(amount) > 0} {
	set FGrab(after) [after [expr $FGrab(intervall)\
		* 1000] CaptureCont]
    } else {
	set FGrab(amount) $FGrab(amount,startvalue)
	.buttonCapt configure -text "Start" -command {Capture Start}
    }
}    


proc CaptureSingle {} {

    global FGrab
    
    set W .captured
    
    .live.meteor captureconfigure -rows [.scalecapturerows get]\
	    -columns [.scalecapturecolumns get]\
	    -contrast [.scalecapturecontrast get]\
	    -bright [.scalecapturebright get]\
	    -chroma [.scalecapturechroma get]\
	    -hue [.scalecapturehue get]\
	    -oformat $FGrab(capture,depth)\
	    -source  $FGrab(capture,source)	
    .live.meteor capture $FGrab(path)$FGrab(name).$FGrab(incr)
    if {$FGrab(autoload)} {
	if {![winfo exists $W.single]} {
	    toplevel $W
	    canvas $W.single
	    set FGrab(image) [image create photo\
		    -file $FGrab(path)$FGrab(name).$FGrab(incr)]
	    set FGrab(canvas,image) [$W.single create image 0 0\
		    -image $FGrab(image)]
	    pack $W.single
	} else { 
	    image delete $FGrab(image)
	    $W.single delete $FGrab(canvas,image)
	    set FGrab(image) [image create photo\
		    -file $FGrab(path)$FGrab(name).$FGrab(incr)]
	    set FGrab(canvas,image) [$W.single create image 0 0\
		    -image $FGrab(image)]
	}
	set height [image height $FGrab(image)]
	set width  [image width $FGrab(image)]
	
	$W.single coords $FGrab(canvas,image) [expr $width/2]\
		[expr $height/2]
	$W.single configure -width $width -height $height
    }
    if {$FGrab(increment)} {
	incr FGrab(incr)
	.buttonResetIncr configure -text $FGrab(incr)
    }   
}
   

proc Correlate {} {

    global FGrab


    if {$FGrab(corr)} {
	foreach k $FGrab(list3,capture) {
	    .scalecapture$k set [.scalevideo$k get]
	}
    }
}

proc Continous {} {

    global FGrab

    
    if {$FGrab(continous)} {
	.butCaptFast configure -state normal
	.checkAutoIncr configure -state disabled
	set FGrab(increment) 1
	.entryAmount configure -state normal
	.labelAmount configure -foreground black
	set FGrab(incr) 1
	.buttonResetIncr configure -text $FGrab(incr) -state normal
	if {!($FGrab(fast))} {
	    .entryIntervall configure -state normal
	    .labelIntervall configure -foreground black
	}
    } else {
	.butCaptFast configure -state disabled
	.checkAutoIncr configure -state normal
	set FGrab(inrement) 0
	.entryAmount configure -state disabled
	.entryIntervall configure -state disabled
	.labelAmount configure -foreground #a3a3a3
	.labelIntervall configure -foreground #a3a3a3
    }	
}    

proc Fast {} {

    global FGrab


    if {$FGrab(fast)} {
	set FGrab(autoload) 0
	.checkAutoShow configure -state disabled
	.entryIntervall configure -state disabled
	.labelIntervall configure -foreground #a3a3a3
    } else {
	.checkAutoShow configure -state normal
	.entryIntervall configure -state normal
	.labelIntervall configure -foreground black
    }
}

proc Source {} {

    global FGrab


    if {$FGrab(capture,source) == "video"} {
	set FGrab(corr) 1
	Correlate
	.checkCaptCor configure -state disabled
	foreach i $FGrab(depth,list) {
	    .radioCaptDepth$i configure -state disabled
	}
    } else {	
	set FGrab(corr) 0
	.checkCaptCor configure -state normal
	foreach i $FGrab(depth,list) {
	    .radioCaptDepth$i configure -state normal
	}
    }
}


proc Increment {} {

    global FGrab

    if {$FGrab(increment)} {
	set FGrab(incr) 1
	.buttonResetIncr configure -text $FGrab(incr) -state normal
    } else {
	set FGrab(incr) ""
	.buttonResetIncr configure -text $FGrab(incr) -state disabled
    }
}


proc ControlPanel {} {

    global FGrab

    set l2 {capture video}
    set l3(capture) {columns rows contrast bright chroma hue}
    set l3(video) {columns rows contrast bright chroma hue fps}
    set length 2.5c

    frame .capture -relief raised -bd 2
    frame .capture2
    frame .capture3
    frame .video -relief raised -bd 2
    frame .frameVideo2
    label .labelCapture -text "Single-Capture Options" -pady 2m\
	    -font $FGrab(font) -relief raised -bd 2
    label .labelLive -text "Live-Video Options" -font $FGrab(font) -pady 2m\
	    -relief raised -bd 2
    button .quit -text "Quit" -command Quit
    pack .labelCapture .capture2 .capture3 .capture .labelLive .frameVideo2\
	    .video .quit -side top -fill x -expand 1

    frame .frameVideoOn -relief raised -bd 2
    frame .frameDevice -relief raised -bd 2
    frame .frameDevice1
    frame .frameDevice2
    frame .frameFormat -relief raised -bd 2
    frame .frameYuv -relief raised -bd 2
    pack .frameVideoOn .frameDevice .frameFormat .frameYuv\
	    -in .frameVideo2 -side left -expand 1 -fill both
    pack .frameDevice1 .frameDevice2 -in .frameDevice -side left\
	    -expand 1 -fill both


    foreach i $FGrab(list1) {
	radiobutton .radioVideo$i -text $i -variable FGrab(video) -value $i\
		-anchor w -command [list Video $i]
	pack .radioVideo$i -in .frameVideoOn -side top -fill x -expand 1
    }
    .radioVideoHold configure -state disabled

    foreach i $FGrab(device,list) {
	radiobutton .radioDevice$i -text $i -variable FGrab(device) -value $i\
		-anchor w
    }
    pack .radioDevicesvideo .radioDevicerca .radioDevicergb  -in .frameDevice1\
	    -side top -fill x   
    pack .radioDevicedev0 .radioDevicedev1 .radioDevicedev2  -in .frameDevice2\
	    -side top -fill x

    foreach i $FGrab(format,list) {
	radiobutton .radioFormat$i -text $i -variable FGrab(format) -value $i\
		-anchor w
	pack .radioFormat$i -in .frameFormat -side top -fill x
    }

    foreach i $FGrab(yuv,list) {
	radiobutton .radioYuv$i -text $i -variable FGrab(yuv) -value $i\
		-anchor w
	pack .radioYuv$i -in .frameYuv -side top -fill both
    }


    frame .frameCaptDepth -relief raised -bd 2
    frame .frameCaptOption1 -relief raised -bd 2
    frame .frameCaptFile -relief raised -bd 2
    pack .frameCaptDepth -in .capture2 -side left -fill y
    pack .frameCaptOption1 -in .capture2 -side right -fill y
    pack .frameCaptFile -in .capture2 -side left -expand 1 -fill both

    foreach i $FGrab(depth,list) {
	radiobutton .radioCaptDepth$i -text $i -variable FGrab(capture,depth)\
		-value $i -anchor w
	pack .radioCaptDepth$i -in .frameCaptDepth -side top\
		-expand 1 -fill both
    }

    radiobutton .radioSourceVideo -text "video-capture"\
	    -variable FGrab(capture,source) -value video -anchor w\
	    -command {Source}
    radiobutton .radioSourceMeteor -text "direkt-capture"\
	    -variable FGrab(capture,source) -value meteor -anchor w\
	    -command {Source}
    pack .radioSourceMeteor .radioSourceVideo -in .frameCaptOption1


    frame .frameCaptFile1
    frame .frameCaptFile2
    frame .frameCaptFile3
    pack .frameCaptFile3 -in .frameCaptFile -side bottom
    pack .frameCaptFile1 .frameCaptFile2 -in .frameCaptFile -side left

    checkbutton .checkAutoIncr -text "Increment" -variable FGrab(increment)\
	    -anchor w -command {Increment}
    button .buttonResetIncr -text $FGrab(incr) -command {Increment}
    label .name -text "Name" -pady 1m
    label .path -text "Path" -pady 1m
    entry .entryName -width 15 -relief sunken -bd 2 -textvariable FGrab(name)
    entry .entryPath -width 15 -relief sunken -bd 2 -textvariable FGrab(path)

    pack .name .path -in .frameCaptFile1 -side top
    pack .entryName .entryPath -in .frameCaptFile2 -side top
    pack .checkAutoIncr .buttonResetIncr -in .frameCaptFile3 -side left 

    frame .fraCaptOption2 -relief raised -bd 2
    frame .fraCaptCont -relief raised -bd 2
    frame .fraCaptBut -relief ridge -bd 4
    pack .fraCaptOption2 -in .capture3 -side left -fill y
    pack .fraCaptBut -in .capture3 -side right -fill y
    pack .fraCaptCont -in .capture3 -side left -fill both -expand 1

    button .buttonCapt -text "Start" -command {Capture Start} -padx 3m -pady 5m
    pack .buttonCapt -in .fraCaptBut -pady 3m -padx 3m

    checkbutton .checkCaptCor -text "Correlate" -variable FGrab(corr)\
	    -anchor w -command {Correlate}
    checkbutton .checkCaptCont -text "Continous" -variable FGrab(continous)\
	    -anchor w -command {Continous}
    checkbutton .checkAutoShow -text "Load Image" -variable FGrab(autoload)\
	    -anchor w
    pack .checkCaptCor .checkCaptCont .checkAutoShow -in .fraCaptOption2\
	    -side top -fill x

    checkbutton .butCaptFast -text "fast grabbing" -variable FGrab(fast)\
	    -anchor w -state disabled -command {Fast}
    pack .butCaptFast -in .fraCaptCont -side top -fill x

    frame .fraCaptCont1
    frame .fraCaptCont2
    pack .fraCaptCont1 .fraCaptCont2 -in .fraCaptCont -side left

    label .labelAmount -text "Amount" -pady 1m -anchor w -fg #a3a3a3
    label .labelIntervall -text {Intervall [s]} -pady 1m -anchor w -fg #a3a3a3

    entry .entryAmount -width 10 -relief sunken -bd 2 -state disabled\
	    -textvariable FGrab(amount)
    entry .entryIntervall -width 10 -relief sunken -bd 2 -state disabled\
	    -textvariable FGrab(intervall)

    pack .labelAmount .labelIntervall -in .fraCaptCont1 -side top -fill x
    pack .entryAmount .entryIntervall -in .fraCaptCont2 -side top



    foreach i $l2 {
	foreach k $l3($i) {
	    set value [lindex $FGrab($i,$k) 0]
	    set from  [lindex $FGrab($i,$k) 1]
	    set to    [lindex $FGrab($i,$k) 2]
	    set res   [lindex $FGrab($i,$k) 3]
	    frame .$i$k
	    label .label$i$k -text $k -anchor e
	    scale .scale$i$k -from $from -to $to  -length $length\
		    -orient vertical -resolution $res\
		    -command [list Configure $i $k]\
		    -sliderlength 10	    
	    .scale$i$k set $value
	    pack .$i$k -in .$i -side left
	    pack .label$i$k .scale$i$k -in .$i$k -side top -expand 1 -fill x
	}
    }
    foreach k $FGrab(list3,video) {
	.scalevideo$k configure -state disabled
    }
}


ControlPanel



























