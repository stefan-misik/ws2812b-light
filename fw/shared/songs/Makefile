COMPOSE = python compose.py

SONGS =  \
    silent_night  \
    jingle_bells

SONGS_INC = $(addsuffix .inc,$(SONGS))

all: $(SONGS_INC)

clean:
	$(RM) $(SONGS_INC)

%.inc: %.tune
	$(COMPOSE) $< -o $@

.PHONY: all clean