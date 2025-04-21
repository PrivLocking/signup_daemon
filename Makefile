ifeq ($(USER),root)
	$(info )
	$(info 838388381 you can NOT run by root !!! )
	$(info )
	$(error )
endif

.PHONY: all clean aaa a build

# your_secure_password_here1

# hash.c  http-parser.c  http-server.c  http-utils.c  main.c  redis-ip.c  redis-store.c  redis-user.c  redis.c  server.c  utils.c
source0 := src/cookie_extract.c
source1 := src/gen_a_new_md5sum_hex_32byte.c
source2 := src/hash.c
source3 := src/http-parser.c
source4 := src/http-server.c
source5 := src/http-utils.c
source6 := src/main.c
source7 := src/redis_check_ip.c
source8 := src/redis_find_signup_sess_and_reset_its_TTL300.c
source9 := src/redis_increment_failed.c
sourceA := src/redis_save_key_to_redis_with_ttl.c
sourceB := src/redis_save_signup_sess_with_TTL300.c
sourceC := src/redis-store.c
sourceD := src/redis-user.c
sourceE := src/redis.c
sourceF := src/redis_get_string.c
sourceG := src/redis_get_int.c
sourceH := src/send_response.c
sourceI := src/send_response_with_new_signup_sess.c
sourceJ := src/server.c
sourceK := src/string_check_a2f_0to9.c
sourceL := src/utils.c
sourceM := src/get_executable_md5.c

sourceListIdx:=0 1 2 3 4 5 6 7 8 9 A B C D E F G H I J K L M
sourceList:=$(foreach ssss,$(sourceListIdx),$(source$(ssss)))



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

#v1:
#	vim $(source1)
define vimFun
v$(1): vp
	vim $(source$(1))
endef
$(foreach vimvim,$(sourceListIdx),$(eval $(call vimFun,$(vimvim))))
vv:
	@$(foreach ssss,$(sourceListIdx), echo "$(ssss) : $(source$(ssss))"; )

aaa := make c && make ball && make in
aaa :
	make sign
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
	find -type f -name "*.c" -o -name "*.h" \
		|grep -v '\.bak[0-9]*' \
		| xargs -n 1 realpath --relative-to=.|sort -u   >> _vim/file01.txt
	find signup_html/ -maxdepth 1 -type f               >> _vim/file01.txt
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
		$(sourceList)   \
		$(LDLIBS)
	strip $(dst)

sign signup:
	( cd signup_html/ && tar -cf - .) | ( cd signup_for_publish/ && tar -xf - )
	-diff -r signup_html/ signup_for_publish/ 2>&1 |grep -v 'No such file or directory'
