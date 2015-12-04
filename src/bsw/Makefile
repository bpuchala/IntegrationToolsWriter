# make command line interface program: bsw

bsw: bsw.cpp ../version.o PBasisSetWriter.o ../fw/PFunctionWriter.o 
	cd .. && $(MAKE) version.o
	$(CXX) PBasisSetWriter.o ../fw/PFunctionWriter.o ../version.o -o bsw bsw.cpp $(CXXFLAGS) $(LFLAGS) $(LDLIBS)

install: bsw
	install bsw $(bindir)

uninstall:
	rm $(bindir)/bsw

../version.o:
	cd .. && $(MAKE) version.o

../version.cc:
	cd .. && $(MAKE) version.cc: 

../fw/PFunctionWriter.o:
	cd ../fw && $(MAKE) PFunctionWriter.o

PBasisSetWriter.o: PBasisSetWriter.cc $(PSYM)/PRealSymBasisFunction.hh $(PSYM)/PRealSymBasisSet.hh $(PSYM)/PRealSymRecursBasisSet.hh $(WRITER)/writer/* ../version.cc 
	$(CXX) -c PBasisSetWriter.cc $(CXXFLAGS)

clean: 
	rm -f *.o bsw



