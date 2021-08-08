.PHONY: all 

all: seq_cst relaxed relaxed2 acquire_release acquire_release2 consume release_sequence memory_barriers memory_barriers2

seq_cst:
	g++ seq_cst.cpp -std=c++11 -o seq_cst

relaxed:
	g++ relaxed.cpp -std=c++11 -o relaxed

relaxed2:
	g++ relaxed2.cpp -std=c++11 -o relaxed2

acquire_release:
	g++ acquire_release.cpp -std=c++11 -o acquire_release

acquire_release2:
	g++ acquire_release2.cpp -std=c++11 -o acquire_release2

release-sequence:
	g++ release_sequence.cpp -std=c++11 -o release-sequence

consume:
	g++ consume.cpp -std=c++11 -o consume

memory_barriers:
	g++ memory_barriers.cpp -std=c++11 -o memory_barriers

memory_barriers2:
	g++ memory_barriers2.cpp -std=c++11 -o memory_barriers2

.PHONY: clean

clean:
	rm seq_cst relaxed relaxed2 acquire_release racquire_release2 release_sequence consume memory_barriers memory_barriers2
