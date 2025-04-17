ifeq ($(USER),root)
	$(info )
	$(info 838388381 you can NOT run by root !!! )
	$(info )
	$(error )
endif

.PHONY: all clean aaa a build

# your_secure_password_here1

# hash.c  http-parser.c  http-server.c  http-utils.c  main.c  redis-ip.c  redis-store.c  redis-user.c  redis.c  server.c  utils.c
source1 := src/hash.c
source2 := src/http-parser.c
source3 := src/http-server.c
source4 := src/http-utils.c
source5 := src/main.c
source6 := src/redis_check_ip.c
source7 := src/redis_increment_failed.c
source8 := src/redis-store.c
source9 := src/redis-user.c
sourceA := src/redis.c
sourceB := src/server.c
sourceC := src/utils.c

dst     := bin/signup_daemon.bin
CFLAGS := -Wall -O3 -static
CFLAGS := -Wall -Os -static
CFLAGS := -Wall -Os -ffunction-sections -fdata-sections -Wl,--gc-sections -static -D_FORTIFY_SOURCE=0 -DNDEBUG
CFLAGS := -Wall -Oz -ffunction-sections -fdata-sections -Wl,--gc-sections -static -D_FORTIFY_SOURCE=0 -DNDEBUG
CFLAGS := -Wall -O3 -ffunction-sections -fdata-sections -Wl,--gc-sections -static
CFLAGS := -Wall -O3 -ffunction-sections -fdata-sections -Wl,--gc-sections -static
CFLAGS := -Wall -O3 -ffunction-sections -fdata-sections -Wl,--gc-sections
LDLIBS := -lgd -lpng -lz -ljpeg -lfreetype -lm -lmicrohttpd -lgnutls
LDLIBS := -lgd -lpng -lz -ljpeg -lfreetype -lm
LDLIBS := -lgd -lpng -lz -ljpeg -lfreetype -lm -lfontconfig -lbrotlidec -lbrotlicommon -lbz2 -lexpat
LDLIBS := -lgd -lpng -lz -ljpeg -lfreetype -lm -lfontconfig -lbrotlidec -lbrotlicommon -lbz2 -lexpat  -lssl -lcrypto
LDLIBS := -lhiredis -ljansson -largon2 -lssl -lcrypto
#-lbrotlicommon -lbrotlienc
installDir:=/home/nginX/bin/
installBin1:=/home/nginX/bin/$(shell basename $(dst))


all:
	@echo "$${allTEXT}"

define allTEXT

aaa : $(aaa)


endef
export allTEXT

b:ball
v:v1

m:
	vim Makefile
v1:
	vim $(source1)
v2:
	vim $(source2)
v3:
	vim $(source3)
v4:
	vim $(source4)
v5:
	vim $(source5)
v6:
	vim $(source6)

aaa := make c && make ball && make in
aaa :
	$(aaa)


b1 : $(mid1)
#	strip --strip-all --strip-unneeded $<
#	md5sum $<
b2 : $(mid2)
#	strip --strip-all --strip-unneeded $<
#	md5sum $<
b3 : $(mid3)
#	strip --strip-all --strip-unneeded $<
#	md5sum $<
b4 : $(mid4)
#	strip --strip-all --strip-unneeded $<
#	md5sum $<

c clean:
	rm -rf bin/*.bin tmp/*.o


in install:
	@echo
	-chmod   u+w   $(installDir)/
	cat   $(dst)   > $(installBin1)
	@ls -l --color   $(installBin1)
	@md5sum          $(installBin1)
	@echo

vpc:
	rm -f tags \
		cscope.in.out \
		cscope.out \
		cscope.po.out
	rm -f _vim/file01.txt

vp vim_prepare : vpc
	mkdir -p _vim/
	echo $(Makefile)                                     > _vim/file01.txt
	-test -f Makefile.env && echo Makefile.env          >> _vim/file01.txt
	find -type f -name "*.c" \
		|grep -v '\.bak[0-9]*' \
		| xargs -n 1 realpath --relative-to=.|sort -u   >> _vim/file01.txt
	sed -i -e '/^\.$$/d' -e '/^$$/d'                       _vim/file01.txt
	cscope -q -R -b -i                                     _vim/file01.txt
	ctags -L                                               _vim/file01.txt
gs:
	git status
gd:
	git diff
gc:
	git add .
	git commit -a

#	git_ssh_example.sh git push
up:
	git push
	sync


#	$(CC) $(CFLAGS) -c -o $(mid1) $(source1) $(LDLIBS)

ball:
	$(CC) $(CFLAGS) -static -o $(dst) \
		$(source1)   \
		$(source2)   \
		$(source3)   \
		$(source4)   \
		$(source5)   \
		$(source6)   \
		$(source7)   \
		$(source8)   \
		$(source9)   \
		$(sourceA)   \
		$(sourceB)   \
		$(sourceC)   \
		$(LDLIBS)
	strip $(dst)

