# make command line interface program: fw

fw: fw.cpp ../version.o PFunctionWriter.o 
	$(CXX) PFunctionWriter.o ../version.o -o fw fw.cpp $(CXXFLAGS) $(LFLAGS) $(LDLIBS)

install: fw
	install fw $(bindir)

uninstall:
	rm $(bindir)/fw

../version.o:
	cd .. && $(MAKE) version.o

PFunctionWriter.o: PFunctionWriter.cc $(WRITER)/writer/PFunctionWriter.hh ../version.cc
	$(CXX) -c PFunctionWriter.cc $(CXXFLAGS)

clean: 
	rm -f *.o fw

