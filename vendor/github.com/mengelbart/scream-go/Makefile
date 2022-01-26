.PHONY: clean

TARGET=scream

$(TARGET): libscream.a
	go build .

libscream.a: ScreamTx.o ScreamTxC.o ScreamRx.o ScreamRxC.o
	ar r $@ $^

%.o: %.cpp
	g++ -Wno-overflow -Wno-write-strings -O2 -o $@ -c $^

clean:
	rm -f *.o *.so *.a $(TARGET)
