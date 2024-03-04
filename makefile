# CC = g++
# CPPFLAGS = -I./HeaderFiles

# target = libevent.so
# src = $(wildcard ./SourceFiles/*.cpp)
# object = $(patsubst %.cpp, %.o, $(src))
# $(target) : $(object)
# 	$(CC) -shared $^ -o $@

# %.o : %.cpp
# 	$(CC) -fpic -c $< -o $@ $(CPPFLAGS)

# target2 = main
# src2 = main.cpp
# LDFLAGS = -L./ -levent
# $(target2) : $(src2)
# 	$(CC) $^ -o $@ $(CPPFLAGS) $(LDFLAGS)

# .PHONY: clean
# clean:
# 	rm -f $(target) $(object) $(target2)

CC = g++
CPPFLAGS = -I./HeaderFiles

target = libevent.a
src = $(wildcard ./SourceFiles/*.cpp)
objects = $(patsubst %.cpp, %.o, $(src))
$(target): $(objects)
	ar rcs $@ $^

%.o: %.cpp
	$(CC) -c $< -o $@ $(CPPFLAGS)

target2 = main
src2 = main.cpp
LDFLAGS = -L./ -levent
$(target2): $(src2)
	$(CC) $(src2) -o $@ $(CPPFLAGS) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(objects) $(target) $(target2)