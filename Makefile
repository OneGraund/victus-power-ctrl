# use build/ fodler for binaries, include/ for headers, src/ for C files
CC ?= @gcc
include_folder := include
build_folder := build
src_folder := src
service_folder := service
exec_name := victus-power-ctrl

#create build folder
$(shell mkdir -p $(build_folder))

all: $(build_folder)/${exec_name}.o $(build_folder)/utils.o $(build_folder)/tests.o
	$(CC) -o ${build_folder}/${exec_name} ${build_folder}/${exec_name}.o $(build_folder)/utils.o $(build_folder)/tests.o

run: all
	./$(build_folder)/${exec_name}

# build/victus-power-ctrl.o
$(build_folder)/${exec_name}.o: \
							${src_folder}/main.c \
							${include_folder}/main.h \
							${include_folder}/tests.h \
							${include_folder}/utils.h
	$(CC) -c -o $@ -I ${include_folder} $< 
	#     target(${exec_name}.o)     first dependency (main.c)

$(build_folder)/utils.o: ${src_folder}/utils.c ${include_folder}/utils.h ${include_folder}/main.h
	$(CC) -c -o $@ -I ${include_folder} $<

$(build_folder)/tests.o: ${src_folder}/tests.c ${include_folder}/tests.h ${include_folder}/main.h ${include_folder}/utils.h
	$(CC) -c -o $@ -I ${include_folder} $<

service_install: all 
	systemctl --user daemon-reload
	sudo install $(build_folder)/${exec_name} /usr/local/bin/${exec_name}
	cp ${service_folder}/${exec_name}.service ${HOME}/.config/systemd/user/
	systemctl --user enable ${exec_name}.service
	
service_start: service_install
	systemctl --user start ${exec_name}.service

service_stop: 
	systemctl --user stop ${exec_name}.service

service_rm: service_install
	systemctl --user stop ${exec_name}.service
	systemctl --user disable ${exec_name}.service
	rm ${HOME}/.config/systemd/user/${exec_name}.service
	systemctl --user daemon-reload

service_status: service_install
	systemctl --user status ${exec_name}.service

service_logs:
	

clean:
	rm -rf ${build_folder}