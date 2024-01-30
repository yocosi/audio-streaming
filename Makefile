GCC=gcc
flag=-Wall -g


client:
	${GCC} -o streamingClient audioclient.c audio.c ${flag}

serveur:
	${GCC} -o streamingServeur audioserver.c audio.c ${flag}

runClient:
	 padsp ./streamingClient 127.0.0.1 test.wav <put_filter_here> [filter_argument_if_needed]

runServeur:
	 ./streamingServeur

clean:
	rm -r streamingClient streamingServeur