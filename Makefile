lib.name = pulqui


class.sources = \
    ./src/pulqui.c \
    ./src/pulqui~.c \
    ./src/pulquilimiter~.c \
    $(empty)

datafiles = \
    pulqui-help.pd \
    pulqui-limiter.pd \
    README.md \
    License.txt \
    $(empty)

datadirs = \
    example-files \
    $(empty)
    
makefiles = ./src/pq-cli-app/Makefile

forLinux = datafiles += ./src/pq-cli-app/pq

forDarwin = datafiles += ./src/pq-cli-app/pq

forWindows = datafiles += ./src/pq-cli-app/pq.exe


include pd-lib-builder/Makefile.pdlibbuilder
