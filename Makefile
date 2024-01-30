lib.name = pulqui

class.sources = \
    ./src/pulqui.c \
    ./src/pulqui~.c \
    ./src/pulquilimiter~.c \
    $(empty)

datafiles = \
    pulqui-help.pd \
    pulqui~-help.pd \
    pulquilimiter~-help.pd \
    pulqui-limiter.pd \
    pulqui-limiter(v0.1).pd \
    README.md \
    readme.html \
    License.txt \
    $(empty)

datadirs = \
    example-files \
    $(empty)

include pd-lib-builder/Makefile.pdlibbuilder
