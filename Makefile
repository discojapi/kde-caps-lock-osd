bin/out : src/main.cpp
	g++ src/main.cpp -o bin/out

clean:
	rm bin/out