#ARCH=ARM
OTHERSLIBPWD=/home/usrc

ifeq ($(ARCH),ARM)
#CROSS=arm-linux-
CROSS=mipsel-openwrt-linux-
endif

SRC:=$(shell ls *.c)
SRC+=$(shell ls *.cpp)
SRC+=$(shell ls $(PWD)/log/*.cpp)
SRC+=$(shell ls $(PWD)/safe/*.cpp)
SRC+=$(shell ls $(PWD)/json/src/json/*.cpp)

#CFLAG:=-m32
#LDFLAG:=-lrt

ifeq ($(ARCH),ARM)
IPATH:=-I$(PWD)/curl_openwrt
IPATH+=-I$(PWD)/log
IPATH+=-I$(PWD)/libconfig-x86-64/include
IPATH+=-I$(PWD)/json
else
IPATH:=-I$(PWD)/curl_x86_64
IPATH+=-I$(PWD)/log
IPATH+=-I$(PWD)/libconfig-x86-64/include
IPATH+=-I$(PWD)/json
endif

ifeq ($(ARCH),ARM)
LPATH:=-L$(PWD)/curl_openwrt
LPATH+=-L$(PWD)/libconfig-x86-64/lib
else
LPATH:=-L$(PWD)/curl_x86_64
LPATH+=-L$(PWD)/libconfig-x86-64/lib
endif

ifeq ($(ARCH),ARM)
LIBS:=-lpthread -ldl -lcurl
LIBS+=$(PWD)/libconfig-x86-64/lib/libconfig++.a
else
LIBS:=-lpthread -ldl -lcurl
LIBS+=$(PWD)/libconfig-x86-64/lib/libconfig++.a
endif

ifeq ($(ARCH),ARM)
TARGET:=arp_resolve.bin
else
TARGET:=arp_resolve.bin
endif

$(TARGET) : $(SRC)
	$(CROSS)g++ $(CFLAG) -o $(TARGET) $^ $(LPATH) $(IPATH) $(LIBS) $(LDFLAG)

clean:
	rm -f  *.bin  *.dis  *.elf  *.o
