CFLAGS=-Wall -g -shared -fPIC -rdynamic
CC=gcc
INCLUDE_PATH =  -Iphp/ -Iphp/include -Iphp/main/ -Iphp/Zend -Iphp/ZendEngine2 -Iphp/TSRM
PHP_LIB=php/libs/libphp5.a

all: modules

modules: libape-chat.c libape-mysql.c libape-setcord.c libape-helloworld.c
	
	$(CC) $(CFLAGS) -Wl,-soname,libmod_setcord.so -o lib/libmod_setcord.so libape-setcord.c
	#$(CC) $(CFLAGS) -Wl,-soname,libmod_mouse.so -o lib/libmod_mouse.so libape-mouse.c
	$(CC) $(CFLAGS) -Wl,-soname,libmod_controller.so -o lib/libmod_controller.so libape-controller.c	
	$(CC) $(CFLAGS) -Wl,-soname,libmod_chat.so -o lib/libmod_chat.so libape-chat.c
	#$(CC) $(CFLAGS) -Wl,-soname,libmod_spidermonkey.so -o lib/libmod_spidermonkey.so libape-spidermonkey.c -I../../js/src/ -I../../js/src/Linux_All_DBG.OBJ -L../../js/src/Linux_All_DBG.OBJ/ -ljs
	
	#$(CC) $(CFLAGS) -Wl,-soname,libmod_mysql.so -L/usr/local/mysql/lib -lmysqlclient -lz -o lib/libmod_mysql.so libape-mysql.c
	#$(CC) $(CFLAGS) -Wl,-soname,libmod_chat.so -o lib/libmod_chat.so libape-chat.c -L./lib -lmod_mysql -Xlinker "-R\$$ORIGIN"
	#$(CC) $(CFLAGS) -Wl,-soname,libmod_helloworld.so -o lib/libmod_helloworld.so libape-helloworld.c
	#$(CC) -Wall -o ./objs/libape-php.o -c libape-php.c $(INCLUDE_PATH)
	#$(CC) $(CFLAGS) -Wl,-soname,libmod_php.so -o lib/libmod_php.so objs/libape-php.o $(PHP_LIB) -ldl -lm -lcrypt -lresolv -lxml2 -lpthread
	
#./configure --enable-embed=static --with-zlib --enable-ftp --enable-static=zlib --disable-simplexml --disable-xmlreader --disable-xmlwriter --disable-soap


