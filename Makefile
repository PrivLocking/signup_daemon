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
source3 := src/http_get_client_ip.c
source4 := src/http_parser.c
source5 := src/http_server_main_loop.c
source6 := src/http_utils.c
source7 := src/main.c
source8 := src/redis_check_ip.c
sourceA := src/redis_increment.c
sourceD := src/redis_set_hset_key_value.c
sourceF := src/redis_store.c
sourceG := src/redis.c
sourceH := src/redis_get_hget_string.c
sourceI := src/redis_get_int.c
sourceJ := src/send_response.c
sourceK := src/send_response_sess.c
sourceL := src/server.c
sourceM := src/string_check_a2f_0to9.c
sourceN := src/utils.c
sourceO:= src/get_executable_md5.c
sourceP:= src/check_post_type_path.c
sourceQ:= src/sess_handle_new_request.c
sourceR:= src/http_main_loop_clean_before_exit.c
sourceS:= src/http_server_main_loop_handler.c

sourceListIdx:=0 1 2 3 4 5 6 7 8 A D F G H I J K L M N O P Q R S 
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
	echo Makefile                                        > _vim/file01.txt
	-test -f Makefile.env && echo Makefile.env          >> _vim/file01.txt
	find -type f -name "*.c" -o -name "*.h" \
		|grep -v '\.bak[0-9]*' \
		| xargs -n 1 realpath --relative-to=.|sort -u   >> _vim/file01.txt
	find signup_html/ -maxdepth 1 -type f               >> _vim/file01.txt
	find login_html/  -maxdepth 1 -type f               >> _vim/file01.txt
	find admin_html/  -maxdepth 1 -type f               >> _vim/file01.txt
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
	( cd signup_html/ && tar -cf - .) | ( cd usignup/ && rm -fr * && tar -xf - )
	-diff -r signup_html/                    usignup/ 2>&1 |grep -v 'No such file or directory'
	( cd login_html/  && tar -cf - .) | ( cd ulogin/ && rm -fr * && tar -xf - )
	-diff -r login_html/                     ulogin/ 2>&1 |grep -v 'No such file or directory'
	( cd admin_html/  && tar -cf - .) | ( cd uadmin/ && rm -fr * && tar -xf - )
	-diff -r admin_html/                     uadmin/ 2>&1 |grep -v 'No such file or directory'
