
BINS=sc3u-nptl.so

all: $(BINS)

%.so: %.c
	$(CC) $(CFLAGS) -m32 -shared -nostartfiles -fPIC -Wall -o $@ $^ -ldl

clean:
	rm -f $(BINS)
