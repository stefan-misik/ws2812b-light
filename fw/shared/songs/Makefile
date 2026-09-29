COMPOSE = python compose.py

SONGS =  \
    silent_night  \
    jingle_bells  \
    we_wish_you_a_merry_christmas  \
    deck_the_halls  \
    joy_to_the_world

SONGS_INC = $(addsuffix .inc,$(SONGS))

all: $(SONGS_INC)

clean:
	$(RM) $(SONGS_INC)

%.inc: %.tune
	$(COMPOSE) $< -o $@

.PHONY: all clean