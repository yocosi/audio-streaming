GCC=gcc
flag=-Wall -g

client:
	${GCC} -o streamingClient audioclient.c audio.h ${flag}

serveur:
	${GCC} -o streamingServeur audioserver.c audio.h ${flag}

runClient:
	 ./streamingClient

runServeur:
	 ./streamingServeur

clean:
	rm -r streamingClient streamingServeur