######################################
	pq command line app
######################################


pq is a command line app that does the same as the Pd [pulqui] object, it generates a wavf file from a given wav file. The wavf file is for later use with the "pulqui-limiter.pd" patch.
It is intended for use in a script or batch if you need to create many files.
WAV files can be fixed point 16 or 24 bit or IEEE floating point 32 bit.

Usage: 

	pq <path/file>

######################################
Linux/macOS:
######################################

You have to give read/write permission to pq. Open a terminal on its dir an do:

	sudo chmod 775 ./pq
	
Then create a script (some-name.sh) in the same dir as pq:

	#!/bin/bash
	./pq <path/file>
	./pq <path/file2>
	./pq <path/file3>
	...
save it and then give it permission:

	sudo chmod 775 some-name.sh
	
and run it.

######################################	
Windows:
######################################

Create an empty text document and name it "some-name.bat" (make sure it ends ".bat" and not ".bat.txt").
Edit it with a text editor:

	%CD%\pq <path\file>
	%CD%\pq <path\file2>
	%CD%\pq <path\file3>
	...
	
Save the file in the same dir as pq and run it.

