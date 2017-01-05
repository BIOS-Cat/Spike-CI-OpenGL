LDFLAGS+=-framework OpenGL `pkg-config --cflags --libs check`

all: open_gl_test_suite

open_gl_test_suite: open_gl_test.c
ifeq ($(TRAVIS),1)
	$(CC) $(CFLAGS) -D"__travis__=1" -o $@ $< $(LDFLAGS)
else
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
endif

clean:
	rm open_gl_test_suite
