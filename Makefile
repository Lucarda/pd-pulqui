lib.name = pulqui

class.sources = \
    ./src/pulqui.c \
    ./src/pulqui~.c \
    ./src/pulquilimiter~.c \
    ./src/pqcrossover~.c \
    ./src/pqpeak~.c \
    $(empty)

datafiles = \
    pulqui-help.pd \
    pulqui~-help.pd \
    pulquilimiter~-help.pd \
    pulqui-limiter.pd \
    pulqui-limiter(v0.1).pd \
    pqcrossover~-help.pd \
    pqpeak~-help.pd \
    README.md \
    readme.html \
    License.txt \
    $(empty)

datadirs = \
    example-files \
    $(empty)

include pd-lib-builder/Makefile.pdlibbuilder
