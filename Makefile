CLIB = -I ./lib/portaudio/include ./lib/portaudio/lib/.libs/libportaudio.a -lrt -lasound -ljack -pthread

audio_visualizer: main.cpp
	g++ -o $@ $^ $(CLIB)

install-deps:
	rmdir lib
	mkdir lib
	cd lib && curl https://files.portaudio.com/archives/pa_stable_v190700_20210406.tgz --output pa_stable
	cd lib && tar zxf pa_stable
	cd lib/portaudio && ./configure && $(MAKE) -j