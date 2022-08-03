lib.name = pulqui

class.sources = pulqui.c
            

datafiles = \
pulqui-help.pd \
pulqui-limiter.pd \
README.md \
pq-readme.txt\
License.txt \


datadirs = \
example-files \


makefiles = \
pq-cli-app/Makefile \

forLinux = datafiles += \
pq-cli-app/pq \

forDarwin = datafiles += \
pq-cli-app/pq \


forWindows = datafiles += \
pq-cli-app/pq.exe \



include pd-lib-builder/Makefile.pdlibbuilder


