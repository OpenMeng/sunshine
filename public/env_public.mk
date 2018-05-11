#include $(ORACLE_HOME)/rdbms/lib/env_rdbms.mk

ifdef DEBUG
	PD = $(SUNSHINE_HOME)/debug
else
	PD = $(SUNSHINE_HOME)/release
endif

PD = $(SUNSHINE_HOME)

BILL_SRC = $(SUNSHINE_SRC)/uni_billing/rate
CTRL_SRC = $(SUNSHINE_SRC)/uni_billing/ctrl
PUB_INC = $(SUNSHINE_SRC)/uni_billing/public/include
BILL_INC = $(BILL_SRC)/public/include
CTRL_INC = $(CTRL_SRC)/ctrl/include
PUB_LIB = $(PD)/lib
BILL_LIB = $(PD)/lib
CTRL_LIB = $(PD)/lib
BILL_BIN = $(PD)/bin
CTRL_BIN = $(PD)/bin

#INCLUDES = -I. -I$(PUB_INC) -Iinclude -I$(ORACLE_HOME)/precomp/public \
#	-I$(ORACLE_HOME)/rdbms/demo -I$(ORACLE_HOME)/rdbms/public -I$(LIBSSH_HOME)/include
INCLUDES = -I. -I$(PUB_INC) -I$(CTRL_SRC)/seq/include -Iinclude -I$(ODBC_HOME)/include -I$(LIBSSH_HOME)/include -I$(OTL_HOME)

#ifdef ORA9
#	ORACLE_LIB = -locci9
#	ORACLE_FLAGS = -DORACLE_V9204
#else
#	ORACLE_LIB = -locci
#	ORACLE_FLAGS = 
#endif
ODBC_LIB = -lodbc
ODBC_FLAGS = 

ifeq "$(OS)" "HP-UX"

ifeq "$(MODE)" "64bit"
	MFLAGS = -mt +DD64
endif

ifdef DEBUG
	DFLAGS = -DDEBUG -g +d
else
	DFLAGS = +O2 +inline_level2
endif
	
	CC = aCC
#	PCFLAGS = $(INCLUDES) -DHPUX -AA -DNAMESPACE -w -mt $(DFLAGS) $(MFLAGS) $(ORACLE_FLAGS)
	PCFLAGS = $(INCLUDES) -DHPUX -AA -DNAMESPACE -w -mt $(DFLAGS) $(MFLAGS) $(ODBC_FLAGS)
#	PUBFLAGS = -L$(ORACLE_HOME)/lib -lclntsh -lpthread -lm -lc -lnsl -lrt $(ORACLE_LIB) -ldl
	PUBFLAGS = -L$(ODBC_HOME)/lib -lclntsh -lpthread -lm -lc -lnsl -lrt $(ODBC_LIB) -ldl
	ARFLAG = rcu
	SOFLAGS = -b -dynamic
	STRIP = strip -l
endif

ifeq "$(OS)" "AIX"

ifeq "$(MODE)" "64bit"
	MFLAGS = -q64
	MLFLAGS = -X64
endif

ifdef DEBUG
	DFLAGS = -DDEBUG -g -qfullpath
else
	DFLAGS = -O2 -qstaticinline
endif
	
	CC = xlC_r -w -qmaxmem=-1
#	PCFLAGS = -bmaxdata:0x80000000 -qthreaded $(INCLUDES) -DAIX -DNAMESPACE -bhalt:5 $(DFLAGS) $(MFLAGS) $(ORACLE_FLAGS)
#	PUBFLAGS = -L$(ORACLE_HOME)/lib -lclntsh -lpthread -lm -lc -lnsl -lrt $(ORACLE_LIB) -ldl -brtl
	PCFLAGS = -bmaxdata:0x80000000 -qthreaded $(INCLUDES) -DAIX -DNAMESPACE -bhalt:5 $(DFLAGS) $(MFLAGS) $(ODBC_FLAGS)
	PUBFLAGS = -L$(ODBC_HOME)/lib  -lpthread -lm -lc -lnsl -lrt $(ODBC_LIB) -ldl -brtl
	ARFLAG = $(MLFLAGS) rcu
	ARFLAG = $(MLFLAGS) rcu
	SOFLAGS = -G -bM:SRE -bnoentry -qrtti=all -qmkshrobj
	STRIP = strip $(MLFLAGS)
endif

ifeq "$(OS)" "Linux"

ifeq "$(MODE)" "64bit"
	MFLAGS = -D_LARGEFILE64_SOURCE=1
else
	MFLAGS = -D_LARGEFILE_SOURCE=1
endif

ifdef DEBUG
	DFLAGS = -DDEBUG -g
else
	FLAGS = -O2
endif
	
	CC = g++
#	PCFLAGS = $(INCLUDES) -DLINUX -fPIC -DNAMESPACE -D_GNU_SOURCE -DSLTS_ENABLE -DSLMXMX_ENABLE -D_REENTRANT -DNS_THREADS $(DFLAGS) $(MFLAGS) $(ORACLE_FLAGS)
#	PUBFLAGS = -L$(ORACLE_HOME)/lib -lclntsh -lpthread -lm -lc -lnsl -lrt $(ORACLE_LIB) -ldl -L$(LIBSSH_HOME)/lib -lssh2 
	PCFLAGS = $(INCLUDES) -DLINUX -fPIC -DNAMESPACE -D_GNU_SOURCE -DSLTS_ENABLE -DSLMXMX_ENABLE -D_REENTRANT -DNS_THREADS $(DFLAGS) $(MFLAGS) $(ODBC_FLAGS)
	PUBFLAGS = -L$(ODBC_HOME)/lib  -lpthread -lm -lc -lnsl -lrt $(ODBC_LIB) -ldl -L$(LIBSSH_HOME)/lib -lssh2 
	ARFLAG = rcu
	ARFLAG = rcu
	SOFLAGS = -shared
	STRIP = strip -g
endif

ifeq "$(OS)" "SunOS"

ifeq "$(MODE)" "64bit"
	MFLAGS = -xarch=generic64
endif

ifdef DEBUG
	DFLAGS = -DDEBUG -g
else
	DFLAGS = -O2
endif
	
	CC = CC
#	PCFLAGS = $(INCLUDES) -DSOLARIS -DNAMESPACE -Kpic -mt $(DFLAGS) $(MFLAGS)
#	PUBFLAGS = -L$(ORACLE_HOME)/lib -R$(ORACLE_HOME)/lib -lclntsh -lpthread -lm -lc -lnsl -lrt $(ORACLE_LIB) -ldl -lsunmath -mt $(MFLAGS) $(ORACLE_FLAGS)
	PCFLAGS = $(INCLUDES) -DSOLARIS -DNAMESPACE -Kpic -mt $(DFLAGS) $(MFLAGS)
	PUBFLAGS = -L$(ORACLE_HOME)/lib -R$(ODBC_HOME)/lib  -lpthread -lm -lc -lnsl -lrt $(ODBC_LIB) -ldl -lsunmath -mt $(MFLAGS) $(ODBC_FLAGS)
	ARFLAG = -cr 
	ARFLAG = -cr 
	SOFLAGS = -G -Kpic
	STRIP = strip 
endif

PLDFLAGS = -L$(PUB_LIB) $(PUBFLAGS) -lpublic
