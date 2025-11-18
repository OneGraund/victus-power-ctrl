# use build/ fodler for binaries, include/ for headers, src/ for C files
CC ?= @gcc
CFLAGS := -Wall -Wextra -Wpedantic -Wformat=2 -Wshadow -Wstrict-prototypes -Wmissing-prototypes
include_folder := include
build_folder := build
src_folder := src
service_folder := service
exec_name := victus-power-ctrl

#create build folder
$(shell mkdir -p $(build_folder))

all: $(build_folder)/${exec_name}.o $(build_folder)/utils.o $(build_folder)/tests.o $(build_folder)/power.o $(build_folder)/temps.o $(build_folder)/fans.o
	$(CC) -o ${build_folder}/${exec_name} ${build_folder}/${exec_name}.o $(build_folder)/utils.o $(build_folder)/tests.o $(build_folder)/power.o $(build_folder)/temps.o $(build_folder)/fans.o

run: all
	./$(build_folder)/${exec_name}

# build/victus-power-ctrl.o
$(build_folder)/${exec_name}.o: \
							${src_folder}/main.c \
							${include_folder}/main.h \
							${include_folder}/tests.h \
							${include_folder}/utils.h
	$(CC) $(CFLAGS) -c -o $@ -I ${include_folder} $< 
	#     target(${exec_name}.o)     first dependency (main.c)

$(build_folder)/utils.o: ${src_folder}/utils.c ${include_folder}/utils.h ${include_folder}/main.h
	$(CC) $(CFLAGS) -c -o $@ -I ${include_folder} $<

$(build_folder)/tests.o: ${src_folder}/tests.c ${include_folder}/tests.h ${include_folder}/main.h ${include_folder}/utils.h
	$(CC) $(CFLAGS) -c -o $@ -I ${include_folder} $<

$(build_folder)/power.o: ${src_folder}/power.c ${include_folder}/power.h ${include_folder}/main.h ${include_folder}/utils.h
	$(CC) $(CFLAGS) -c -o $@ -I ${include_folder} $<

$(build_folder)/temps.o: ${src_folder}/temps.c ${include_folder}/temps.h ${include_folder}/main.h ${include_folder}/utils.h
	$(CC) $(CFLAGS) -c -o $@ -I ${include_folder} $<

$(build_folder)/fans.o: ${src_folder}/fans.c ${include_folder}/fans.h ${include_folder}/main.h ${include_folder}/utils.h
	$(CC) $(CFLAGS) -c -o $@ -I ${include_folder} $<

service_install: all 
	sudo install $(build_folder)/${exec_name} /usr/local/bin/${exec_name}
	sudo cp ${service_folder}/${exec_name}.service /etc/systemd/system/
	sudo systemctl daemon-reload
	sudo systemctl enable ${exec_name}.service
	
service_start: service_install
	sudo systemctl start ${exec_name}.service

service_stop: 
	sudo systemctl stop ${exec_name}.service

service_rm:
	sudo systemctl stop ${exec_name}.service
	sudo systemctl disable ${exec_name}.service
	sudo rm /etc/systemd/system/${exec_name}.service
	sudo systemctl daemon-reload

service_status:
	sudo systemctl status ${exec_name}.service

service_logs:
	sudo journalctl -u ${exec_name}.service -f
	

clean:
	rm -rf ${build_folder}