# Autoconf/automake file

# General compiler/linker flags
AM_CFLAGS = ${CFLAGS}
AM_FCFLAGS = ${FCFLAGS}
AM_LDFLAGS = ${LDFLAGS}

all:
if WITH_SHAKEOUT2_LARGEDATA_DIR
	echo "USING WITH_SHAKEOUT2_LARGEDATA_DIR in shakeout2"
else
	echo "NOT USING WITH_SHAKEOUT2_LARGEDATA_DIR in shakeout2"
	if [ ! -d "shakeout2" ] && [ ! -L "shakeout2" ]; then \
		./make_data_files.py; \
	fi 
endif

install:
if WITH_SHAKEOUT2_LARGEDATA_DIR
	if [ ! -d "${prefix}/data/shakeout2" ]; then \
		$(MKDIR_P) ${prefix}/data; \
		$(LN_S) -f $(SHAKEOUT2_LARGEDATA_DIR) ${prefix}/data/shakeout2; \
		echo "USING WITH_SHAKEOUT2_LARGEDATA_DIR in shakeout2"; \
	fi
	cp config ${prefix}/data
else
	if [ ! -d "${prefix}/data/shakeout2" ]; then \
		$(MKDIR_P) ${prefix}/data; \
		mv shakeout2 ${prefix}/data; \
		$(LN_S) -f ${prefix}/data/shakeout2 .; \
		echo "NOT USING WITH_SHAKEOUT2_LARGEDATA_DIR in shakeout2"; \
	fi
	cp config ${prefix}/data
endif

clean:
	echo "clean"
