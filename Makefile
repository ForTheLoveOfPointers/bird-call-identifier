CLIB = -I ./lib/portaudio/include ./lib/portaudio/lib/.libs/libportaudio.a -lrt -lasound -ljack -pthread \
	-I ./lib/fftw-3.3.10/api -lfftw3

audio_visualizer: main.cpp
	g++ -o $@ $^ $(CLIB)

install-deps:
	make install-portaudio
	make install-fftw

install-portaudio:
	rmdir lib
	mkdir lib
	cd lib && curl https://files.portaudio.com/archives/pa_stable_v190700_20210406.tgz --output pa_stable
	cd lib && tar zxf pa_stable
	cd lib/portaudio && ./configure && $(MAKE) -j
	
install-fftw:
	cd lib && curl http://www.fftw.org/fftw-3.3.10.tar.gz --output fftw_stable
	cd lib && tar xf fftw_stable
	cd lib/fftw-3.3.10 && ./configure && $(MAKE) -j && sudo $(MAKE) install